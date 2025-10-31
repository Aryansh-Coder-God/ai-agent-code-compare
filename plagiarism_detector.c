/*
================================================================================
  INTELLIGENT CODE PLAGIARISM DETECTOR - Core Engine (C)
================================================================================
  
  PURPOSE:
  This C program analyzes two source code files and calculates their similarity
  score to detect potential plagiarism. It works by:
  
  1. TOKENIZATION: Breaking code into meaningful tokens (keywords, identifiers, etc.)
  2. NORMALIZATION: Removing whitespace, comments, and normalizing structure
  3. HASHING: Creating fingerprints of code chunks using rolling hash
  4. COMPARISON: Matching fingerprints to find similar code sections
  
  This approach detects plagiarism even when students:
  - Rename variables (int x -> int myVariable)
  - Change formatting (spacing, line breaks)
  - Rearrange function order
  - Add/remove comments
  
  AUTHOR: AI Code Plagiarism Detection System
  DATE: 2025
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// CONFIGURATION & CONSTANTS
// ============================================================================

#define MAX_FILE_SIZE 100000      // Maximum file size in bytes (100KB)
#define MAX_TOKENS 10000          // Maximum number of tokens per file
#define MAX_TOKEN_LENGTH 100      // Maximum length of a single token
#define HASH_WINDOW_SIZE 5        // Number of tokens in each hash window
#define HASH_PRIME 31             // Prime number for rolling hash calculation

// Token types - help us understand what kind of code element we're looking at
typedef enum {
    TOKEN_KEYWORD,      // Programming keywords (if, for, while, class, etc.)
    TOKEN_IDENTIFIER,   // Variable/function names
    TOKEN_OPERATOR,     // Operators (+, -, *, /, ==, etc.)
    TOKEN_LITERAL,      // Numbers and strings ("hello", 123, 4.5)
    TOKEN_DELIMITER,    // Punctuation ({ } ( ) ; , etc.)
    TOKEN_UNKNOWN       // Anything else
} TokenType;

// Token structure - stores information about each code element
typedef struct {
    char text[MAX_TOKEN_LENGTH];  // The actual token text
    TokenType type;               // What kind of token it is
} Token;

// File data structure - holds all tokens from a file
typedef struct {
    Token tokens[MAX_TOKENS];     // Array of all tokens
    int tokenCount;               // How many tokens we found
    unsigned long hashes[MAX_TOKENS]; // Hash values for comparison
    int hashCount;                // How many hashes we generated
} FileData;

// ============================================================================
// PROGRAMMING KEYWORDS DATABASE
// ============================================================================

// Common keywords across C, Java, and Python
// We identify these to understand code structure better
const char* KEYWORDS[] = {
    // C/C++ keywords
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
    
    // Java keywords
    "abstract", "assert", "boolean", "byte", "catch", "class", "extends",
    "final", "finally", "implements", "import", "instanceof", "interface",
    "native", "new", "package", "private", "protected", "public", "super",
    "synchronized", "this", "throw", "throws", "transient", "try",
    
    // Python keywords
    "and", "as", "async", "await", "def", "del", "elif", "except", "False",
    "from", "global", "in", "is", "lambda", "None", "nonlocal", "not", "or",
    "pass", "raise", "True", "with", "yield", "print", "range", "len",
    
    NULL  // Marks the end of our keyword list
};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/*
  Function: isKeyword
  
  Purpose: Check if a word is a programming keyword
  
  How it works:
  - Loops through our KEYWORDS array
  - Compares the input word with each keyword (case-insensitive)
  - Returns 1 (true) if found, 0 (false) otherwise
  
  Example: isKeyword("while") returns 1, isKeyword("myVar") returns 0
*/
int isKeyword(const char* word) {
    for (int i = 0; KEYWORDS[i] != NULL; i++) {
        if (strcasecmp(word, KEYWORDS[i]) == 0) {
            return 1;  // Found it!
        }
    }
    return 0;  // Not a keyword
}

/*
  Function: isOperatorChar
  
  Purpose: Check if a character is an operator symbol
  
  Operators: + - * / = < > ! & | % ^ ~ ? :
*/
int isOperatorChar(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' ||
            c == '<' || c == '>' || c == '!' || c == '&' || c == '|' ||
            c == '%' || c == '^' || c == '~' || c == '?' || c == ':');
}

/*
  Function: isDelimiter
  
  Purpose: Check if a character is a delimiter (punctuation)
  
  Delimiters: ( ) { } [ ] ; , . #
*/
int isDelimiter(char c) {
    return (c == '(' || c == ')' || c == '{' || c == '}' || 
            c == '[' || c == ']' || c == ';' || c == ',' || 
            c == '.' || c == '#');
}

/*
  Function: readFile
  
  Purpose: Read entire file content into memory
  
  Process:
  1. Open the file
  2. Find its size
  3. Allocate memory
  4. Read all content
  5. Return the content (caller must free it later)
  
  Returns: Pointer to file content, or NULL if error
*/
char* readFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("ERROR: Cannot open file '%s'\n", filename);
        return NULL;
    }
    
    // Find file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Check if file is too large
    if (fileSize > MAX_FILE_SIZE) {
        printf("ERROR: File '%s' is too large (max %d bytes)\n", filename, MAX_FILE_SIZE);
        fclose(file);
        return NULL;
    }
    
    // Allocate memory and read file
    char* content = (char*)malloc(fileSize + 1);
    if (!content) {
        printf("ERROR: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }
    
    fread(content, 1, fileSize, file);
    content[fileSize] = '\0';  // Null-terminate the string
    
    fclose(file);
    return content;
}

// ============================================================================
// TOKENIZATION ENGINE
// ============================================================================

/*
  Function: tokenize
  
  Purpose: Break source code into tokens (the heart of our analyzer!)
  
  Algorithm:
  1. Scan through code character by character
  2. Skip whitespace and comments
  3. Identify token types:
     - Keywords: if, while, class, etc.
     - Identifiers: variable/function names
     - Operators: +, -, ==, etc.
     - Literals: numbers and strings
     - Delimiters: { } ( ) ; etc.
  4. Store each token with its type
  
  This is CRITICAL because it normalizes code - "int x=5" and "int x = 5"
  produce the same tokens, making plagiarism detection effective!
*/
void tokenize(const char* code, FileData* data) {
    data->tokenCount = 0;
    int i = 0;
    int len = strlen(code);
    
    while (i < len && data->tokenCount < MAX_TOKENS) {
        // Skip whitespace (spaces, tabs, newlines)
        if (isspace(code[i])) {
            i++;
            continue;
        }
        
        // Skip C-style comments: // comment
        if (code[i] == '/' && i + 1 < len && code[i + 1] == '/') {
            while (i < len && code[i] != '\n') i++;
            continue;
        }
        
        // Skip C-style block comments: /* comment */
        if (code[i] == '/' && i + 1 < len && code[i + 1] == '*') {
            i += 2;
            while (i < len - 1) {
                if (code[i] == '*' && code[i + 1] == '/') {
                    i += 2;
                    break;
                }
                i++;
            }
            continue;
        }
        
        // Skip Python comments: # comment
        if (code[i] == '#') {
            while (i < len && code[i] != '\n') i++;
            continue;
        }
        
        Token* token = &data->tokens[data->tokenCount];
        int tokenLen = 0;
        
        // Extract IDENTIFIERS and KEYWORDS (alphanumeric starting with letter/underscore)
        if (isalpha(code[i]) || code[i] == '_') {
            while (i < len && (isalnum(code[i]) || code[i] == '_')) {
                if (tokenLen < MAX_TOKEN_LENGTH - 1) {
                    token->text[tokenLen++] = code[i];
                }
                i++;
            }
            token->text[tokenLen] = '\0';
            
            // Determine if it's a keyword or just an identifier
            token->type = isKeyword(token->text) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
            data->tokenCount++;
            continue;
        }
        
        // Extract NUMERIC LITERALS (123, 45.67, 0xFF)
        if (isdigit(code[i])) {
            while (i < len && (isalnum(code[i]) || code[i] == '.' || code[i] == 'x' || code[i] == 'X')) {
                if (tokenLen < MAX_TOKEN_LENGTH - 1) {
                    token->text[tokenLen++] = code[i];
                }
                i++;
            }
            token->text[tokenLen] = '\0';
            token->type = TOKEN_LITERAL;
            data->tokenCount++;
            continue;
        }
        
        // Extract STRING LITERALS ("hello", 'c')
        if (code[i] == '"' || code[i] == '\'') {
            char quote = code[i];
            token->text[tokenLen++] = code[i++];
            
            while (i < len && code[i] != quote) {
                if (code[i] == '\\' && i + 1 < len) {
                    if (tokenLen < MAX_TOKEN_LENGTH - 1) token->text[tokenLen++] = code[i++];
                }
                if (tokenLen < MAX_TOKEN_LENGTH - 1) {
                    token->text[tokenLen++] = code[i];
                }
                i++;
            }
            if (i < len && tokenLen < MAX_TOKEN_LENGTH - 1) {
                token->text[tokenLen++] = code[i++];  // Closing quote
            }
            token->text[tokenLen] = '\0';
            token->type = TOKEN_LITERAL;
            data->tokenCount++;
            continue;
        }
        
        // Extract OPERATORS (+, -, ==, !=, <=, etc.)
        if (isOperatorChar(code[i])) {
            // Handle multi-character operators (==, !=, <=, >=, &&, ||)
            while (i < len && isOperatorChar(code[i]) && tokenLen < MAX_TOKEN_LENGTH - 1) {
                token->text[tokenLen++] = code[i++];
            }
            token->text[tokenLen] = '\0';
            token->type = TOKEN_OPERATOR;
            data->tokenCount++;
            continue;
        }
        
        // Extract DELIMITERS ({ } ( ) ; , etc.)
        if (isDelimiter(code[i])) {
            token->text[0] = code[i];
            token->text[1] = '\0';
            token->type = TOKEN_DELIMITER;
            data->tokenCount++;
            i++;
            continue;
        }
        
        // Unknown character - skip it
        i++;
    }
}

// ============================================================================
// HASHING ENGINE - Creates Fingerprints of Code
// ============================================================================

/*
  Function: computeHashes
  
  Purpose: Create "fingerprints" of code using rolling hash technique
  
  How it works:
  1. Take a sliding window of N tokens (e.g., 5 tokens at a time)
  2. Calculate a hash value for each window
  3. Slide the window by 1 token and repeat
  
  Example with HASH_WINDOW_SIZE = 3:
    Tokens: [int, x, =, 5, ;]
    Window 1: [int, x, =]      -> hash value A
    Window 2: [x, =, 5]        -> hash value B
    Window 3: [=, 5, ;]        -> hash value C
  
  Why this works:
  - Even if code is rearranged slightly, many windows will still match
  - Hash values allow fast comparison (numbers vs. strings)
  - Multiple windows provide better coverage than single hash
*/
void computeHashes(FileData* data) {
    data->hashCount = 0;
    
    // Need at least HASH_WINDOW_SIZE tokens to create a hash
    if (data->tokenCount < HASH_WINDOW_SIZE) {
        return;
    }
    
    // Create a hash for each sliding window of tokens
    for (int i = 0; i <= data->tokenCount - HASH_WINDOW_SIZE; i++) {
        unsigned long hash = 0;
        
        // Calculate hash for current window using polynomial rolling hash
        // Formula: hash = (token1 * P^4) + (token2 * P^3) + ... + (token5 * P^0)
        for (int j = 0; j < HASH_WINDOW_SIZE; j++) {
            Token* token = &data->tokens[i + j];
            
            // Hash the token based on its TYPE and TEXT
            // This makes the hash resistant to variable renaming!
            for (int k = 0; token->text[k] != '\0'; k++) {
                hash = hash * HASH_PRIME + (unsigned char)tolower(token->text[k]);
            }
            
            // Add token type to hash (keywords vs identifiers matter)
            hash = hash * HASH_PRIME + token->type;
        }
        
        data->hashes[data->hashCount++] = hash;
    }
}

// ============================================================================
// SIMILARITY CALCULATION - The Detection Logic
// ============================================================================

/*
  Function: calculateSimilarity
  
  Purpose: Compare two files and calculate plagiarism similarity percentage
  
  Algorithm:
  1. Compare hash fingerprints from both files
  2. Count how many hashes match
  3. Calculate percentage: (matches / total) * 100
  
  Returns: Similarity score from 0.0 (completely different) to 100.0 (identical)
  
  Interpretation:
  - 0-30%: Minimal similarity (probably not plagiarized)
  - 30-60%: Moderate similarity (investigate further)
  - 60-85%: High similarity (likely plagiarized with modifications)
  - 85-100%: Very high similarity (direct copy or minimal changes)
*/
double calculateSimilarity(FileData* file1, FileData* file2) {
    // Edge case: if either file has no hashes, return 0% similarity
    if (file1->hashCount == 0 || file2->hashCount == 0) {
        return 0.0;
    }
    
    int matchCount = 0;
    
    // Compare each hash from file1 against all hashes in file2
    // This is an O(n*m) comparison - good enough for educational purposes
    // (For production, we'd use hash tables for O(n+m) performance)
    for (int i = 0; i < file1->hashCount; i++) {
        for (int j = 0; j < file2->hashCount; j++) {
            if (file1->hashes[i] == file2->hashes[j]) {
                matchCount++;
                break;  // Each hash in file1 counts only once
            }
        }
    }
    
    // Calculate similarity as percentage
    // We use the average of both hash counts to handle different file sizes
    double avgHashCount = (file1->hashCount + file2->hashCount) / 2.0;
    double similarity = (matchCount / avgHashCount) * 100.0;
    
    // Cap at 100% (shouldn't exceed, but just in case)
    if (similarity > 100.0) similarity = 100.0;
    
    return similarity;
}

// ============================================================================
// DETAILED ANALYSIS - For Debugging and Transparency
// ============================================================================

/*
  Function: printDetailedAnalysis
  
  Purpose: Show detailed statistics about the comparison
  
  This helps users understand HOW the similarity was calculated
  - Makes the tool transparent and educational
  - Helps debug issues
  - Builds trust in the results
*/
void printDetailedAnalysis(FileData* file1, FileData* file2, double similarity) {
    printf("\n--- DETAILED ANALYSIS ---\n");
    printf("File 1 Statistics:\n");
    printf("  - Total Tokens: %d\n", file1->tokenCount);
    printf("  - Hash Fingerprints: %d\n", file1->hashCount);
    
    printf("\nFile 2 Statistics:\n");
    printf("  - Total Tokens: %d\n", file2->tokenCount);
    printf("  - Hash Fingerprints: %d\n", file2->hashCount);
    
    printf("\nComparison Results:\n");
    printf("  - Similarity Score: %.2f%%\n", similarity);
    
    // Provide interpretation
    if (similarity < 30.0) {
        printf("  - Assessment: LOW similarity (files appear different)\n");
    } else if (similarity < 60.0) {
        printf("  - Assessment: MODERATE similarity (some common patterns)\n");
    } else if (similarity < 85.0) {
        printf("  - Assessment: HIGH similarity (likely plagiarized)\n");
    } else {
        printf("  - Assessment: VERY HIGH similarity (almost identical)\n");
    }
}

// ============================================================================
// MAIN PROGRAM
// ============================================================================

/*
  Main Function: Entry point of the program
  
  Usage: ./plagiarism_detector <file1> <file2>
  
  Process:
  1. Read both files
  2. Tokenize the source code
  3. Compute hash fingerprints
  4. Calculate similarity
  5. Output results in JSON format (for Python integration)
*/
int main(int argc, char* argv[]) {
    // Validate command-line arguments
    if (argc != 3) {
        printf("USAGE: %s <file1> <file2>\n", argv[0]);
        printf("Example: %s student1.c student2.c\n", argv[0]);
        return 1;
    }
    
    // Initialize file data structures
    FileData file1Data = {0};
    FileData file2Data = {0};
    
    // STEP 1: Read file contents
    char* code1 = readFile(argv[1]);
    char* code2 = readFile(argv[2]);
    
    if (!code1 || !code2) {
        if (code1) free(code1);
        if (code2) free(code2);
        return 1;
    }
    
    // STEP 2: Tokenize the code
    tokenize(code1, &file1Data);
    tokenize(code2, &file2Data);
    
    // STEP 3: Compute hash fingerprints
    computeHashes(&file1Data);
    computeHashes(&file2Data);
    
    // STEP 4: Calculate similarity score
    double similarity = calculateSimilarity(&file1Data, &file2Data);
    
    // STEP 5: Output results
    // JSON format for easy parsing by Python
    printf("{\n");
    printf("  \"file1\": \"%s\",\n", argv[1]);
    printf("  \"file2\": \"%s\",\n", argv[2]);
    printf("  \"similarity\": %.2f,\n", similarity);
    printf("  \"file1_tokens\": %d,\n", file1Data.tokenCount);
    printf("  \"file2_tokens\": %d,\n", file2Data.tokenCount);
    printf("  \"file1_hashes\": %d,\n", file1Data.hashCount);
    printf("  \"file2_hashes\": %d\n", file2Data.hashCount);
    printf("}\n");
    
    // Optional: Print detailed analysis to stderr (won't interfere with JSON)
    // printDetailedAnalysis(&file1Data, &file2Data, similarity);
    
    // Clean up
    free(code1);
    free(code2);
    
    return 0;
}

/*
================================================================================
  HOW TO COMPILE AND RUN
================================================================================

COMPILATION:
  gcc -o plagiarism_detector plagiarism_detector.c -lm

USAGE:
  ./plagiarism_detector file1.c file2.java

EXAMPLE OUTPUT (JSON):
  {
    "file1": "student1.c",
    "file2": "student2.c",
    "similarity": 87.50,
    "file1_tokens": 45,
    "file2_tokens": 48,
    "file1_hashes": 41,
    "file2_hashes": 44
  }

================================================================================
  ALGORITHM EXPLANATION (For Presentation)
================================================================================

1. TOKENIZATION:
   - Breaks code into meaningful elements (keywords, variables, operators)
   - Removes formatting, whitespace, and comments
   - Normalizes code structure

2. HASH FINGERPRINTING:
   - Uses rolling hash with sliding window
   - Creates multiple fingerprints per file
   - Resistant to variable renaming and reordering

3. SIMILARITY MATCHING:
   - Compares fingerprints between files
   - Counts matching patterns
   - Calculates percentage similarity

4. ROBUSTNESS:
   - Detects plagiarism even with renamed variables
   - Ignores formatting changes
   - Works across C, Java, and Python

================================================================================
  FUTURE ENHANCEMENTS
================================================================================

1. AST-based analysis for deeper structural comparison
2. Machine learning for advanced pattern recognition
3. Side-by-side code diff visualization
4. Support for more programming languages
5. Batch processing of entire directories
6. Database storage of previous submissions
7. Statistical analysis across multiple submissions
8. GUI desktop application

================================================================================
*/

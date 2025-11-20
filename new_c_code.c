/*
================================================================================
                    C CODE PLAGIARISM DETECTOR
================================================================================
Purpose: Compare two C source files and calculate similarity percentage
to detect potential plagiarism.

How it works:
1. Read two C files
2. Extract tokens (keywords, variables, operators, etc.)
3. Create hash fingerprints from token sequences
4. Compare fingerprints and calculate similarity score
5. Output the result

Compilation: gcc -o plagiarism plagiarism.c
Usage: ./plagiarism file1.c file2.c
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

#define MAX_FILE_SIZE 50000      // Maximum file size (50KB)
#define MAX_TOKENS 5000          // Maximum tokens per file
#define MAX_TOKEN_LEN 50         // Maximum token length
#define WINDOW_SIZE 4            // Tokens per hash window

// Token types to categorize code elements
typedef enum {
    TYPE_KEYWORD,     // if, while, for, int, void, etc.
    TYPE_IDENTIFIER,  // variable/function names
    TYPE_OPERATOR,    // +, -, *, /, ==, etc.
    TYPE_NUMBER,      // 123, 45.67, etc.
    TYPE_STRING,      // "hello", 'c'
    TYPE_SYMBOL       // { } ( ) ; , etc.
} TokenType;

// Token structure - stores each code element
typedef struct {
    char text[MAX_TOKEN_LEN];
    TokenType type;
} Token;

// File data - stores all tokens and hashes from one file
typedef struct {
    Token tokens[MAX_TOKENS];
    int tokenCount;
    unsigned long hashes[MAX_TOKENS];
    int hashCount;
} FileData;

// ============================================================================
// C KEYWORDS DATABASE
// ============================================================================

// Common C programming keywords
const char* C_KEYWORDS[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
    NULL  // End marker
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Check if a word is a C keyword
int isKeyword(const char* word) {
    for (int i = 0; C_KEYWORDS[i] != NULL; i++) {
        if (strcasecmp(word, C_KEYWORDS[i]) == 0) {
            return 1;  // Found keyword
        }
    }
    return 0;  // Not a keyword
}

// Check if character is an operator
int isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' ||
            c == '<' || c == '>' || c == '!' || c == '&' || c == '|' ||
            c == '%' || c == '^' || c == '~' || c == '?' || c == ':');
}

// Check if character is a symbol/delimiter
int isSymbol(char c) {
    return (c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ';' || c == ',' ||
            c == '.' || c == '#');
}

// Read entire file into memory
char* readFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("ERROR: Cannot open file '%s'\n", filename);
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size > MAX_FILE_SIZE) {
        printf("ERROR: File too large (max %d bytes)\n", MAX_FILE_SIZE);
        fclose(file);
        return NULL;
    }

    // Read file
    char* content = (char*)malloc(size + 1);
    fread(content, 1, size, file);
    content[size] = '\0';

    fclose(file);
    return content;
}

// ============================================================================
// TOKENIZATION - Breaking code into meaningful parts
// ============================================================================

/*
  Purpose: Convert source code into tokens
  
  Process:
  1. Skip whitespace and comments
  2. Identify token type (keyword, identifier, operator, etc.)
  3. Extract token text
  4. Store token with its type
  
  This allows us to compare code structure regardless of formatting
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

        // Skip single-line comments //
        if (code[i] == '/' && i + 1 < len && code[i + 1] == '/') {
            while (i < len && code[i] != '\n') i++;
            continue;
        }

        // Skip multi-line comments /* */
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

        Token* token = &data->tokens[data->tokenCount];
        int len_token = 0;

        // Extract IDENTIFIER or KEYWORD (starts with letter/underscore)
        if (isalpha(code[i]) || code[i] == '_') {
            while (i < len && (isalnum(code[i]) || code[i] == '_')) {
                if (len_token < MAX_TOKEN_LEN - 1) {
                    token->text[len_token++] = code[i];
                }
                i++;
            }
            token->text[len_token] = '\0';

            // Determine if keyword or identifier
            token->type = isKeyword(token->text) ? TYPE_KEYWORD : TYPE_IDENTIFIER;
            data->tokenCount++;
            continue;
        }

        // Extract NUMBER (digits, decimals, hex)
        if (isdigit(code[i])) {
            while (i < len && (isdigit(code[i]) || code[i] == '.' ||
                   code[i] == 'x' || code[i] == 'X')) {
                if (len_token < MAX_TOKEN_LEN - 1) {
                    token->text[len_token++] = code[i];
                }
                i++;
            }
            token->text[len_token] = '\0';
            token->type = TYPE_NUMBER;
            data->tokenCount++;
            continue;
        }

        // Extract STRING ("hello" or 'c')
        if (code[i] == '"' || code[i] == '\'') {
            char quote = code[i];
            token->text[len_token++] = code[i++];

            while (i < len && code[i] != quote) {
                // Handle escaped quotes
                if (code[i] == '\\' && i + 1 < len) {
                    if (len_token < MAX_TOKEN_LEN - 1) {
                        token->text[len_token++] = code[i++];
                    }
                }
                if (len_token < MAX_TOKEN_LEN - 1) {
                    token->text[len_token++] = code[i];
                }
                i++;
            }
            if (i < len && len_token < MAX_TOKEN_LEN - 1) {
                token->text[len_token++] = code[i++];  // Closing quote
            }
            token->text[len_token] = '\0';
            token->type = TYPE_STRING;
            data->tokenCount++;
            continue;
        }

        // Extract OPERATOR (+, -, ==, !=, etc.)
        if (isOperator(code[i])) {
            while (i < len && isOperator(code[i]) && len_token < MAX_TOKEN_LEN - 1) {
                token->text[len_token++] = code[i++];
            }
            token->text[len_token] = '\0';
            token->type = TYPE_OPERATOR;
            data->tokenCount++;
            continue;
        }

        // Extract SYMBOL ({ } ( ) ; , etc.)
        if (isSymbol(code[i])) {
            token->text[0] = code[i];
            token->text[1] = '\0';
            token->type = TYPE_SYMBOL;
            data->tokenCount++;
            i++;
            continue;
        }

        // Unknown character - skip
        i++;
    }
}

// ============================================================================
// HASHING - Creating fingerprints of code
// ============================================================================

/*
  Purpose: Create hash fingerprints to identify similar code patterns
  
  How it works:
  - Take sliding windows of consecutive tokens
  - Calculate a hash value for each window
  - Move window by 1 position and repeat
  
  Example:
    Tokens: [int] [x] [=] [5] [;]
    Window 1: [int][x][=][5]      -> hash1
    Window 2: [x][=][5][;]        -> hash2
  
  Why: Even if code is rearranged, many hash patterns will match
*/
void computeHashes(FileData* data) {
    data->hashCount = 0;

    // Need minimum tokens to create hashes
    if (data->tokenCount < WINDOW_SIZE) {
        return;
    }

    // Create hash for each sliding window
    for (int i = 0; i <= data->tokenCount - WINDOW_SIZE; i++) {
        unsigned long hash = 0;

        // Calculate hash for current window
        for (int j = 0; j < WINDOW_SIZE; j++) {
            Token* token = &data->tokens[i + j];

            // Hash the token text (case-insensitive)
            for (int k = 0; token->text[k] != '\0'; k++) {
                hash = hash * 31 + (unsigned char)tolower(token->text[k]);
            }

            // Add token type to hash
            hash = hash * 31 + token->type;
        }

        data->hashes[data->hashCount++] = hash;
    }
}

// ============================================================================
// SIMILARITY CALCULATION - The main detection logic
// ============================================================================

/*
  Purpose: Calculate how similar two files are
  
  Algorithm:
  1. Compare hashes from both files
  2. Count how many match
  3. Calculate percentage: (matches / total) * 100
  
  Returns: Similarity score from 0.0 to 100.0
  
  Interpretation:
  - 0-20%:   Not similar (different programs)
  - 20-40%:  Low similarity (might use same library/template)
  - 40-70%:  Medium similarity (suspicious)
  - 70-100%: High similarity (likely plagiarism)
*/
double calculateSimilarity(FileData* file1, FileData* file2) {
    // If either file has no hashes, return 0%
    if (file1->hashCount == 0 || file2->hashCount == 0) {
        return 0.0;
    }

    int matches = 0;

    // Compare each hash in file1 with all hashes in file2
    for (int i = 0; i < file1->hashCount; i++) {
        for (int j = 0; j < file2->hashCount; j++) {
            if (file1->hashes[i] == file2->hashes[j]) {
                matches++;
                break;  // Count each file1 hash only once
            }
        }
    }

    // Calculate percentage
    // Use average of hash counts to handle different file sizes
    double avgHashes = (file1->hashCount + file2->hashCount) / 2.0;
    double similarity = (matches / avgHashes) * 100.0;

    // Ensure result doesn't exceed 100%
    if (similarity > 100.0) similarity = 100.0;

    return similarity;
}

// ============================================================================
// RESULTS AND INTERPRETATION
// ============================================================================

// Print results in a readable format
void printResults(const char* file1, const char* file2, 
                  FileData* data1, FileData* data2, double similarity) {
    printf("\n");
    printf("========================================\n");
    printf("   C CODE PLAGIARISM DETECTION REPORT\n");
    printf("========================================\n\n");

    printf("File 1: %s\n", file1);
    printf("  - Tokens: %d\n", data1->tokenCount);
    printf("  - Hash Patterns: %d\n\n", data1->hashCount);

    printf("File 2: %s\n", file2);
    printf("  - Tokens: %d\n", data2->tokenCount);
    printf("  - Hash Patterns: %d\n\n", data2->hashCount);

    printf("SIMILARITY SCORE: %.2f%%\n\n", similarity);

    // Interpretation
    if (similarity < 20.0) {
        printf("RESULT: Not similar - Different programs\n");
    } else if (similarity < 40.0) {
        printf("RESULT: Low similarity - Minimal overlap\n");
    } else if (similarity < 70.0) {
        printf("RESULT: Medium similarity - Investigate further\n");
    } else {
        printf("RESULT: High similarity - Likely plagiarism\n");
    }

    printf("========================================\n\n");
}

// ============================================================================
// MAIN PROGRAM
// ============================================================================

int main(int argc, char* argv[]) {
    // Check command-line arguments
    if (argc != 3) {
        printf("USAGE: %s <file1.c> <file2.c>\n", argv[0]);
        printf("Example: %s student1.c student2.c\n", argv[0]);
        return 1;
    }

    // Initialize file data structures
    FileData data1 = {0};
    FileData data2 = {0};

    printf("Reading files...\n");

    // STEP 1: Read both files
    char* code1 = readFile(argv[1]);
    char* code2 = readFile(argv[2]);

    if (!code1 || !code2) {
        if (code1) free(code1);
        if (code2) free(code2);
        return 1;
    }

    printf("Tokenizing code...\n");

    // STEP 2: Convert code to tokens
    tokenize(code1, &data1);
    tokenize(code2, &data2);

    printf("Computing hashes...\n");

    // STEP 3: Create hash fingerprints
    computeHashes(&data1);
    computeHashes(&data2);

    printf("Comparing files...\n");

    // STEP 4: Calculate similarity
    double similarity = calculateSimilarity(&data1, &data2);

    // STEP 5: Display results
    printResults(argv[1], argv[2], &data1, &data2, similarity);

    // Clean up memory
    free(code1);
    free(code2);

    return 0;
}

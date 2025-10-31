# 🔍 Intelligent Code Plagiarism Detector

A powerful yet beginner-friendly plagiarism detection system that compares multiple code submissions and detects similarities—even when students rename variables, rearrange functions, or change formatting.

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![Language](https://img.shields.io/badge/C-00599C?logo=c&logoColor=white)
![Python](https://img.shields.io/badge/Python-3776AB?logo=python&logoColor=white)
![License](https://img.shields.io/badge/license-MIT-green.svg)

---

## 📋 Table of Contents

- [What It Does](#-what-it-does)
- [How It Works](#-how-it-works)
- [Features](#-features)
- [Installation](#-installation)
- [Usage](#-usage)
- [Project Structure](#-project-structure)
- [Algorithm Explanation](#-algorithm-explanation)
- [Example Output](#-example-output)
- [Future Enhancements](#-future-enhancements)
- [For Presentation](#-for-presentation)

---

## 🎯 What It Does

This system analyzes multiple code files (C, Java, Python) and detects plagiarism by:

1. **Tokenizing** code into meaningful elements (keywords, identifiers, operators)
2. **Normalizing** structure by removing comments, whitespace, and formatting
3. **Hashing** code chunks using rolling hash algorithm
4. **Comparing** fingerprints to calculate similarity scores

The detector is **smart enough** to catch plagiarism even when students:
- ✅ Rename variables (`int x` → `int myVariable`)
- ✅ Change formatting (spacing, indentation, line breaks)
- ✅ Rearrange function order
- ✅ Add/remove comments
- ✅ Use different naming conventions

---

## 🧠 How It Works

### Architecture Overview

```
┌─────────────────┐
│  Web Interface  │  (HTML/CSS/JavaScript)
│   File Upload   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Flask Backend  │  (Python)
│   Coordinates   │
│   Comparisons   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  C Engine       │  (C Language)
│  Tokenization   │
│  Hashing        │
│  Similarity     │
└─────────────────┘
```

### Detection Pipeline

```
Input Code
    │
    ▼
[TOKENIZATION] → Break into keywords, identifiers, operators
    │
    ▼
[NORMALIZATION] → Remove formatting, comments, whitespace
    │
    ▼
[HASHING] → Create fingerprints with rolling hash (window=5)
    │
    ▼
[COMPARISON] → Match fingerprints between files
    │
    ▼
[OUTPUT] → Similarity score (0-100%)
```

---

## ✨ Features

### Core Features
- 🔍 **Multi-file comparison** - Upload and compare multiple files at once
- 📊 **Visual heatmap** - Color-coded similarity matrix
- 📈 **Detailed reports** - Token counts and analysis statistics
- 🎨 **Beautiful UI** - Modern, responsive web interface
- ⚡ **Fast processing** - Efficient C-based engine
- 🌐 **Multi-language support** - C, C++, Java, Python

### Smart Detection
- **Token-based analysis** - Ignores superficial changes
- **Rolling hash fingerprints** - Detects code patterns
- **Resistance to obfuscation** - Catches renamed variables
- **Structural comparison** - Identifies similar logic flow

### Similarity Levels
- 🟢 **0-30%** - Low similarity (probably original)
- 🟡 **30-60%** - Moderate similarity (common patterns)
- 🟠 **60-85%** - High similarity (likely plagiarized)
- 🔴 **85-100%** - Very high similarity (almost identical)

---

## 🚀 Installation

### Prerequisites

```bash
# 1. GCC Compiler (for C engine)
gcc --version

# 2. Python 3.x
python3 --version

# 3. Flask (Python web framework)
pip install flask
```

### Setup

```bash
# 1. Clone or download the project
cd plagiarism-detector

# 2. Install Python dependencies
pip install flask

# 3. Compile the C detector
gcc -o plagiarism_detector plagiarism_detector.c -lm

# 4. Run the application
python app.py
```

---

## 💻 Usage

### Web Interface (Recommended)

1. **Start the server:**
   ```bash
   python app.py
   ```

2. **Open your browser:**
   ```
   http://localhost:5000
   ```

3. **Upload files:**
   - Click "Choose Files" or drag & drop
   - Select 2 or more code files (.c, .cpp, .java, .py)
   - Click "Analyze Files"

4. **View results:**
   - Color-coded heatmap shows all pairwise comparisons
   - Detailed report lists similarity scores
   - Assessment provides interpretation

### Command Line (C Engine Only)

```bash
# Compile
gcc -o plagiarism_detector plagiarism_detector.c -lm

# Run comparison
./plagiarism_detector file1.c file2.c

# Output (JSON format)
{
  "file1": "file1.c",
  "file2": "file2.c",
  "similarity": 87.50,
  "file1_tokens": 45,
  "file2_tokens": 48,
  "file1_hashes": 41,
  "file2_hashes": 44
}
```

### Example Test Cases

The project includes 4 example files demonstrating different similarity levels:

```bash
# Use the web interface "Load Example" button
# Or run manually:

./plagiarism_detector examples/original.c examples/high_plagiarism.c
# Expected: ~85-95% similarity

./plagiarism_detector examples/original.c examples/medium_plagiarism.c
# Expected: ~50-70% similarity

./plagiarism_detector examples/original.c examples/low_plagiarism.c
# Expected: ~20-40% similarity
```

---

## 📁 Project Structure

```
plagiarism-detector/
│
├── plagiarism_detector.c    # Core C engine (tokenization, hashing)
├── plagiarism_detector       # Compiled C executable
│
├── app.py                    # Flask web application
├── templates/
│   └── index.html           # Web interface (HTML/CSS/JS)
│
├── examples/                 # Test cases
│   ├── original.c           # Original submission
│   ├── high_plagiarism.c    # ~90% similar (renamed variables)
│   ├── medium_plagiarism.c  # ~60% similar (restructured)
│   └── low_plagiarism.c     # ~30% similar (different approach)
│
├── uploads/                  # Temporary upload directory
│
└── README.md                # This file
```

---

## 🧮 Algorithm Explanation

### 1. Tokenization

**Purpose:** Break code into meaningful elements

**Process:**
```c
Input:  int x = 5;
Tokens: [KEYWORD:int] [IDENTIFIER:x] [OPERATOR:=] [LITERAL:5] [DELIMITER:;]
```

**Why it matters:** Different formatting produces same tokens:
```c
int x=5;        // Same tokens
int x = 5;      // Same tokens
int    x=   5;  // Same tokens
```

### 2. Hash Fingerprinting

**Purpose:** Create unique identifiers for code patterns

**Technique:** Rolling hash with sliding window (size = 5 tokens)

**Example:**
```
Tokens: [int, x, =, 5, ;, int, y, =, 10, ;]

Window 1: [int, x, =, 5, ;]      → Hash: 0x1A2B3C4D
Window 2: [x, =, 5, ;, int]      → Hash: 0x2B3C4D5E
Window 3: [=, 5, ;, int, y]      → Hash: 0x3C4D5E6F
...
```

**Formula:**
```c
hash = (token1 * P^4) + (token2 * P^3) + ... + (token5 * P^0)
where P = 31 (prime number)
```

### 3. Similarity Calculation

**Purpose:** Compare hash fingerprints between two files

**Algorithm:**
```python
matches = 0
for hash1 in file1.hashes:
    if hash1 in file2.hashes:
        matches += 1

avg_hash_count = (len(file1.hashes) + len(file2.hashes)) / 2
similarity = (matches / avg_hash_count) * 100
```

**Why it works:**
- Renamed variables still produce similar hash patterns
- Reordered code has many matching windows
- Different code has few/no matching hashes

---

## 📊 Example Output

### Web Interface

![Example Screenshot](docs/screenshot.png)

**Heatmap:**
```
                original.c  high_plag.c  medium_plag.c  low_plag.c
original.c         100%        92.3%         58.7%        28.4%
high_plag.c        92.3%       100%          61.2%        31.5%
medium_plag.c      58.7%       61.2%         100%         45.8%
low_plag.c         28.4%       31.5%         45.8%        100%
```

### Console Output

```json
{
  "file1": "examples/original.c",
  "file2": "examples/high_plagiarism.c",
  "similarity": 92.34,
  "file1_tokens": 156,
  "file2_tokens": 158,
  "file1_hashes": 152,
  "file2_hashes": 154
}
```

**Assessment:** VERY HIGH similarity (almost identical)

---

## 🔮 Future Enhancements

### Short-term (Easy to implement)
1. **Side-by-side diff viewer** - Show matching code sections
2. **PDF report generation** - Download detailed analysis
3. **Batch directory processing** - Analyze entire folders
4. **More languages** - JavaScript, C++, Go support
5. **Threshold configuration** - Customize similarity levels

### Medium-term (Requires more work)
6. **AST-based comparison** - Parse syntax trees for deeper analysis
7. **Code normalization** - Convert to canonical form
8. **Database storage** - Save results and history
9. **User authentication** - Multi-user support
10. **API endpoints** - RESTful API for integration

### Long-term (Advanced features)
11. **Machine Learning** - Train model on known plagiarism cases
12. **Cross-language detection** - Compare C vs Java vs Python
13. **Semantic analysis** - Understand code meaning, not just structure
14. **Real-time collaboration** - Live comparison updates
15. **Desktop GUI** - Standalone application with Electron

---

## 🎓 For Presentation

### Key Talking Points

1. **Problem Statement**
   - Plagiarism is common in programming courses
   - Students try to hide it by renaming variables
   - Manual detection is time-consuming and unreliable

2. **Our Solution**
   - Token-based analysis ignores superficial changes
   - Rolling hash detects code patterns efficiently
   - Visual heatmap makes results easy to understand

3. **Technical Highlights**
   - **C engine:** Fast, efficient, uses only standard libraries
   - **Python backend:** Orchestrates comparisons, handles uploads
   - **Web interface:** Modern, responsive, user-friendly
   - **Smart detection:** Resistant to variable renaming and reordering

4. **Live Demo**
   - Upload example files showing high/medium/low similarity
   - Show heatmap visualization
   - Explain similarity scores and assessments
   - Demonstrate detection of renamed variable plagiarism

5. **Advantages**
   - ✅ Fast and efficient (C-based engine)
   - ✅ Accurate (token-based analysis)
   - ✅ User-friendly (web interface)
   - ✅ Extensible (easy to add features)
   - ✅ Educational (well-documented code)

### Demo Script

```
1. "Let me show you our Intelligent Code Plagiarism Detector..."
2. Upload original.c and high_plagiarism.c
3. "Notice these files look different - different variable names, comments..."
4. Click "Analyze Files"
5. "But our system detects 92% similarity! Here's the heatmap..."
6. "The C engine tokenized the code, created fingerprints, and matched patterns"
7. "Even though variables were renamed, the structure is identical"
8. Upload all 4 example files
9. "Now we see the full comparison matrix - color-coded for easy interpretation"
10. "This tool can help professors catch plagiarism automatically"
```

---

## 📝 Credits

**Project:** Intelligent Code Plagiarism Detector  
**Author:** AI Code Plagiarism Detection System  
**Date:** 2025  
**Purpose:** Educational project demonstrating plagiarism detection techniques  

**Technologies Used:**
- C (core detection engine)
- Python + Flask (web application)
- HTML/CSS/JavaScript (user interface)
- Rolling hash algorithm
- Tokenization techniques

---

## 📄 License

MIT License - Feel free to use this for educational purposes!

---

## 🤝 Contributing

Ideas for improvement:
1. Fork the project
2. Create a feature branch
3. Make your changes
4. Submit a pull request

---

## ❓ FAQ

**Q: Can it detect plagiarism across different languages?**  
A: Currently it works best within the same language, but the tokenization approach allows some cross-language detection.

**Q: How accurate is it?**  
A: Very accurate for direct code copying with modifications. May produce false positives for very simple programs (e.g., "Hello World").

**Q: Can students fool it?**  
A: It's resistant to simple obfuscation (renaming, reformatting), but sophisticated rewrites or algorithmic changes will reduce similarity scores.

**Q: How many files can I compare?**  
A: No hard limit! The system performs N*(N-1)/2 comparisons for N files.

**Q: Is it suitable for production use?**  
A: This is an educational project. For production, consider additional features like database storage, authentication, and performance optimization.

---

## 📞 Support

For questions or issues:
1. Check the code comments (heavily documented!)
2. Review this README
3. Examine the example files
4. Trace through the algorithm step-by-step

**Good luck with your presentation!** 🚀

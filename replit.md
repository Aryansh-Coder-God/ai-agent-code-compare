# Intelligent Code Plagiarism Detector

## Project Overview
An educational plagiarism detection system built with C and Python that analyzes code submissions and detects similarities even when students rename variables, rearrange functions, or change formatting.

## Purpose
- **Educational Tool**: Demonstrate plagiarism detection techniques using tokenization and hashing
- **College-Level Project**: Perfect for presentations showcasing C + Python integration
- **Real-World Application**: Helps detect code plagiarism in programming courses

## Current State
✅ **Fully Functional** - The system is complete and working:
- C-based detection engine compiled and operational
- Flask web application running on port 5000
- Interactive web interface with file upload
- Visual heatmap showing similarity scores
- Example test cases demonstrating different plagiarism levels

## Technology Stack
- **Backend**: Python 3.11 + Flask
- **Core Engine**: C (compiled with GCC)
- **Frontend**: HTML5, CSS3, JavaScript
- **Algorithm**: Tokenization + Rolling Hash Fingerprinting

## Project Architecture

### Core Components
1. **plagiarism_detector.c** - C engine that performs:
   - Code tokenization (breaks code into keywords, identifiers, operators)
   - Hash fingerprinting (rolling hash with 5-token windows)
   - Similarity calculation (pairwise comparison)

2. **app.py** - Flask backend that:
   - Handles file uploads
   - Calls C engine for comparisons
   - Generates similarity matrix
   - Serves web interface

3. **templates/index.html** - Web UI featuring:
   - Drag-and-drop file upload
   - Color-coded heatmap visualization
   - Detailed comparison reports

4. **examples/** - Test cases:
   - original.c (baseline)
   - high_plagiarism.c (~90% similar - renamed variables)
   - medium_plagiarism.c (~60% similar - restructured)
   - low_plagiarism.c (~30% similar - different approach)

## How to Use
1. Access the web interface at the preview URL
2. Upload 2+ code files (.c, .cpp, .java, .py)
3. Click "Analyze Files" to run detection
4. View similarity heatmap and detailed reports
5. Or click "Load Example" to see demo with test files

## Key Features
- **Multi-file comparison**: Upload multiple files for pairwise analysis
- **Smart detection**: Detects plagiarism despite variable renaming
- **Visual reports**: Color-coded heatmap (red=high similarity, green=low)
- **Detailed statistics**: Token counts and hash fingerprints
- **Multi-language support**: C, C++, Java, Python

## Algorithm Overview

### 1. Tokenization
Breaks code into meaningful tokens while removing formatting:
```
Input:  int x = 5;
Tokens: [KEYWORD:int] [IDENTIFIER:x] [OPERATOR:=] [LITERAL:5]
```

### 2. Hash Fingerprinting
Creates fingerprints using rolling hash (5-token sliding window):
```
Tokens: [int, x, =, 5, ;, int, y, =, 10, ;]
Hash1:  [int, x, =, 5, ;]      → 0x1A2B3C4D
Hash2:  [x, =, 5, ;, int]      → 0x2B3C4D5E
Hash3:  [=, 5, ;, int, y]      → 0x3C4D5E6F
```

### 3. Similarity Calculation
Compares hash fingerprints between files:
```
similarity = (matching_hashes / average_hash_count) * 100%
```

## Similarity Interpretation
- **0-30%**: Low similarity (files appear different)
- **30-60%**: Moderate similarity (some common patterns)
- **60-85%**: High similarity (likely plagiarized)
- **85-100%**: Very high similarity (almost identical)

## File Structure
```
project/
├── plagiarism_detector.c    # C detection engine
├── app.py                    # Flask web application
├── templates/
│   └── index.html           # Web interface
├── examples/                 # Test cases
│   ├── original.c
│   ├── high_plagiarism.c
│   ├── medium_plagiarism.c
│   └── low_plagiarism.c
├── uploads/                  # Temporary upload directory
└── README.md                # Full documentation
```

## Recent Changes
**2025-10-31**: Initial project creation
- Implemented C tokenization engine with rolling hash
- Created Flask backend with file upload handling
- Built interactive web interface with heatmap visualization
- Generated comprehensive example test cases
- Added extensive inline documentation for learning

## User Preferences
- **Code Style**: Heavily commented for educational purposes
- **Documentation**: Beginner-friendly explanations throughout
- **Purpose**: College-level presentation project

## Dependencies
- Python 3.11
- Flask 3.1.2
- GCC compiler
- Standard C libraries only (stdio.h, string.h, stdlib.h, ctype.h)

## Future Enhancements
Potential improvements for extending the project:
1. AST-based structural analysis
2. Side-by-side code diff viewer
3. PDF report generation
4. Machine learning for advanced detection
5. Cross-language comparison improvements
6. Database storage for submission history
7. User authentication and multi-user support
8. API endpoints for integration with LMS systems

## Notes for Presentation
- Emphasize the token-based approach (resistant to renaming)
- Demonstrate with example files showing different similarity levels
- Explain rolling hash algorithm with visual diagrams
- Highlight clean, modular code structure
- Show live demo of web interface
- Discuss real-world applications in education

## Development Status
✅ **Complete and Ready for Presentation**
- All core features implemented
- Comprehensive documentation provided
- Example test cases included
- Web interface fully functional
- Code heavily commented for learning

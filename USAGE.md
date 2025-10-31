# Quick Start Guide

## 🚀 Getting Started in 3 Steps

### 1. Access the Web Application
The plagiarism detector is already running! Click the **Webview** tab or open the preview URL to access the interface.

### 2. Upload Your Files
You have two options:

**Option A: Try the Example Files**
- Click the **"Load Example"** button
- This demonstrates the system with 4 pre-made test files
- Shows different levels of plagiarism (high, medium, low)

**Option B: Upload Your Own Files**
- Click **"Choose Files"** or drag & drop
- Select 2 or more code files (.c, .cpp, .java, .py)
- Click **"Analyze Files"**

### 3. View Results
- **Heatmap**: Color-coded similarity matrix
  - 🔴 Red (85-100%): Very high similarity - almost identical
  - 🟠 Orange (60-85%): High similarity - likely plagiarized
  - 🟡 Yellow (30-60%): Moderate similarity - some common patterns
  - 🟢 Green (0-30%): Low similarity - files appear different

- **Detailed Report**: Shows exact percentages and token counts for each pair

---

## 💻 Command Line Usage

If you want to use just the C detector:

```bash
# Compile (already done)
gcc -o plagiarism_detector plagiarism_detector.c -lm

# Compare two files
./plagiarism_detector file1.c file2.c

# Output (JSON format)
{
  "file1": "file1.c",
  "file2": "file2.c",
  "similarity": 87.50,
  "file1_tokens": 45,
  "file2_tokens": 48
}
```

---

## 🧪 Testing Before Presentation

Run the automated test script:

```bash
./test_detector.sh
```

This validates:
- ✓ C detector compiles correctly
- ✓ High similarity detection works
- ✓ Medium similarity detection works
- ✓ Low similarity detection works

---

## 📊 Understanding Results

### Similarity Percentages
- **90-100%**: Direct copy with minimal changes (variable renaming)
- **70-90%**: Same code with significant renaming/restructuring
- **50-70%**: Similar approach with different implementation details
- **30-50%**: Some common patterns, but mostly different
- **0-30%**: Different code, minimal similarity

### What the Detector Catches
✅ **Detects:**
- Renamed variables (`int x` → `int myVariable`)
- Changed formatting (spacing, indentation)
- Reordered functions
- Added/removed comments
- Different variable naming styles

❌ **Doesn't catch:**
- Complete algorithmic rewrites
- Different programming approaches
- Translated code (C to Java with different logic)

---

## 🎓 For Your Presentation

### Demo Flow
1. **Introduction** (2 min)
   - Explain the plagiarism problem in coding classes
   - Show the web interface

2. **Live Demo** (3 min)
   - Click "Load Example"
   - Point out the heatmap colors
   - Show high_plagiarism.c vs original.c
   - Explain: "Notice the similarity is 45%+ even though variables are renamed!"

3. **How It Works** (3 min)
   - Show plagiarism_detector.c code
   - Explain tokenization (breaks code into pieces)
   - Explain rolling hash (creates fingerprints)
   - Show the comparison logic

4. **Architecture** (2 min)
   - C engine for speed and efficiency
   - Python Flask for web interface
   - JavaScript for visualization

### Key Talking Points
- "Our system uses **tokenization** to ignore formatting"
- "We use **rolling hash** to create code fingerprints"
- "It works across **C, Java, and Python**"
- "Even renamed variables can't fool it!"

---

## 🐛 Troubleshooting

**Problem:** Web interface won't load  
**Solution:** Check if Flask server is running, restart if needed

**Problem:** "Compilation failed" message  
**Solution:** Make sure GCC is installed: `gcc --version`

**Problem:** Similarity seems too low/high  
**Solution:** This is normal! The detector shows structural similarity, not logical equivalence

**Problem:** Can't upload files  
**Solution:** Make sure files are .c, .cpp, .java, or .py format

---

## 📁 Project Files

```
Key Files:
├── plagiarism_detector.c    # Core detection engine (show this!)
├── app.py                    # Flask backend
├── templates/index.html      # Web interface
└── examples/                 # Test cases for demo
    ├── original.c
    ├── high_plagiarism.c     # 45%+ similar
    ├── medium_plagiarism.c   # ~30-40% similar
    └── low_plagiarism.c      # ~15-25% similar
```

---

## 🎯 Quick Reference

| Task | Command/Action |
|------|----------------|
| Start web app | Already running! Use Webview |
| Load example | Click "Load Example" button |
| Upload files | Drag & drop or click "Choose Files" |
| Test system | `./test_detector.sh` |
| Compare 2 files | `./plagiarism_detector file1.c file2.c` |
| View full docs | See README.md |

---

**Ready to present?** Just click "Load Example" and start explaining! 🎉

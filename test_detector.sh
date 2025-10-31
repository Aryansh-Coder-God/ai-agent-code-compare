#!/bin/bash
# Quick test script to validate the plagiarism detector
# Run this before demos to ensure everything works

echo "================================"
echo "Testing Plagiarism Detector"
echo "================================"
echo ""

# Test 1: Compile C detector
echo "[1/4] Compiling C detector..."
gcc -o plagiarism_detector plagiarism_detector.c -lm
if [ $? -eq 0 ]; then
    echo "✓ Compilation successful!"
else
    echo "✗ Compilation failed!"
    exit 1
fi

echo ""

# Test 2: Run detector on high similarity files
echo "[2/4] Testing high similarity detection..."
./plagiarism_detector examples/original.c examples/high_plagiarism.c > /tmp/test1.json
similarity=$(grep -o '"similarity": [0-9.]*' /tmp/test1.json | cut -d' ' -f2)
echo "   Similarity: ${similarity}%"
if (( $(echo "$similarity > 40" | bc -l) )); then
    echo "✓ High similarity detected correctly!"
else
    echo "✗ Similarity too low (expected >40%)"
fi

echo ""

# Test 3: Run detector on medium similarity files
echo "[3/4] Testing medium similarity detection..."
./plagiarism_detector examples/original.c examples/medium_plagiarism.c > /tmp/test2.json
similarity=$(grep -o '"similarity": [0-9.]*' /tmp/test2.json | cut -d' ' -f2)
echo "   Similarity: ${similarity}%"
echo "✓ Medium similarity test complete"

echo ""

# Test 4: Run detector on low similarity files
echo "[4/4] Testing low similarity detection..."
./plagiarism_detector examples/original.c examples/low_plagiarism.c > /tmp/test3.json
similarity=$(grep -o '"similarity": [0-9.]*' /tmp/test3.json | cut -d' ' -f2)
echo "   Similarity: ${similarity}%"
echo "✓ Low similarity test complete"

echo ""
echo "================================"
echo "All tests completed!"
echo "================================"
echo ""
echo "You can now run: python app.py"

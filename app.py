"""
================================================================================
  INTELLIGENT CODE PLAGIARISM DETECTOR - Web Application (Python + Flask)
================================================================================

  PURPOSE:
  This Flask web application provides a user-friendly interface for the
  plagiarism detection system. It:
  
  1. Accepts multiple code file uploads via web interface
  2. Calls the C detection engine for pairwise comparisons
  3. Generates similarity reports with visualization
  4. Displays results in an interactive heatmap
  
  ARCHITECTURE:
  - Flask backend handles HTTP requests and file management
  - C engine performs actual plagiarism detection
  - JSON communication between Python and C
  - Interactive HTML/JavaScript frontend for visualization
  
  AUTHOR: AI Code Plagiarism Detection System
  DATE: 2025
================================================================================
"""

from flask import Flask, render_template, request, jsonify, send_from_directory
import os
import subprocess
import json
from werkzeug.utils import secure_filename
import shutil

# ============================================================================
# APPLICATION CONFIGURATION
# ============================================================================

app = Flask(__name__)

# Security: limit upload file size to 5MB
app.config['MAX_CONTENT_LENGTH'] = 5 * 1024 * 1024

# File upload configuration
UPLOAD_FOLDER = 'uploads'
ALLOWED_EXTENSIONS = {'c', 'cpp', 'java', 'py', 'h', 'hpp'}

# Ensure upload directory exists
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

# Path to compiled C program
C_DETECTOR = './plagiarism_detector'

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

def allowed_file(filename):
    """
    Check if uploaded file has an allowed extension.
    
    Args:
        filename (str): Name of the uploaded file
    
    Returns:
        bool: True if file extension is allowed, False otherwise
    
    Example:
        allowed_file('student1.c') -> True
        allowed_file('document.pdf') -> False
    """
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS


def compile_c_detector():
    """
    Compile the C plagiarism detector program.
    
    This function is called on startup to ensure the C engine is ready.
    Uses GCC compiler with standard flags.
    
    Returns:
        bool: True if compilation successful, False otherwise
    """
    try:
        print("Compiling C plagiarism detector...")
        result = subprocess.run(
            ['gcc', '-o', 'plagiarism_detector', 'plagiarism_detector.c', '-lm'],
            capture_output=True,
            text=True,
            timeout=30
        )
        
        if result.returncode == 0:
            print("✓ C detector compiled successfully!")
            return True
        else:
            print(f"✗ Compilation failed: {result.stderr}")
            return False
            
    except Exception as e:
        print(f"✗ Compilation error: {e}")
        return False


def run_plagiarism_check(file1_path, file2_path):
    """
    Run the C plagiarism detector on two files.
    
    This function:
    1. Calls the compiled C program with two file paths
    2. Captures the JSON output
    3. Parses and returns the results
    
    Args:
        file1_path (str): Path to first code file
        file2_path (str): Path to second code file
    
    Returns:
        dict: Parsed JSON result with similarity score and statistics
              Returns None if execution fails
    
    Example result:
        {
            "file1": "uploads/student1.c",
            "file2": "uploads/student2.c",
            "similarity": 87.50,
            "file1_tokens": 45,
            "file2_tokens": 48,
            "file1_hashes": 41,
            "file2_hashes": 44
        }
    """
    try:
        # Execute the C program with file paths as arguments
        result = subprocess.run(
            [C_DETECTOR, file1_path, file2_path],
            capture_output=True,
            text=True,
            timeout=10
        )
        
        if result.returncode == 0:
            # Parse JSON output from C program
            output = result.stdout.strip()
            data = json.loads(output)
            return data
        else:
            print(f"Detector error: {result.stderr}")
            return None
            
    except subprocess.TimeoutExpired:
        print("Detector timeout - file too large or infinite loop")
        return None
    except json.JSONDecodeError as e:
        print(f"JSON parse error: {e}")
        if 'result' in locals():
            print(f"Output was: {result.stdout}")
        return None
    except Exception as e:
        print(f"Execution error: {e}")
        return None


def compare_all_files(file_paths):
    """
    Compare all uploaded files pairwise.
    
    For N files, this performs N*(N-1)/2 comparisons.
    Creates a similarity matrix for visualization.
    
    Args:
        file_paths (list): List of file paths to compare
    
    Returns:
        dict: Contains:
            - files: List of filenames
            - comparisons: List of all pairwise comparison results
            - matrix: 2D similarity matrix for heatmap
    
    Example:
        Input: ['file1.c', 'file2.c', 'file3.c']
        Performs: (file1 vs file2), (file1 vs file3), (file2 vs file3)
    """
    n = len(file_paths)
    filenames = [os.path.basename(path) for path in file_paths]
    
    # Initialize results structure
    results = {
        'files': filenames,
        'comparisons': [],
        'matrix': [[0.0 for _ in range(n)] for _ in range(n)]
    }
    
    # Perform pairwise comparisons
    for i in range(n):
        for j in range(i + 1, n):
            # Run C detector on this pair
            comparison = run_plagiarism_check(file_paths[i], file_paths[j])
            
            if comparison:
                # Extract similarity score
                similarity = comparison.get('similarity', 0.0)
                
                # Store in results
                results['comparisons'].append({
                    'file1': filenames[i],
                    'file2': filenames[j],
                    'similarity': similarity,
                    'file1_tokens': comparison.get('file1_tokens', 0),
                    'file2_tokens': comparison.get('file2_tokens', 0)
                })
                
                # Update similarity matrix (symmetric)
                results['matrix'][i][j] = similarity
                results['matrix'][j][i] = similarity
    
    # Diagonal elements are 100% (file compared to itself)
    for i in range(n):
        results['matrix'][i][i] = 100.0
    
    return results


# ============================================================================
# WEB ROUTES
# ============================================================================

@app.route('/')
def index():
    """
    Home page route.
    
    Displays the main upload interface where users can:
    - Upload multiple code files
    - Start plagiarism detection
    - View results
    """
    return render_template('index.html')


@app.route('/upload', methods=['POST'])
def upload_files():
    """
    File upload endpoint.
    
    Handles multi-file upload via HTML form.
    Processes files and runs plagiarism detection.
    
    Process:
    1. Validate uploaded files
    2. Save to upload directory
    3. Run pairwise comparisons
    4. Return JSON results
    
    Returns:
        JSON response with:
        - success: boolean
        - results: comparison data and matrix
        - error: error message if failed
    """
    try:
        # Check if files were uploaded
        if 'files[]' not in request.files:
            return jsonify({'success': False, 'error': 'No files uploaded'})
        
        files = request.files.getlist('files[]')
        
        # Validate minimum number of files
        if len(files) < 2:
            return jsonify({'success': False, 'error': 'Please upload at least 2 files'})
        
        # Clear previous uploads
        for filename in os.listdir(UPLOAD_FOLDER):
            file_path = os.path.join(UPLOAD_FOLDER, filename)
            try:
                if os.path.isfile(file_path):
                    os.unlink(file_path)
            except Exception as e:
                print(f"Error deleting {file_path}: {e}")
        
        # Save uploaded files
        saved_files = []
        for file in files:
            if file and file.filename and allowed_file(file.filename):
                # Secure the filename to prevent directory traversal attacks
                filename = secure_filename(file.filename)
                filepath = os.path.join(UPLOAD_FOLDER, filename)
                file.save(filepath)
                saved_files.append(filepath)
        
        # Validate we have at least 2 valid files
        if len(saved_files) < 2:
            return jsonify({
                'success': False,
                'error': 'At least 2 valid code files required (.c, .cpp, .java, .py)'
            })
        
        # Run plagiarism detection on all pairs
        print(f"Analyzing {len(saved_files)} files...")
        results = compare_all_files(saved_files)
        
        # Sort comparisons by similarity (highest first)
        results['comparisons'].sort(key=lambda x: x['similarity'], reverse=True)
        
        return jsonify({'success': True, 'results': results})
        
    except Exception as e:
        print(f"Upload error: {e}")
        return jsonify({'success': False, 'error': str(e)})


@app.route('/example')
def load_example():
    """
    Load example test cases.
    
    Demonstrates the system with pre-created sample files showing
    various levels of plagiarism (high, medium, low similarity).
    
    Returns:
        JSON response with example comparison results
    """
    try:
        # Check if example files exist
        example_files = [
            'examples/original.c',
            'examples/high_plagiarism.c',
            'examples/medium_plagiarism.c',
            'examples/low_plagiarism.c'
        ]
        
        existing_files = [f for f in example_files if os.path.exists(f)]
        
        if len(existing_files) < 2:
            return jsonify({
                'success': False,
                'error': 'Example files not found. Please create them first.'
            })
        
        # Run comparison on example files
        results = compare_all_files(existing_files)
        results['comparisons'].sort(key=lambda x: x['similarity'], reverse=True)
        
        return jsonify({'success': True, 'results': results})
        
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)})


# ============================================================================
# APPLICATION STARTUP
# ============================================================================

if __name__ == '__main__':
    print("=" * 80)
    print("  INTELLIGENT CODE PLAGIARISM DETECTOR")
    print("  Initializing web application...")
    print("=" * 80)
    
    # Compile C detector on startup
    if not compile_c_detector():
        print("\n⚠ WARNING: C detector compilation failed!")
        print("The web app will start, but plagiarism detection won't work.")
        print("Please check that gcc is installed and plagiarism_detector.c exists.\n")
    
    # Create example directory
    os.makedirs('examples', exist_ok=True)
    
    print("\n✓ Server starting...")
    print("✓ Access the application at: http://0.0.0.0:5000")
    print("✓ Upload code files and view plagiarism reports\n")
    
    # Run Flask server
    # host='0.0.0.0' makes it accessible from outside the container
    # port=5000 is the standard Replit web preview port
    app.run(host='0.0.0.0', port=5000, debug=True)

"""
================================================================================
  HOW TO RUN THE APPLICATION
================================================================================

PREREQUISITES:
  1. GCC compiler installed (for compiling C detector)
  2. Python 3.x installed
  3. Flask installed (pip install flask)

STARTUP:
  python app.py

ACCESS:
  Open browser to: http://localhost:5000

USAGE:
  1. Click "Choose Files" button
  2. Select 2 or more code files (.c, .cpp, .java, .py)
  3. Click "Analyze Files"
  4. View similarity heatmap and detailed reports

EXAMPLE TEST:
  - Create sample files in examples/ directory
  - Use "Load Example" button to test with pre-made cases

================================================================================
  API ENDPOINTS
================================================================================

GET /
  - Home page with upload interface

POST /upload
  - Upload multiple files for comparison
  - Returns JSON with similarity matrix and detailed results

GET /example
  - Load and analyze example test cases
  - Demonstrates different plagiarism levels

================================================================================
  FILE STRUCTURE
================================================================================

project/
├── app.py                      # This file - Flask web application
├── plagiarism_detector.c       # C detection engine
├── plagiarism_detector         # Compiled C program
├── templates/
│   └── index.html             # Web interface
├── uploads/                   # Temporary upload directory
└── examples/                  # Example test cases
    ├── original.c
    ├── high_plagiarism.c
    ├── medium_plagiarism.c
    └── low_plagiarism.c

================================================================================
"""

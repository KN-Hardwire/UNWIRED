import os
import sys
import re

def lint_project_file(file_path):
    """Checks for absolute paths in Altium .PrjPcb files."""
    errors = []
    # Match Windows absolute paths (e.g., C:\ or D:\)
    abs_path_pattern = re.compile(r'[a-zA-Z]:\\')
    
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            for line_num, line in enumerate(f, 1):
                if abs_path_pattern.search(line):
                    errors.append(f"Line {line_num}: Absolute path detected -> {line.strip()}")
    except Exception as e:
        return [f"Error reading file: {str(e)}"]
    
    return errors

def check_pdf_requirements(changed_files):
    """Enforces PDF updates for schematic and PCB changes."""
    errors = []
    
    # Identify which PCB directories had changes
    for file in changed_files:
        if file.endswith('.SchDoc'):
            pdf_path = os.path.join(os.path.dirname(file), 'schematic.pdf')
            if pdf_path not in changed_files:
                errors.append(f"Missing documentation: {file} was modified, but {pdf_path} was not updated in this PR.")
        
        if file.endswith('.PcbDoc'):
            pdf_path = os.path.join(os.path.dirname(file), 'pcb.pdf')
            if pdf_path not in changed_files:
                errors.append(f"Missing documentation: {file} was modified, but {pdf_path} was not updated in this PR.")
                
    return errors

def main():
    # Expects list of changed files from environment or arguments
    changed_files = sys.argv[1:]
    exit_code = 0

    print("--- Hardware Guard: Starting Verification ---")

    # 1. Check for Absolute Paths in all .PrjPcb files in the repo
    print("\n[1/2] Linting Project Files...")
    for root, dirs, files in os.walk('.'):
        for file in files:
            if file.endswith('.PrjPcb'):
                path = os.path.join(root, file)
                print(f"Checking: {path}")
                lint_errors = lint_project_file(path)
                if lint_errors:
                    print(f"  FAILED: Absolute paths found in {path}:")
                    for err in lint_errors:
                        print(f"    - {err}")
                    exit_code = 1
                else:
                    print(f"  PASSED: {path} is clean.")

    # 2. Check PDF Requirements for changed hardware files
    print("\n[2/2] Verifying Documentation Exports...")
    if not changed_files:
        print("No files changed in this PR.")
    else:
        pdf_errors = check_pdf_requirements(changed_files)
        if pdf_errors:
            print("FAILED: Documentation requirements not met:")
            for err in pdf_errors:
                print(f"  - {err}")
            exit_code = 1
        else:
            print("PASSED: All modified designs have corresponding PDF updates.")

    if exit_code != 0:
        print("\n--- Result: FAILED ---")
        print("Please fix the issues above and commit the changes.")
    else:
        print("\n--- Result: SUCCESS ---")

    sys.exit(exit_code)

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
Script to compile all .ts files to .qm files and generate updated translations.qrc
"""

import os
import subprocess
import glob
import sys

def find_lrelease():
    """Find lrelease executable, preferring Qt 6 over Qt 5."""
    import glob
    
    search_paths = []
    
    # System PATH first (most reliable)
    system_paths = [
        "lrelease",      # Unix/Linux or Windows with Qt in PATH
        "lrelease-qt6",  # Some Linux distributions
        "lrelease.exe",  # Windows fallback
    ]
    search_paths.extend(system_paths)
    
    # Windows: Search Qt installation directories dynamically
    if os.name == 'nt':
        qt_base_dirs = [r"C:\Qt", r"C:\Qt5", r"C:\Qt6"]
        
        for base_dir in qt_base_dirs:
            if os.path.exists(base_dir):
                # Find all Qt versions and compilers
                lrelease_paths = glob.glob(os.path.join(base_dir, "*", "*", "bin", "lrelease.exe"))
                
                # Sort to prefer Qt 6 over Qt 5, and newer versions first
                lrelease_paths.sort(key=lambda x: (
                    0 if "\\6." in x else 1,  # Qt 6 first
                    -float(x.split("\\")[-3].replace(".", "")) if x.split("\\")[-3].replace(".", "").isdigit() else 0  # Newer versions first
                ), reverse=False)
                
                search_paths.extend(lrelease_paths)
    
    # Unix/Linux: Common package installation paths
    else:
        unix_paths = [
            "/usr/bin/lrelease-qt6",
            "/usr/bin/lrelease",
            "/usr/lib/qt6/bin/lrelease",
            "/usr/lib/x86_64-linux-gnu/qt6/bin/lrelease",
            "/usr/lib/qt5/bin/lrelease",
            "/usr/lib/x86_64-linux-gnu/qt5/bin/lrelease",
            "/opt/Qt/*/gcc_64/bin/lrelease",
            "/opt/Qt/*/bin/lrelease",
        ]
        
        # Expand wildcards for Unix paths
        for pattern in unix_paths:
            if "*" in pattern:
                search_paths.extend(glob.glob(pattern))
            else:
                search_paths.append(pattern)
    
    # Try Qt 5 fallback on Unix systems
    search_paths.append("lrelease-qt5")
    
    for path in search_paths:
        try:
            result = subprocess.run([path, "-version"], 
                                  capture_output=True, text=True, timeout=5)
            if result.returncode == 0:
                version_output = result.stdout + result.stderr
                print(f"Found lrelease at: {path}")
                if "Qt 6" in version_output:
                    print("  Using Qt 6 (preferred)")
                elif "Qt 5" in version_output:
                    print("  Using Qt 5")
                return path
        except (FileNotFoundError, subprocess.TimeoutExpired, PermissionError):
            continue
    
    return None

def compile_translations():
    """Compile all .ts files to .qm files."""
    lrelease = find_lrelease()
    if not lrelease:
        print("ERROR: lrelease not found. Please ensure Qt tools are installed and in PATH.")
        return False
    
    ts_files = glob.glob("*.ts")
    if not ts_files:
        print("No .ts files found.")
        return False
    
    success_count = 0
    for ts_file in ts_files:
        try:
            result = subprocess.run([lrelease, ts_file], 
                                  capture_output=True, text=True, timeout=30)
            if result.returncode == 0:
                print(f"✓ Compiled {ts_file}")
                success_count += 1
            else:
                print(f"✗ Failed to compile {ts_file}: {result.stderr}")
        except subprocess.TimeoutExpired:
            print(f"✗ Timeout compiling {ts_file}")
        except Exception as e:
            print(f"✗ Error compiling {ts_file}: {e}")
    
    print(f"\nCompiled {success_count}/{len(ts_files)} translation files.")
    return success_count > 0

def generate_qrc():
    """Generate translations.qrc based on available .qm files."""
    qm_files = glob.glob("*.qm")
    qm_files.sort()
    
    if not qm_files:
        print("No .qm files found.")
        return False
    
    qrc_content = ['<RCC>', '    <qresource prefix="/translations">']
    
    for qm_file in qm_files:
        qrc_content.append(f'        <file>{qm_file}</file>')
    
    qrc_content.extend(['    </qresource>', '</RCC>'])
    
    with open('translations.qrc', 'w', encoding='utf-8') as f:
        f.write('\n'.join(qrc_content) + '\n')
    
    print(f"Generated translations.qrc with {len(qm_files)} files:")
    for qm_file in qm_files:
        print(f"  - {qm_file}")
    
    return True

def main():
    import sys
    
    # Check for --qrc-only flag
    if len(sys.argv) > 1 and sys.argv[1] == '--qrc-only':
        print("Generating translations.qrc file only...")
        if generate_qrc():
            print("translations.qrc generated successfully!")
        else:
            print("Failed to generate translations.qrc!")
            sys.exit(1)
        return
    
    print("Starting translation compilation process...")
    
    # Compile .ts files to .qm files
    if compile_translations():
        # Generate updated .qrc file
        generate_qrc()
        print("\nTranslation compilation completed successfully!")
    else:
        print("\nTranslation compilation failed!")
        sys.exit(1)

if __name__ == '__main__':
    main()

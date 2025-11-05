$vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
$vcvarsPath = "$vsPath\VC\Auxiliary\Build\vcvars64.bat"

# Set up MSVC environment
cmd /c "`"$vcvarsPath`" && set" | ForEach-Object {
    if ($_ -match "^(.*?)=(.*)$") {
        [Environment]::SetEnvironmentVariable($matches[1], $matches[2])
    }
}

# Verify environment
Write-Host "Checking environment setup..."
Write-Host "CL.exe path: $(where.exe cl.exe 2>&1)"

# Clean and configure
Write-Host "Cleaning build directory..."
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
Remove-Item -Force CMakeCache.txt -ErrorAction SilentlyContinue

cd C:\Users\Torres\Documents\GitHub\wiRedPanda

Write-Host "Configuring cmake..."
cmake --preset windows-qt

Write-Host "Building..."
cmake --build --preset windows-qt --target wiredpanda 2>&1 | Tee-Object -FilePath build_output.txt

@echo off
setlocal enabledelayedexpansion

REM Setup MSVC environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

REM Configure and build
cd /d C:\Users\Torres\Documents\GitHub\wiRedPanda
cmake --preset windows-qt
cmake --build --preset windows-qt --target wiredpanda

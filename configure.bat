@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cd /d C:\Users\Torres\Documents\GitHub\wiRedPanda
cmake --preset windows-qt
echo CMAKE Configure Complete
pause

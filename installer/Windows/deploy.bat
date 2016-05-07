SET PATH=%PATH%;C:\Qt\5.5\mingw492_32\bin\;C:\Program Files\7-Zip;C:\Program Files (x86)\NSIS\
if not exist Build mkdir Build
cd Build
qmake.exe ..\..\..\app\
mingw32-make.exe -j4
cd ..
"windeployqt.exe" "Build\release\wpanda.exe"
"makensis.exe" wpanda.nsi
7z.exe x wpanda_32.exe -o"%cd%\wpanda_32"
7z.exe a -tzip wpanda_portable.zip "%cd%\wpanda_32"
RMDIR /S /Q "%cd%\wpanda_32"
pause

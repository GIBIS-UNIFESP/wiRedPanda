SET PATH=%PATH%;C:\Qt\Tools\mingw530_32\bin;C:\Qt\5.8\mingw53_32\bin;C:\Program Files\7-Zip;C:\Program Files (x86)\NSIS\
del WiredPanda_*

if not exist Build mkdir Build
cd Build
qmake.exe ..\..\..\app\
mingw32-make.exe -j
cd ..

windeployqt.exe "Build\release\wpanda.exe"
makensis.exe "wpanda.nsi"
7z.exe x WiredPanda_*.exe -o"%cd%\wpanda_32"

FOR /F "tokens=*" %%i IN ( 'DIR /b /a-d WiredPanda_*.exe') DO (SET EXEFILE=%%i)
ECHO %EXEFILE%
set OUTFILE=%EXEFILE:Windows_Installer_x86_64.exe=Windows_Portable_x86_64.zip%

7z.exe a -tzip %OUTFILE% "%cd%\wpanda_32"
RMDIR /S /Q "%cd%\wpanda_32"

pause

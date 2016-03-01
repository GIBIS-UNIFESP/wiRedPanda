;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;Include File Association Library
  !include "FileAssociation.nsh"

;--------------------------------
;General

  ;Name and file
  !define MUI_PRODUCT "Wired Panda"
  !define MUI_FILE "wpanda"
  # These three must be integers
  !define VERSIONMAJOR 1
  !define VERSIONMINOR 9
  !define VERSIONBUILD 0001
  !define MUI_VERSION "${VERSIONMAJOR}.{VERSIONMINOR}"
  !define MUI_BRANDINGTEXT "${MUI_PRODUCT} ${MUI_VERSION}"
  !define COMPANYNAME "UNIFESP"
  !define DESCRIPTION "WiRed Panda is a software designed to help students to learn about logic circuits and simulate them in an easy and friendly way."

  !define BUILD_DIR "C:\Build\build-WPanda-Desktop_Qt_5_5_1_MinGW_32bit-Release\app\release"
  CRCCheck On
  Name "${MUI_PRODUCT}"
  Icon "logo.ico"
  # These will be displayed by the "Click here for support information" link in "Add/Remove Programs"
  # It is possible to use "mailto:" links in here to open the email client
  !define HELPURL "https://github.com/GIBIS-UNIFESP/wiRedPanda/issues/" # "Support Information" link
  !define UPDATEURL "https://github.com/GIBIS-UNIFESP/wiRedPanda/" # "Product Updates" link
  !define ABOUTURL "https://github.com/GIBIS-UNIFESP/wiRedPanda/" # "Publisher" link
  # This is the size (in kB) of all the files copied into "Program Files"
  !define INSTALLSIZE 40332

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\${MUI_PRODUCT}" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

  OutFile "wpanda_32.exe"
  ShowInstDetails "nevershow"
  ShowUninstDetails "nevershow"
  ;SetCompressor "bzip2"

  !define MUI_ICON "logo.ico"
  !define MUI_UNICON "logo.ico"
  ###### !define MUI_SPECIALBITMAP "Bitmap.bmp"

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Folder selection page

  InstallDir "$PROGRAMFILES\${MUI_PRODUCT}"

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "..\LICENSE"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections


;--------------------------------
;Installer Sections
Section "Wired Panda" Installationinfo

;Add files
  SetOutPath "$INSTDIR"

  File "${BUILD_DIR}\${MUI_FILE}.exe"
  File "logo.ico"
  ; File "${MUI_FILE}.ini"
  File "..\LICENSE"
  File "${BUILD_DIR}\*.dll"
  SetOutPath "$INSTDIR\iconengines"
  File "${BUILD_DIR}\iconengines\qsvgicon.dll"
  SetOutPath "$INSTDIR\imageformats"
  File "${BUILD_DIR}\imageformats\*.dll"
  SetOutPath "$INSTDIR\platforms"
  File "${BUILD_DIR}\platforms\*.dll"
  SetOutPath "$INSTDIR\translations"
  File "${BUILD_DIR}\translations\*.qm"

  ; hier komen dan nog de bestanden die in de playlist staan
  #SetOutPath "$INSTDIR"
  #file /r mpg
  SetOutPath "$INSTDIR"
  #file /r xtras

;create desktop shortcut
  CreateShortCut "$DESKTOP\${MUI_PRODUCT}.lnk" "$INSTDIR\${MUI_FILE}.exe" "" "$INSTDIR\logo.ico" 0

;create start-menu items
  CreateDirectory "$SMPROGRAMS\${MUI_PRODUCT}"
  CreateShortCut "$SMPROGRAMS\${MUI_PRODUCT}\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\${MUI_PRODUCT}\${MUI_PRODUCT}.lnk" "$INSTDIR\${MUI_FILE}.exe" "" "$INSTDIR\logo.ico" 0


;Register .panda File Association
  ${registerExtension} "$INSTDIR\${MUI_FILE}.exe" ".panda" "WiRED Panda File"


;write uninstall information to the registry
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "DisplayName" "${MUI_PRODUCT} (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "QuietUninstallString" "$INSTDIR\uninstall.exe /S"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "DisplayIcon" "$INSTDIR\logo.ico"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "Publisher" "${COMPANYNAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "HelpLink" "${HELPURL}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "URLUpdateInfo" "${UPDATEURL}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "URLInfoAbout" "${ABOUTURL}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "DisplayVersion" "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "VersionMajor" ${VERSIONMAJOR}
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "VersionMinor" ${VERSIONMINOR}

	# There is no option for modifying or repairing the install
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "NoRepair" 1
	# Set the INSTALLSIZE constant (!defined at the top of this script) so Add/Remove Programs can accurately report the size
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "EstimatedSize" ${INSTALLSIZE}
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_InstInfo ${LANG_ENGLISH} "Install Wired Panda."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${Installationinfo} $(DESC_InstInfo)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

;--------------------------------
;Uninstaller Section
Section "Uninstall"

;Delete Files
  RMDir /r "$INSTDIR\*.*"

;Remove the installation directory
  RMDir "$INSTDIR"

;Delete Start Menu Shortcuts
  Delete "$DESKTOP\${MUI_PRODUCT}.lnk"
  Delete "$SMPROGRAMS\${MUI_PRODUCT}\*.*"
  RmDir  "$SMPROGRAMS\${MUI_PRODUCT}"

;Delete Uninstaller And Unistall Registry Entries
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${MUI_PRODUCT}"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}"
  ${unregisterExtension} ".panda" "WiRED Panda File"
SectionEnd


;--------------------------------
;MessageBox Section


;Function that calls a messagebox when installation finished correctly
Function .onInstSuccess
  MessageBox MB_OK "You have successfully installed ${MUI_PRODUCT}. Use the desktop icon to start the program."
FunctionEnd


Function un.onUninstSuccess
  MessageBox MB_OK "You have successfully uninstalled ${MUI_PRODUCT}."
FunctionEnd


;eof

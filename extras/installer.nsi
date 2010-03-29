; © 2010 David Given.
; LBW is licensed under the MIT open source license. See the COPYING
; file in this distribution for the full text.

!include MUI2.nsh
	
Name "LBW"
OutFile "${OUTFILE}"

InstallDir "$PROGRAMFILES\Cowlark Technologies\LBW"

InstallDirRegKey HKLM "Software\Cowlark Technologies\LBW" \
	"InstallationDirectory"

RequestExecutionLevel admin
SetCompressor /solid lzma

;--------------------------------

!define MUI_WELCOMEPAGE_TITLE "LBW ${VERSION}"
!define MUI_WELCOMEPAGE_TEXT "LBW is a system for running unmodified Linux \
	binaries on Windows.$\r$\n\
	$\r$\n\
	WARNING. LBW is currently very, very immature! It is riddled with bugs. \
	Do not use LBW if you value your data!$\r$\n\
	$\r$\n\
	NOTE. LBW uses Interix, a.k.a. Services for Unix, a.k.a. Subsystem for \
	Unix Applications. If you don't have this installed, LBW won't work.$\r$\n\
	$\r$\n\
	This wizard will install LBW and a simple BusyBox-based shell on your \
	computer.$\r$\n\
	$\r$\n\
	$_CLICK"
	
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp"
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME

!define MUI_COMPONENTSPAGE_NODESC
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE "Installation complete"
!define MUI_FINISHPAGE_TEXT "LBW is now ready to use.$\r$\n\
	$\r$\n\
	You did remember the warning about the bugs? Beware!"
	
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

;--------------------------------

; The stuff to install
Section "LBW (required)"
	SectionIn RO
	SetOutPath $INSTDIR
	File "bin\lbw"
	File "extras\icon.ico"
	File "extras\interix.termcap"
	File "LICENSE"
	File "README"
	  
	; Write the installation path into the registry
	WriteRegStr HKLM SOFTWARE\NSIS_LBW "Install_Dir" "$INSTDIR"
	  
	; Write the uninstall keys for Windows
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LBW" "DisplayName" "LBW"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LBW" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LBW" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LBW" "NoRepair" 1
	WriteUninstaller "uninstall.exe"
	
	; Create shortcuts.
	CreateDirectory "$SMPROGRAMS\LBW"
	CreateShortCut "$SMPROGRAMS\LBW\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
	CreateShortCut "$SMPROGRAMS\LBW\LBW website.lnk" "http://lbw.sourceforge.net" "" "$INSTDIR\icon.ico" 0 SW_SHOWNORMAL "" "Go to the LBW website"	
	CreateShortCut "$SMPROGRAMS\LBW\README.lnk" "notepad.exe" "$INSTDIR\README" "$INSTDIR\icon.ico" 0 SW_SHOWNORMAL "" "Read the release notes"	
SectionEnd

Section "BusyBox shell"
	SetOutPath $INSTDIR
	File "extras\busybox"
	File "bin\busyboxshell.sh"
	File "extras\BusyBox.txt"
	CreateDirectory "$SMPROGRAMS\LBW"
	CreateShortCut "$SMPROGRAMS\LBW\BusyBox shell.lnk" "posix.exe" "/c lbw busybox ash busyboxshell.sh" "$INSTDIR\icon.ico" 0 SW_SHOWNORMAL "" "Start a minimal shell using BusyBox"
SectionEnd
	
;--------------------------------

; Uninstaller

Section "Uninstall"
	; Remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LBW"
	DeleteRegKey HKLM SOFTWARE\NSIS_LBW

	; Remove files and uninstaller
	Delete "$INSTDIR\lbw"
	Delete "$INSTDIR\icon.ico"
	Delete "$INSTDIR\interix.termcap"
	Delete "$INSTDIR\LICENSE"
	Delete "$INSTDIR\README"
	Delete "$INSTDIR\busybox"
	Delete "$INSTDIR\busyboxshell.sh"
	Delete "$INSTDIR\BusyBox.txt"
	Delete "$INSTDIR\core" ; just in case

	; Remove shortcuts, if any
	Delete "$SMPROGRAMS\LBW\*.*"

	; Remove directories used
	RMDir "$SMPROGRAMS\LBW"
	RMDir "$INSTDIR"
SectionEnd

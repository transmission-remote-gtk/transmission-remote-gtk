!include "MUI2.nsh"
;!include "FileAssociation.nsh"
;!include "ProtocolAssociation.nsh"
!include "x64.nsh"

;--------------------------------

; The name of the installer
Name "Transmission Remote GTK"

; The file to write
!ifndef REV
OutFile "transmission-remote-gtk-1.1-installer.exe"
!else
OutFile "transmission-remote-gtk-${REV}-installer.exe"
!endif

; The default installation directory
!define ProgramFilesDir "Transmission Remote GTK"

RequestExecutionLevel admin

;--------------------------------

XPStyle on

Var StartMenuFolder

!define MUI_ICON "transmission_large.ico"
!define MUI_UNICON "transmission_large.ico"
!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_BITMAP "logo.bmp"
;!define MUI_WELCOMEFINISHPAGE_BITMAP "nsis_wizard.bmp"
;!define MUI_UNWELCOMEFINISHPAGE_BITMAP "nsis_wizard.bmp"
;!define MUI_COMPONENTSPAGE_CHECKBITMAP "${NSISDIR}\Contrib\Graphics\Checks\colorful.bmp"
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_ABORTWARNING

;!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.txt"
;!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show ReadMe"
;!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_RESERVEFILE_LANGDLL

; English
LangString NAME_SecTransmissionRemoteGTK ${LANG_ENGLISH} "Transmission Remote GTK (required)"
LangString DESC_SecTransmissionRemoteGTK ${LANG_ENGLISH} "The application."
LangString NAME_GeoIP ${LANG_ENGLISH} "GeoIP database"
LangString DESC_GeoIP ${LANG_ENGLISH} "Shows the country of origin for a peer"
LangString NAME_SecGlibGtkEtc ${LANG_ENGLISH} "Glib, GTK, and other dependencies (recommended)."
LangString DESC_SecGlibGtkEtc ${LANG_ENGLISH} "If unset, you'll need to install these yourself."
LangString NAME_SecDesktopIcon ${LANG_ENGLISH} "Create icon on desktop"
LangString DESC_SecDesktopIcon ${LANG_ENGLISH} "If set, a shortcut for Transmission Remote will be created on the desktop."
;LangString NAME_SecFiletypeAssociations ${LANG_ENGLISH} "Register Filetype Associations"
;LangString DESC_SecFiletypeAssociations ${LANG_ENGLISH} "Register Associations to Transmission Remote"
;LangString NAME_SecRegiterTorrent ${LANG_ENGLISH} "Register .torrent"
;LangString DESC_SecRegiterTorrent ${LANG_ENGLISH} "Register .torrent to Transmission Remote"
;LangString NAME_SecRegiterMagnet ${LANG_ENGLISH} "Register Magnet URI"
;LangString DESC_SecRegiterMagnet ${LANG_ENGLISH} "Register Magnet URI to Transmission Remote"
;LangString DESC_SecGeoIPDatabase ${LANG_ENGLISH} "GeoIP database"
;LangString NAME_SecLanguages ${LANG_ENGLISH} "Languages"
;LangString DESC_SecLanguages ${LANG_ENGLISH} "Languages for Transmission Remote"

;--------------------------------

; The stuff to install
Section $(NAME_SecTransmissionRemoteGTK) SecTransmissionRemoteGTK
  SectionIn RO
  
  SetOutPath $INSTDIR
  
  File /oname=README.TXT "..\README"
  File /oname=COPYING.TXT "..\COPYING"
  File /oname=ChangeLog.TXT "..\ChangeLog"
  File /oname=AUTHORS.TXT "..\AUTHORS"

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR\bin
  
  ; Put file there
  File ".libs\transmission-remote-gtk.exe"
  
  #SetOutPath $INSTDIR\etc\gtk-2.0
  
  #File "C:\MinGW\msys\1.0\etc\gtk-2.0\gtkrc"
  
  #SetOutPath $INSTDIR\share\themes\MS-Windows\gtk-2.0
  
  #File "C:\MinGW\msys\1.0\share\themes\MS-Windows\gtk-2.0\gtkrc"
  
  SetOutPath $INSTDIR\share\locale\uk\LC_MESSAGES
  
  File "C:\MinGW\msys\1.0\lib\locale\uk\LC_MESSAGES\transmission-remote-gtk.mo"
  
  SetOutPath $INSTDIR\share\locale\fr\LC_MESSAGES
  
  File "C:\MinGW\msys\1.0\lib\locale\fr\LC_MESSAGES\transmission-remote-gtk.mo"

  SetOutPath $INSTDIR\share\locale\ru\LC_MESSAGES
  
  File "C:\MinGW\msys\1.0\lib\locale\ru\LC_MESSAGES\transmission-remote-gtk.mo"

  SetOutPath $INSTDIR\share\locale\pl\LC_MESSAGES

  File "C:\MinGW\msys\1.0\lib\locale\pl\LC_MESSAGES\transmission-remote-gtk.mo"

  SetOutPath $INSTDIR\share\locale\ko\LC_MESSAGES

  File "C:\MinGW\msys\1.0\lib\locale\ko\LC_MESSAGES\transmission-remote-gtk.mo"

  SetOutPath $INSTDIR\share\locale\es\LC_MESSAGES

  File "C:\MinGW\msys\1.0\lib\locale\es\LC_MESSAGES\transmission-remote-gtk.mo"

  SetOutPath $INSTDIR\share\locale\de\LC_MESSAGES

  File "C:\MinGW\msys\1.0\lib\locale\de\LC_MESSAGES\transmission-remote-gtk.mo"

  SetOutPath $INSTDIR\share\locale\lt\LC_MESSAGES

  File "C:\MinGW\msys\1.0\lib\locale\lt\LC_MESSAGES\transmission-remote-gtk.mo"

  SetOutPath $INSTDIR\share\icons\hicolor\scalable\apps

  File "C:\MinGW\msys\1.0\share\icons\hicolor\scalable\apps\transmission-remote-gtk.svg"

  SetOutPath $INSTDIR\share\icons\hicolor\48x48\apps

  File "C:\MinGW\msys\1.0\share\icons\hicolor\48x48\apps\transmission-remote-gtk.png"

  SetOutPath $INSTDIR\share\icons\hicolor\32x32\apps

  File "C:\MinGW\msys\1.0\share\icons\hicolor\32x32\apps\transmission-remote-gtk.png"

  SetOutPath $INSTDIR\share\icons\hicolor\24x24\apps

  File "C:\MinGW\msys\1.0\share\icons\hicolor\24x24\apps\transmission-remote-gtk.png"

  SetOutPath $INSTDIR\share\icons\hicolor\22x22\apps

  File "C:\MinGW\msys\1.0\share\icons\hicolor\22x22\apps\transmission-remote-gtk.png"

  SetOutPath $INSTDIR\share\icons\hicolor\16x16\apps

  File "C:\MinGW\msys\1.0\share\icons\hicolor\16x16\apps\transmission-remote-gtk.png"

  SetOutPath $INSTDIR\lib\gtk-2.0\2.10.0\engines
	
  File "C:\MinGW\msys\1.0\lib\gtk-2.0\2.10.0\engines\libpixmap.dll"
  File "C:\MinGW\msys\1.0\lib\gtk-2.0\2.10.0\engines\libwimp.dll"
	
  SetOutPath $INSTDIR\lib\gtk-2.0\modules
	
  File "C:\MinGW\msys\1.0\lib\gtk-2.0\modules\libgail.dll" 
	
  SetOutPath $INSTDIR\etc\gtk-2.0
  
  File "C:\MinGW\msys\1.0\etc\gtk-2.0\gtkrc"
  
  SetOutPath $INSTDIR\share\themes\MS-Windows\gtk-2.0
  
  File "C:\MinGW\msys\1.0\share\themes\MS-Windows\gtk-2.0\gtkrc" 
  
  !ifndef PORTABLE
  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\TransmissionRemoteGTK" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TransmissionRemoteGTK" "DisplayName" "Transmission Remote GTK"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TransmissionRemoteGTK" "Publisher" "Alan Fitton"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TransmissionRemoteGTK" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TransmissionRemoteGTK" "DisplayIcon" "$INSTDIR\bin\transmission-remote-gtk.exe,0"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TransmissionRemoteGTK" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TransmissionRemoteGTK" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
!endif
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    SetShellVarContext current
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Transmission Remote GTK.lnk" "$INSTDIR\bin\transmission-remote-gtk.exe" "" "$INSTDIR\bin\transmission-remote-gtk.exe" 0 
!ifndef PORTABLE
    SetShellVarContext all
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Transmission Remote GTK.lnk" "$INSTDIR\bin\transmission-remote-gtk.exe" "" "$INSTDIR\bin\transmission-remote-gtk.exe" 0 
!endif
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

Section $(NAME_GeoIP) SecGeoIP
  SetOutPath $INSTDIR
  
  File "..\GeoIP.dat"
  File "..\GeoIPv6.dat"
SectionEnd

Section $(NAME_SecGlibGtkEtc) SecGlibGtkEtc
  SetOutPath $INSTDIR\bin

  File "C:\MinGW\msys\1.0\bin\libgtk-win32-2.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libffi-5.dll"
  File "C:\MinGW\msys\1.0\bin\libcairo-gobject-2.dll"
  File "C:\MinGW\msys\1.0\bin\libcrypto-8.dll"
  File "C:\MinGW\msys\1.0\bin\libssl-8.dll"
  File "C:\MinGW\msys\1.0\bin\freetype6.dll"
  File "C:\MinGW\msys\1.0\bin\libatk-1.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libcairo-2.dll"
  File "C:\MinGW\msys\1.0\bin\libcurl-4.dll"
  File "C:\MinGW\msys\1.0\bin\libexpat-1.dll"
  File "C:\MinGW\msys\1.0\bin\libfontconfig-1.dll"
  File "C:\MinGW\msys\1.0\bin\libgdk-win32-2.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libgdk_pixbuf-2.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libgio-2.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libglib-2.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libgmodule-2.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libgobject-2.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libgthread-2.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libjson-glib-1.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libpango-1.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libpangocairo-1.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libpangoft2-1.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libpangowin32-1.0-0.dll"
  File "C:\MinGW\msys\1.0\bin\libpng14-14.dll"
  File "C:\MinGW\msys\1.0\bin\zlib1.dll"  
  File "C:\MinGW\msys\1.0\bin\libintl-8.dll"
  File "C:\MinGW\msys\1.0\bin\intl.dll"
  File "C:\MinGW\msys\1.0\bin\libiconv-2.dll"
  File "C:\MinGW\msys\1.0\bin\gspawn-win32-helper-console.exe"
  File "C:\MinGW\msys\1.0\bin\gspawn-win32-helper.exe"
  File "C:\MinGW\msys\1.0\bin\libproxy.dll"
  File "C:\MinGW\msys\1.0\bin\libmodman.dll"
  File "C:\MinGW\msys\1.0\bin\libstdc++-6.dll"
  File "C:\MinGW\msys\1.0\bin\libgcc_s_sjlj-1.dll"
  File "C:\MinGW\msys\1.0\bin\libgcc_s_dw2-1.dll"
    
  #SetOutPath $INSTDIR\lib\gtk-2.0\2.10.0\engines
	
  #File "C:\MinGW\msys\1.0\lib\gtk-2.0\2.10.0\engines\libpixmap.dll"
  #File "C:\MinGW\msys\1.0\lib\gtk-2.0\2.10.0\engines\libwimp.dll"
  
  #SetOutPath $INSTDIR\lib\gtk-2.0\modules
  
  #File "C:\MinGW\msys\1.0\lib\gtk-2.0\modules\libgail.dll"
  
  #SetOutPath $INSTDIR\share\locale\lt\LC_MESSAGES
  
  #File "C:\MinGW\share\locale\lt\LC_MESSAGES\gtk20.mo"
  #File "C:\MinGW\share\locale\lt\LC_MESSAGES\gtk20-properties.mo"
  #File "C:\MinGW\share\locale\lt\LC_MESSAGES\glib20.mo"
  #File "C:\MinGW\share\locale\lt\LC_MESSAGES\gdk-pixbuf.mo"
  #File "C:\MinGW\share\locale\lt\LC_MESSAGES\atk10.mo"
  
  #SetOutPath $INSTDIR\share\locale\uk\LC_MESSAGES
  
  #File "C:\MinGW\share\locale\uk\LC_MESSAGES\libiconv.mo"
  #File "C:\MinGW\share\locale\uk\LC_MESSAGES\gtk20.mo"
  #File "C:\MinGW\share\locale\uk\LC_MESSAGES\gtk20-properties.mo"
  #File "C:\MinGW\share\locale\uk\LC_MESSAGES\glib20.mo"
  #File "C:\MinGW\share\locale\uk\LC_MESSAGES\gettext-tools.mo"
  #File "C:\MinGW\share\locale\uk\LC_MESSAGES\gettext-runtime.mo"
  #File "C:\MinGW\share\locale\uk\LC_MESSAGES\gdk-pixbuf.mo"
  #File "C:\MinGW\share\locale\uk\LC_MESSAGES\atk10.mo"
  
  SetOutPath $INSTDIR\share\locale\fr\LC_MESSAGES
  
  File "C:\MinGW\msys\1.0\share\locale\fr\LC_MESSAGES\gtk20.mo"
  File "C:\MinGW\msys\1.0\share\locale\fr\LC_MESSAGES\gtk20-properties.mo"
  #File "C:\MinGW\share\locale\fr\LC_MESSAGES\glib20.mo"
  File "C:\MinGW\share\locale\fr\LC_MESSAGES\gettext-runtime.mo"
  #File "C:\MinGW\share\locale\fr\LC_MESSAGES\gdk-pixbuf.mo"
  #File "C:\MinGW\share\locale\fr\LC_MESSAGES\atk10.mo"
  
  SetOutPath $INSTDIR\share\locale\ru\LC_MESSAGES
  
  File "C:\MinGW\share\locale\ru\LC_MESSAGES\libiconv.mo"
  #File "C:\MinGW\share\locale\ru\LC_MESSAGES\gtk20.mo"
  #File "C:\MinGW\share\locale\ru\LC_MESSAGES\gtk20-properties.mo"
  #File "C:\MinGW\share\locale\ru\LC_MESSAGES\glib20.mo"
  File "C:\MinGW\share\locale\ru\LC_MESSAGES\gettext-tools.mo"
  File "C:\MinGW\share\locale\ru\LC_MESSAGES\gettext-runtime.mo"
  #File "C:\MinGW\share\locale\ru\LC_MESSAGES\gdk-pixbuf.mo"
  #File "C:\MinGW\share\locale\ru\LC_MESSAGES\atk10.mo"

  SetOutPath $INSTDIR\share\locale\pl\LC_MESSAGES

  File "C:\MinGW\share\locale\pl\LC_MESSAGES\libiconv.mo"
  #File "C:\MinGW\share\locale\pl\LC_MESSAGES\gtk20.mo"
  #File "C:\MinGW\share\locale\pl\LC_MESSAGES\gtk20-properties.mo"
  #File "C:\MinGW\share\locale\pl\LC_MESSAGES\glib20.mo"
  File "C:\MinGW\share\locale\pl\LC_MESSAGES\gettext-tools.mo"
  File "C:\MinGW\share\locale\pl\LC_MESSAGES\gettext-runtime.mo"
  #File "C:\MinGW\share\locale\pl\LC_MESSAGES\gdk-pixbuf.mo"
  #File "C:\MinGW\share\locale\pl\LC_MESSAGES\atk10.mo"

  SetOutPath $INSTDIR\share\locale\ko\LC_MESSAGES

  #File "C:\MinGW\share\locale\ko\LC_MESSAGES\gtk20.mo"
  #File "C:\MinGW\share\locale\ko\LC_MESSAGES\gtk20-properties.mo"
  #File "C:\MinGW\share\locale\ko\LC_MESSAGES\glib20.mo"
  File "C:\MinGW\share\locale\ko\LC_MESSAGES\gettext-tools.mo"
  File "C:\MinGW\share\locale\ko\LC_MESSAGES\gettext-runtime.mo"
  #File "C:\MinGW\share\locale\ko\LC_MESSAGES\gdk-pixbuf.mo"
  #File "C:\MinGW\share\locale\ko\LC_MESSAGES\atk10.mo"

  SetOutPath $INSTDIR\share\locale\es\LC_MESSAGES

  File "C:\MinGW\share\locale\es\LC_MESSAGES\libiconv.mo"
  #File "C:\MinGW\share\locale\es\LC_MESSAGES\gtk20.mo"
  #File "C:\MinGW\share\locale\es\LC_MESSAGES\gtk20-properties.mo"
  #File "C:\MinGW\share\locale\es\LC_MESSAGES\glib20.mo"
  File "C:\MinGW\share\locale\es\LC_MESSAGES\gettext-tools.mo"
  File "C:\MinGW\share\locale\es\LC_MESSAGES\gettext-runtime.mo"
  #File "C:\MinGW\share\locale\es\LC_MESSAGES\gdk-pixbuf.mo"
  #File "C:\MinGW\share\locale\es\LC_MESSAGES\atk10.mo"

  SetOutPath $INSTDIR\share\locale\de\LC_MESSAGES

  File "C:\MinGW\share\locale\de\LC_MESSAGES\libiconv.mo"
  #File "C:\MinGW\share\locale\de\LC_MESSAGES\gtk20.mo"
  #File "C:\MinGW\share\locale\de\LC_MESSAGES\gtk20-properties.mo"
  #File "C:\MinGW\share\locale\de\LC_MESSAGES\glib20.mo"
  File "C:\MinGW\share\locale\de\LC_MESSAGES\gettext-tools.mo"
  File "C:\MinGW\share\locale\de\LC_MESSAGES\gettext-runtime.mo"
  #File "C:\MinGW\share\locale\de\LC_MESSAGES\gdk-pixbuf.mo"
  #File "C:\MinGW\share\locale\de\LC_MESSAGES\atk10.mo"

  SetOutPath $INSTDIR\share\icons\hicolor

  File "C:\MinGW\msys\1.0\share\icons\hicolor\index.theme"
  File "C:\MinGW\msys\1.0\share\icons\hicolor\icon-theme.cache"
  
SectionEnd

; Optional section (can be disabled by the user)

Section /o $(NAME_SecDesktopIcon) SecDesktopIcon
  SetShellVarContext current
  SetOutPath "$INSTDIR\bin"
  CreateShortCut "$DESKTOP\Transmission Remote GTK.lnk" "$INSTDIR\bin\transmission-remote-gtk.exe" "" "$INSTDIR\bin\transmission-remote-gtk.exe" 0
SectionEnd

;!ifndef PORTABLE
;SubSection $(NAME_SecFiletypeAssociations) SecFiletypeAssociations

;  Section $(NAME_SecRegiterTorrent) SecRegiterTorrent
;    ${registerExtension} "$INSTDIR\Transmission Remote.exe" ".torrent" "Transmission Remote Torrent"
;  SectionEnd

;  Section $(NAME_SecRegiterMagnet) SecRegiterMagnet
;    ${registerProtocol} "$INSTDIR\Transmission Remote.exe" "magnet" "Magnet URI"
;  SectionEnd

;SubSectionEnd
;!endif

; Translation

;SectionGroup $(NAME_SecLanguages) SecLanguages

;  Section /o "Brazilian Portuguese" SecLanguagesBrazilianPortuguese
;    CreateDirectory "$INSTDIR\pt-BR"
;    SetOutPath "$INSTDIR\pt-BR"
;    File "TransmissionClientNew\bin\Release\pt-BR\Transmission Remote.resources.dll"
;  SectionEnd

 ; Section /o "Chinese" SecLanguagesChinese
 ;   CreateDirectory "$INSTDIR\zh-CN"
 ;   SetOutPath "$INSTDIR\zh-CN"
 ;   File "TransmissionClientNew\bin\Release\zh-CN\Transmission Remote.resources.dll"
 ; SectionEnd
  
;SectionGroupEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

;!ifndef PORTABLE
  ; Unregister File Association
;  ${unregisterExtension} ".torrent" "Transmission Remote Torrent"
;  ${unregisterProtocol} "magnet" "Magnet URI"
  
;!endif

  ; Remove files and uninstaller
  Delete "$INSTDIR\COPYING.txt"
  Delete "$INSTDIR\README.txt"
  Delete "$INSTDIR\AUTHORS.txt"  
  Delete "$INSTDIR\ChangeLog.txt"
  Delete "$INSTDIR\GeoIP.dat"
  Delete "$INSTDIR\GeoIPv6.dat"
  Delete "$INSTDIR\uninstall.exe"
  Delete "$INSTDIR\bin\transmission-remote-gtk.exe"
  Delete "$INSTDIR\bin\freetype6.dll"
  Delete "$INSTDIR\bin\intl.dll"
  Delete "$INSTDIR\bin\libatk-1.0-0.dll"
  Delete "$INSTDIR\bin\libcairo-2.dll"
  Delete "$INSTDIR\bin\libcurl-4.dll"
  Delete "$INSTDIR\bin\libexpat-1.dll"
  Delete "$INSTDIR\bin\libfontconfig-1.dll"
  Delete "$INSTDIR\bin\libgdk-win32-2.0-0.dll"
  Delete "$INSTDIR\bin\libgdk_pixbuf-2.0-0.dll"
  Delete "$INSTDIR\bin\libgio-2.0-0.dll"
  Delete "$INSTDIR\bin\libglib-2.0-0.dll"
  Delete "$INSTDIR\bin\libgmodule-2.0-0.dll"
  Delete "$INSTDIR\bin\libgobject-2.0-0.dll"
  Delete "$INSTDIR\bin\libgthread-2.0-0.dll"
  Delete "$INSTDIR\bin\libjson-glib-1.0-0.dll"
  Delete "$INSTDIR\bin\libpango-1.0-0.dll"
  Delete "$INSTDIR\bin\libpangocairo-1.0-0.dll"
  Delete "$INSTDIR\bin\libpangoft2-1.0-0.dll"
  Delete "$INSTDIR\bin\libpangowin32-1.0-0.dll"
  Delete "$INSTDIR\bin\libpng14-14.dll"
  Delete "$INSTDIR\bin\intl.dll"
  Delete "$INSTDIR\bin\libintl-8.dll"
  Delete "$INSTDIR\bin\libiconv-2.dll"
  Delete "$INSTDIR\bin\zlib1.dll"
  Delete "$INSTDIR\bin\libgtk-win32-2.0-0.dll"
  Delete "$INSTDIR\bin\libffi-5.dll"
  Delete "$INSTDIR\bin\libcairo-gobject-2.dll"
  Delete "$INSTDIR\bin\libcrypto-8.dll"
  Delete "$INSTDIR\bin\libssl-8.dll"
 
  Delete "$INSTDIR\bin\libproxy.dll"
  Delete "$INSTDIR\bin\libmodman.dll"
  Delete "$INSTDIR\bin\libstdc++-6.dll"
  Delete "$INSTDIR\bin\libgcc_s_sjlj-1.dll"
  Delete "$INSTDIR\bin\libgcc_s_dw2-1.dll"
  
  Delete "$INSTDIR\bin\gspawn-win32-helper-console.exe"
  Delete "$INSTDIR\bin\gspawn-win32-helper.exe"
  #Delete "$INSTDIR\lib\gtk-2.0\2.10.0\engines\libpixmap.dll"
  #Delete "$INSTDIR\lib\gtk-2.0\2.10.0\engines\libwimp.dll"
  #Delete "$INSTDIR\lib\gtk-2.0\modules\libgail.dll"
  #Delete "$INSTDIR\etc\gtk-2.0\gtkrc"

  Delete "$INSTDIR\share\icons\hicolor\16x16\apps\transmission-remote-gtk.png"
  Delete "$INSTDIR\share\icons\hicolor\22x22\apps\transmission-remote-gtk.png"
  Delete "$INSTDIR\share\icons\hicolor\24x24\apps\transmission-remote-gtk.png"
  Delete "$INSTDIR\share\icons\hicolor\32x32\apps\transmission-remote-gtk.png"
  Delete "$INSTDIR\share\icons\hicolor\48x48\apps\transmission-remote-gtk.png"
  Delete "$INSTDIR\share\icons\hicolor\icon-theme.cache"
  Delete "$INSTDIR\share\icons\hicolor\index.theme"
  Delete "$INSTDIR\share\icons\hicolor\scalable\apps\transmission-remote-gtk.svg"
  #Delete "$INSTDIR\share\locale\de\LC_MESSAGES\atk10.mo"
  #Delete "$INSTDIR\share\locale\de\LC_MESSAGES\gdk-pixbuf.mo"
  Delete "$INSTDIR\share\locale\de\LC_MESSAGES\gettext-runtime.mo"
  Delete "$INSTDIR\share\locale\de\LC_MESSAGES\gettext-tools.mo"
  #Delete "$INSTDIR\share\locale\de\LC_MESSAGES\glib20.mo"
  #Delete "$INSTDIR\share\locale\de\LC_MESSAGES\gtk20-properties.mo"
  #Delete "$INSTDIR\share\locale\de\LC_MESSAGES\gtk20.mo"
  Delete "$INSTDIR\share\locale\de\LC_MESSAGES\libiconv.mo"
  Delete "$INSTDIR\share\locale\de\LC_MESSAGES\transmission-remote-gtk.mo"
  #Delete "$INSTDIR\share\locale\lt\LC_MESSAGES\atk10.mo"
  #Delete "$INSTDIR\share\locale\lt\LC_MESSAGES\gdk-pixbuf.mo"
  #Delete "$INSTDIR\share\locale\lt\LC_MESSAGES\glib20.mo"
  #Delete "$INSTDIR\share\locale\lt\LC_MESSAGES\gtk20-properties.mo"
  #Delete "$INSTDIR\share\locale\lt\LC_MESSAGES\gtk20.mo"
  Delete "$INSTDIR\share\locale\lt\LC_MESSAGES\transmission-remote-gtk.mo"
  #Delete "$INSTDIR\share\locale\es\LC_MESSAGES\atk10.mo"
  #Delete "$INSTDIR\share\locale\es\LC_MESSAGES\gdk-pixbuf.mo"
  Delete "$INSTDIR\share\locale\es\LC_MESSAGES\gettext-runtime.mo"
  Delete "$INSTDIR\share\locale\es\LC_MESSAGES\gettext-tools.mo"
  #Delete "$INSTDIR\share\locale\es\LC_MESSAGES\glib20.mo"
  #Delete "$INSTDIR\share\locale\es\LC_MESSAGES\gtk20-properties.mo"
  #Delete "$INSTDIR\share\locale\es\LC_MESSAGES\gtk20.mo"
  Delete "$INSTDIR\share\locale\es\LC_MESSAGES\libiconv.mo"
  Delete "$INSTDIR\share\locale\es\LC_MESSAGES\transmission-remote-gtk.mo"
  #Delete "$INSTDIR\share\locale\ko\LC_MESSAGES\atk10.mo"
  #Delete "$INSTDIR\share\locale\ko\LC_MESSAGES\gdk-pixbuf.mo"
  Delete "$INSTDIR\share\locale\ko\LC_MESSAGES\gettext-runtime.mo"
  Delete "$INSTDIR\share\locale\ko\LC_MESSAGES\gettext-tools.mo"
  #Delete "$INSTDIR\share\locale\ko\LC_MESSAGES\glib20.mo"
  #Delete "$INSTDIR\share\locale\ko\LC_MESSAGES\gtk20-properties.mo"
  #Delete "$INSTDIR\share\locale\ko\LC_MESSAGES\gtk20.mo"
  Delete "$INSTDIR\share\locale\ko\LC_MESSAGES\transmission-remote-gtk.mo"
  #Delete "$INSTDIR\share\locale\pl\LC_MESSAGES\atk10.mo"
  #Delete "$INSTDIR\share\locale\pl\LC_MESSAGES\gdk-pixbuf.mo"
  Delete "$INSTDIR\share\locale\pl\LC_MESSAGES\gettext-runtime.mo"
  Delete "$INSTDIR\share\locale\pl\LC_MESSAGES\gettext-tools.mo"
  #Delete "$INSTDIR\share\locale\pl\LC_MESSAGES\glib20.mo"
  #Delete "$INSTDIR\share\locale\pl\LC_MESSAGES\gtk20-properties.mo"
  #Delete "$INSTDIR\share\locale\pl\LC_MESSAGES\gtk20.mo"
  Delete "$INSTDIR\share\locale\pl\LC_MESSAGES\libiconv.mo"
  Delete "$INSTDIR\share\locale\pl\LC_MESSAGES\transmission-remote-gtk.mo"
  #Delete "$INSTDIR\share\locale\ru\LC_MESSAGES\atk10.mo"
  #Delete "$INSTDIR\share\locale\ru\LC_MESSAGES\gdk-pixbuf.mo"
  Delete "$INSTDIR\share\locale\ru\LC_MESSAGES\gettext-runtime.mo"
  Delete "$INSTDIR\share\locale\ru\LC_MESSAGES\gettext-tools.mo"
  #Delete "$INSTDIR\share\locale\ru\LC_MESSAGES\glib20.mo"
  #Delete "$INSTDIR\share\locale\ru\LC_MESSAGES\gtk20-properties.mo"
  #Delete "$INSTDIR\share\locale\ru\LC_MESSAGES\gtk20.mo"
  Delete "$INSTDIR\share\locale\ru\LC_MESSAGES\libiconv.mo"
  Delete "$INSTDIR\share\locale\ru\LC_MESSAGES\transmission-remote-gtk.mo"
  #Delete "$INSTDIR\share\locale\fr\LC_MESSAGES\atk10.mo"
  #Delete "$INSTDIR\share\locale\fr\LC_MESSAGES\gdk-pixbuf.mo"
  Delete "$INSTDIR\share\locale\fr\LC_MESSAGES\gettext-runtime.mo"
  #Delete "$INSTDIR\share\locale\fr\LC_MESSAGES\glib20.mo"
  Delete "$INSTDIR\share\locale\fr\LC_MESSAGES\gtk20-properties.mo"
  Delete "$INSTDIR\share\locale\fr\LC_MESSAGES\gtk20.mo"
  Delete "$INSTDIR\share\locale\fr\LC_MESSAGES\transmission-remote-gtk.mo"
  #Delete "$INSTDIR\share\locale\uk\LC_MESSAGES\atk10.mo"
  #Delete "$INSTDIR\share\locale\uk\LC_MESSAGES\gdk-pixbuf.mo"
  #Delete "$INSTDIR\share\locale\uk\LC_MESSAGES\gettext-runtime.mo"
  #Delete "$INSTDIR\share\locale\uk\LC_MESSAGES\gettext-tools.mo"
  #Delete "$INSTDIR\share\locale\uk\LC_MESSAGES\glib20.mo"
  #Delete "$INSTDIR\share\locale\uk\LC_MESSAGES\gtk20-properties.mo"
  #Delete "$INSTDIR\share\locale\uk\LC_MESSAGES\gtk20.mo"
  #Delete "$INSTDIR\share\locale\uk\LC_MESSAGES\libiconv.mo"
  #Delete "$INSTDIR\share\locale\uk\LC_MESSAGES\transmission-remote-gtk.mo"
  #Delete "$INSTDIR\share\themes\MS-Windows\gtk-2.0\gtkrc"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Script.pod"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Script.pm"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Maketext\TPJ13.pod"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Maketext\GutsLoader.pm"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Maketext\Guts.pm"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Maketext.pod"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Maketext.pm"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Language.pod"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Language.pm"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Currency.pod"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Currency.pm"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Country.pod"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Country.pm"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Constants.pod"
  Delete "$INSTDIR\share\perl5\5.8\Locale\Constants.pm"
  
  ; Remove shortcuts, if any
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  SetShellVarContext current
  Delete "$SMPROGRAMS\$StartMenuFolder\*.*"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
!ifndef PORTABLE
  SetShellVarContext all
  Delete "$SMPROGRAMS\$StartMenuFolder\*.*"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
!endif

  ; Remove directories used
  #RMDir "$INSTDIR\share\themes\MS-Windows\gtk-2.0"
  #RMDir "$INSTDIR\share\themes\MS-Windows"
  RMDir "$INSTDIR\share\themes"
  #RMDir "$INSTDIR\share\locale\uk\LC_MESSAGES"
  #RMDir "$INSTDIR\share\locale\uk"
  RMDir "$INSTDIR\share\locale\fr\LC_MESSAGES"
  RMDir "$INSTDIR\share\locale\fr"
  RMDir "$INSTDIR\share\locale\ru\LC_MESSAGES"
  RMDir "$INSTDIR\share\locale\ru"
  RMDir "$INSTDIR\share\locale\pl\LC_MESSAGES"
  RMDir "$INSTDIR\share\locale\pl"
  RMDir "$INSTDIR\share\locale\ko\LC_MESSAGES"
  RMDir "$INSTDIR\share\locale\ko"
  RMDir "$INSTDIR\share\locale\es\LC_MESSAGES"
  RMDir "$INSTDIR\share\locale\es"
  RMDir "$INSTDIR\share\locale\de\LC_MESSAGES"
  RMDir "$INSTDIR\share\locale\de"
  RMDir "$INSTDIR\share\locale\lt\LC_MESSAGES"
  RMDir "$INSTDIR\share\locale\lt"
  RMDir "$INSTDIR\share\locale"
  RMDir "$INSTDIR\share\icons\hicolor\scalable\apps"
  RMDir "$INSTDIR\share\icons\hicolor\scalable"
  RMDir "$INSTDIR\share\icons\hicolor\48x48\apps"
  RMDir "$INSTDIR\share\icons\hicolor\48x48"
  RMDir "$INSTDIR\share\icons\hicolor\32x32\apps"
  RMDir "$INSTDIR\share\icons\hicolor\32x32"
  RMDir "$INSTDIR\share\icons\hicolor\24x24\apps"
  RMDir "$INSTDIR\share\icons\hicolor\24x24"
  RMDir "$INSTDIR\share\icons\hicolor\22x22\apps"
  RMDir "$INSTDIR\share\icons\hicolor\22x22"
  RMDir "$INSTDIR\share\icons\hicolor\16x16\apps"
  RMDir "$INSTDIR\share\icons\hicolor\16x16"
  RMDir "$INSTDIR\share\icons\hicolor"
  RMDir "$INSTDIR\share\icons"
  RMDir "$INSTDIR\share\perl5\5.8\Locale\Maketext"
  RMDir "$INSTDIR\share\perl5\5.8\Locale"
  RMDir "$INSTDIR\share\perl5\5.8"
  RMDir "$INSTDIR\share\perl5"
  RMDir "$INSTDIR\share"
  #RMDir "$INSTDIR\etc\gtk-2.0"
  RMDir "$INSTDIR\etc"
  #RMDir "$INSTDIR\lib\gtk-2.0\modules"
  #RMDir "$INSTDIR\lib\gtk-2.0\2.10.0\engines"
  #RMDir "$INSTDIR\lib\gtk-2.0\2.10.0"
  #RMDir "$INSTDIR\lib\gtk-2.0"
  RMDir "$INSTDIR\lib"
  RMDir "$INSTDIR\bin"
  RMDir "$INSTDIR"
  
  DeleteRegKey /ifempty HKCU "SOFTWARE\TransmissionRemoteGTK"
!ifndef PORTABLE
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TransmissionRemoteGTK"
!endif

SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTransmissionRemoteGTK} $(DESC_SecTransmissionRemoteGTK)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecGeoIP} $(DESC_GeoIP)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecGlibGtkEtc} $(DESC_SecGlibGtkEtc)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktopIcon} $(DESC_SecDesktopIcon)
;  !insertmacro MUI_DESCRIPTION_TEXT ${SecFiletypeAssociations} $(DESC_SecFiletypeAssociations)
;  !insertmacro MUI_DESCRIPTION_TEXT ${SecGeoIPDatabase} $(DESC_SecGeoIPDatabase)
;  !insertmacro MUI_DESCRIPTION_TEXT ${SecLanguages} $(DESC_SecLanguages)
;  !insertmacro MUI_DESCRIPTION_TEXT ${SecRegiterTorrent} $(DESC_SecRegiterTorrent)
;  !insertmacro MUI_DESCRIPTION_TEXT ${SecRegiterMagnet} $(DESC_SecRegiterMagnet)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function .onInit
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "Transmission Remote GTK") ?e'
  Pop $R0
  StrCmp $R0 0 +3
    MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
    Abort

  !insertmacro MUI_LANGDLL_DISPLAY
!ifdef PORTABLE
  StrCpy $INSTDIR "\${ProgramFilesDir}"
!else
  ${If} ${RunningX64}
      StrCpy $INSTDIR "$PROGRAMFILES64\${ProgramFilesDir}"
  ${Else}
      StrCpy $INSTDIR "$PROGRAMFILES\${ProgramFilesDir}"
  ${Endif}
!endif
FunctionEnd

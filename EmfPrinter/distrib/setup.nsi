;
; The NSIS (http://nsis.sf.net) installer of EmfPrinter
;


;---------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General


; NOTE: the version should be the same as the one in globals.h
!define PRODUCT_NAME            "EmfPrinter"
!define PRODUCT_VERSION         "0.3.1"
!define PRODUCT_PUBLISHER       "Francesco Montorsi"

; should we include in the package the .pdb files?
!define DEBUG                   "TRUE"

  ;Name and file
  Name "EmfPrinter ${PRODUCT_VERSION} Installer"
  OutFile "${PRODUCT_NAME}-${PRODUCT_VERSION}.exe"
  ;Icon "icon.gif"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
  
  ;Vista redirects $SMPROGRAMS to all users without this
  RequestExecutionLevel admin

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "i386\gnugpl.txt"
  !insertmacro MUI_PAGE_DIRECTORY
  Page custom printerName
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Reserve Files
  
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------
;Variables

  Var RETVAL
  Var PRINTER_NAME
  
;--------------------------------
;Installer Sections

Section "" ;No components page, name is not important

  SetOutPath "$INSTDIR"
  File i386\ReadMe.html
  File i386\gnugpl.txt
  File i386\*.dll
  File i386\emfprinter_inst.exe
  File i386\emfutils.exe
  File i386\imagemagick\*.dll
  File i386\imagemagick\*.exe
  
  StrCmp "${DEBUG}" "FALSE" "+2"    ; jump 2 lines below if $DEBUG == FALSE
    File i386\*.pdb
  
  SetOutPath "$INSTDIR\docs"
  File i386\docs\style.css
  File i386\docs\*.png


  ;Store installation folder
  WriteRegStr HKLM "Software\EmfPrinter" "Path" "$INSTDIR"
  
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  ;Read the printer name chosen by the user
  !insertmacro MUI_INSTALLOPTIONS_READ $PRINTER_NAME "printerNameSetupPage.ini" "Field 2" "State"
  
  ; TODO check if name is non-null
  
  ; run our mini printer installer 
  ExecWait '"$INSTDIR\emfprinter_inst.exe" "install" "$INSTDIR" "$PRINTER_NAME"' $RETVAL
   
  ; now check the return value of the mini installer:
  ; if zero, then install was ok; otherwise it failed
  StrCmp "$RETVAL" "0" "+2"    ; jump 2 lines below if $RETVAL == 0
    Abort "Couldn't install the printer... emfprinter_inst.exe failed. Aborting."

  ;Create shortcuts
  CreateDirectory "$SMPROGRAMS\EmfPrinter"
  CreateShortCut "$SMPROGRAMS\EmfPrinter\ReadMe.lnk" "$INSTDIR\ReadMe.html"

  CreateShortCut "$SMPROGRAMS\EmfPrinter\CheckInstall.lnk" "$INSTDIR\emfprinter_inst.exe" \
                 "check" "$INSTDIR\emfprinter_inst.exe" 1 SW_SHOWNORMAL "" \
                 "Checks if the virtual printer is correctly installed"

  CreateShortCut "$SMPROGRAMS\EmfPrinter\EmfUtilsEditor.lnk" "$INSTDIR\emfutils.exe" "edit"

  CreateShortCut "$SMPROGRAMS\EmfPrinter\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Installer Functions

Function .onInit
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "printerNameSetupPage.ini"
FunctionEnd

Function printerName
  !insertmacro MUI_HEADER_TEXT "Choose the printer name" "Choose the virtual printer name"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "printerNameSetupPage.ini"
FunctionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ; run our mini printer installer 
  ExecWait '"$INSTDIR\emfprinter_inst.exe" "uninstall"'

  Delete "$INSTDIR\Readme.html"
  Delete "$INSTDIR\docs\style.css"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\*.exe"
 
  RMDir "$INSTDIR"


  ; clean start menu

  Delete "$SMPROGRAMS\EmfPrinter\Uninstall.lnk"
  Delete "$SMPROGRAMS\EmfPrinter\ReadMe.lnk"
  Delete "$SMPROGRAMS\EmfPrinter\EmfUtilsEditor.lnk"
  Delete "$SMPROGRAMS\EmfPrinter\CheckInstall.lnk"
  
  ;Delete empty start menu folder
  RMDir "$SMPROGRAMS\EmfPrinter"
  
  DeleteRegKey /ifempty HKLM "Software\EmfPrinter"

SectionEnd


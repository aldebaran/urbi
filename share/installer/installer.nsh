; NSIS installer
;
; What it does:
;   - Install the whole installdir
;   - Make menu entries for the runtime, the doc, and the uninstaller
;   - Install Microsoft Visual Studio templates, using the VS90COMNTOOLS
;     environment variable to locate Visual Studio.
;
; To build the installer:
;   cd to the installdir and run makensis.exe /NOCD share/installer/installer.nsh
; be careful to remove gostai-engine-runtime.exe before running the command again


!include share\installer\WriteEnvStr.nsh
!include Sections.nsh
!define SECTION_ON ${SF_SELECTED}

Name "Gostai Engine Runtime"
outFile gostai-engine-runtime.exe

; Text on top of the selection of the list.
ComponentText "The following components would be installed on your system."

InstallDir "$PROGRAMFILES\Gostai Engine Runtime"

;Get installation folder from registry if available
InstallDirRegKey HKCU "Software\Gostai Engine" ""

Section "Urbi" opt_urbi
  ; Cannot disable Urbi from the list.
  SectionIn RO

;  Write URBI_ROOT=$INSTDIR to persistent env
  Push URBI_INSTALL_DIR
  Push $INSTDIR
  Call WriteEnvStr

  setOutPath $INSTDIR

; Install everything, excluding installer sources and some top-level files.
  File /r /x share/installer /x share /x MANIFEST *
  File /r share
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\Gostai"
  CreateShortcut  "$SMPROGRAMS\Gostai\runtime.lnk" "$INSTDIR\bin\urbi.exe" "--interactive --port 54000"
  CreateShortcut  "$SMPROGRAMS\Gostai\uninstall.lnk" "$INSTDIR\Uninstall.exe"

; If the documentation is not installed, use online documentation instead.
  IfFileExists $INSTDIR\share\doc\urbi-sdk\urbi-sdk.pdf 0 no_doc
    CreateShortcut  "$SMPROGRAMS\Gostai\doc.lnk" "$INSTDIR\share\doc\urbi-sdk\urbi-sdk.pdf"
  Goto shortcuts_end
no_doc:
    CreateShortcut  "$SMPROGRAMS\Gostai\doc.lnk" "http://gostai.com/downloads/urbi-sdk/doc/"
shortcuts_end:

; Run vcredist to install Visual Studio libraries.
; msiexec arguments are /qn (mute) or /qb! (no user interactions)
; Use this simpler line if you use the vcredist shipped with visual studio
;  ExecWait '"$INSTDIR\vcredist-x86.exe" /q:a /c:"msiexec /i vcredist.msi /qn"' $R0
; Use this line if you use the standalone one from www.microsoft.com
;  ExecWait '"$INSTDIR\vcredist-x86.exe" /q:a /c:"VCREDI~1.EXE /q:a /c:""msiexec /i vcredist.msi /qn"" "' $R0

; Do not mute the installation of vcredist because it may raised an error
; message which cannot be catch with the exit code.  However the /q option
; is used to avoid user interactions.
  ExecWait '"$INSTDIR\vcredist-x86.exe" /q'

; Error 1723 => Need to update the Windows Installer to a newer version.
SectionEnd

; Install Gostai Console
Section "Gostai Console" opt_console
  IfFileExists $INSTDIR\gostai-console-installer.exe 0 +2
  ExecWait '"$INSTDIR\\gostai-console-installer.exe"'
SectionEnd

; Install Gostai Editor
Section "Gostai Editor" opt_editor
  IfFileExists $INSTDIR\gostai-editor-installer.exe 0 +2
  ExecWait '"$INSTDIR\\gostai-editor-installer.exe"'
SectionEnd

; Install our visual studio wizard
;
; This is done by creating two files in the vcprojects directory.
!ifdef vcxx-2008
Section "Microsoft Visual Studio 2008 Wizard" opt_vcxx2008
  DetailPrint "Install: Microsoft Visual Studio 2008 Wizard"
  ClearErrors
  ReadEnvStr $1 VS90COMNTOOLS
  IfErrors enverror
  FileOpen $0 $1\..\..\VC\vcprojects\uobject.vsdir w
  IfErrors fileerror
  FileWrite $0 "uobject.vsz| |uobject|1|URBI UObject template.| |6777| |uobject"
  FileClose $0

  FileOpen $0 $1\..\..\VC\vcprojects\uobject.vsz w
  IfErrors fileerror
  FileWrite $0 "VSWIZARD 7.0"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 "Wizard=VsWizard.VsWizardEngine.9.0"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 'Param="WIZARD_NAME = uobject"'
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 'Param="ABSOLUTE_PATH = $INSTDIR\share\templates\Visual Studio"'
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 'Param="FALLBACK_LCID = 1033"'
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 'Param="URBI_INSTALL_DIR = $INSTDIR"'
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileClose $0
  GoTo done
enverror:
    MessageBox MB_OK "Microsoft Visual Studio 2008 not detected, not installing the UObject wizard."
    GoTo done
fileerror:
    MessageBox MB_OK "Problem installing the Microsof Visual Studio Wizard: Error opening file for writing."
    GoTo done
done:
SectionEnd
!endif

Section "-End"
    MessageBox MB_YESNO|MB_ICONQUESTION "Do you want to start using URBI now?" IDNO done
    ReadEnvStr $2 COMSPEC
    Exec '"$2" /K "cd $INSTDIR\bin"'

; If the documentation is not installed, use online documentation instead.
  IfFileExists $INSTDIR\share\doc\urbi-sdk\urbi-sdk.htmldir\getting-started.html 0 no_doc
    ExecShell "open" '"$INSTDIR\share\doc\urbi-sdk\urbi-sdk.htmldir\getting-started.html"'
  Goto done
no_doc:
    ExecShell "open" "http://gostai.com/downloads/urbi-sdk/doc/getting-started.html"
done:
SectionEnd

Section uninstall
  Push URBI_INSTALL_DIR
  Call un.DeleteEnvStr
; RMDIR /r is dangerous, the user might have chosen a standard install dir such as the desktop
  RMDir /r $INSTDIR\bin
  RMDir /r $INSTDIR\share
  RMDir /r $INSTDIR\gostai
  RMDir /r $INSTDIR\include
  RMDir /r $INSTDIR\libexec
  Delete  $INSTDIR\Uninstall.exe
  Delete  $INSTDIR\urbi.bat
  Delete  $INSTDIR\urbi-d.bat
  Delete  $INSTDIR\LICENSE.txt
  Delete  $INSTDIR\README.txt
  Delete  $INSTDIR\vcredist-x86.exe
  RMDir   $INSTDIR
  Delete  "$SMPROGRAMS\Gostai\runtime.lnk"
  Delete  "$SMPROGRAMS\Gostai\doc.lnk"
  Delete  "$SMPROGRAMS\Gostai\uninstall.lnk"
  RMDir "$SMPROGRAMS\Gostai"
  ReadEnvStr $1 VS90COMNTOOLS
  IfErrors done
  Delete  $1\..\..\VC\vcprojects\uobject.vsdir
  Delete  $1\..\..\VC\vcprojects\uobject.vsz
done:
  RMDir $INSTDIR
SectionEnd

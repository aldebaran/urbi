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

Name "Gostai Engine Runtime"
outFile gostai-engine-runtime.exe

InstallDir "$PROGRAMFILES\Gostai Engine Runtime"

;Get installation folder from registry if available
InstallDirRegKey HKCU "Software\Gostai Engine" ""

Section
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
  CreateShortcut  "$SMPROGRAMS\Gostai\doc.lnk" "$INSTDIR\share\doc\urbi-sdk\urbi-sdk.pdf"
  CreateShortcut  "$SMPROGRAMS\Gostai\uninstall.lnk" "$INSTDIR\Uninstall.exe"


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


; Install urbi console
  IfFileExists $INSTDIR\urbi-console-installer.exe 0 +2
  ExecWait '"$INSTDIR\\urbi-console-installer.exe"'

; Install our visual studio wizard
;
; This is done by creating two files in the vcprojects directory.
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
  GoTo gettingStarted
enverror:
    MessageBox MB_OK "Microsoft Visual Studio 2008 not detected, not installing the UObject wizard."
    GoTo gettingStarted
fileerror:
    MessageBox MB_OK "Problem installing the Microsof Visual Studio Wizard: Error opening file for writing."
    GoTo gettingStarted

gettingStarted:
    MessageBox MB_YESNO|MB_ICONQUESTION "Do you want to start using URBI now?" IDNO done
    ReadEnvStr $2 COMSPEC
    Exec '"$2" /K "cd $INSTDIR\bin"'
    ExecShell "open" "$INSTDIR\share\doc\urbi-sdk\urbi-sdk.htmldir\getting-started.html"

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

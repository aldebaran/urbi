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

; Install everything, excluding installer sources and top-level files.
  File /r /x share/installer /x *.txt /x share /x MANIFEST *
  File /r share
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\Gostai"
  CreateShortcut  "$SMPROGRAMS\Gostai\runtime.lnk" "$INSTDIR\bin\urbi.exe" "--port 54000"
  CreateShortcut  "$SMPROGRAMS\Gostai\doc.lnk" "$INSTDIR\share\doc\urbi-sdk\urbi-sdk.pdf"
  CreateShortcut  "$SMPROGRAMS\Gostai\uninstall.lnk" "$INSTDIR\Uninstall.exe"

; Install our visual studio wizard
;
; This is done by creating two files in the vcprojects directory.
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

Section uninstall
  Push URBI_INSTALL_DIR
  Call un.DeleteEnvStr
; RMDIR /r is dangerous, the user might have chosen a standard install dir such as the desktop
  RMDir /r $INSTDIR\bin
  RMDir /r $INSTDIR\share
  RMDir /r $INSTDIR\gostai
  RMDir /r $INSTDIR\include
  Delete  $INSTDIR\Uninstall.exe
  Delete  $INSTDIR\urbi.bat
  ReadEnvStr $1 VS90COMNTOOLS
  IfErrors done
  Delete  $1\..\..\VC\vcprojects\uobject.vsdir
  Delete  $1\..\..\VC\vcprojects\uobject.vsz
done:
  RMDir $INSTDIR
SectionEnd

; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Comical"
#define MyAppVerName "Comical 0.8"
#define MyAppPublisher "James Athey"
#define MyAppURL "http://comical.sourceforge.net/"
#define MyAppExeName "Comical.exe"
#define MyAppUrlName "Comical.url"
#define MyAppVersion "0.8"

[Setup]
AppName={#MyAppName}
AppVerName={#MyAppVerName}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=COPYING.txt
OutputBaseFilename={#MyAppName}-Win32-{#MyAppVersion}
Compression=lzma
SolidCompression=true
OutputDir=.\

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked

[Files]
Source: Comical.exe; DestDir: {app}; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
Source: unicows.dll; DestDir: {app}; Attribs: readonly; Flags: ignoreversion
Source: ChangeLog.txt; DestDir: {app}
Source: Readme.txt; DestDir: {app}; Flags: isreadme
Source: TODO.txt; DestDir: {app}
Source: COPYING.txt; DestDir: {app}
Source: Comical Icons\cbr.ico; DestDir: {app}
Source: Comical Icons\cbz.ico; DestDir: {app}
Source: mingwm10.dll; DestDir: {app}

[INI]
Filename: {app}\{#MyAppUrlName}; Section: InternetShortcut; Key: URL; String: {#MyAppURL}

[Icons]
Name: {group}\{#MyAppName}; Filename: {app}\{#MyAppExeName}
Name: {group}\{cm:ProgramOnTheWeb,{#MyAppName}}; Filename: {app}\{#MyAppUrlName}
Name: {group}\{cm:UninstallProgram,{#MyAppName}}; Filename: {uninstallexe}
Name: {userdesktop}\{#MyAppName}; Filename: {app}\{#MyAppExeName}; Tasks: desktopicon

[Run]
Filename: {app}\{#MyAppExeName}; Description: {cm:LaunchProgram,{#MyAppName}}; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: {app}\{#MyAppUrlName}
[Registry]
Root: HKCR; SubKey: .cbr; ValueType: string; ValueData: ComicBookRAR; Flags: uninsdeletekey
Root: HKCR; SubKey: ComicBookRAR; ValueType: string; ValueData: Comic Book RAR; Flags: uninsdeletekey
Root: HKCR; SubKey: ComicBookRAR\Shell\Open\Command; ValueType: string; ValueData: """{app}\Comical.exe"" ""%1"""; Flags: uninsdeletevalue
Root: HKCR; Subkey: ComicBookRAR\DefaultIcon; ValueType: string; ValueData: {app}\cbr.ico,-1; Flags: uninsdeletevalue
Root: HKCR; SubKey: .cbz; ValueType: string; ValueData: ComicBookZIP; Flags: uninsdeletekey
Root: HKCR; SubKey: ComicBookZIP; ValueType: string; ValueData: Comic Book ZIP; Flags: uninsdeletekey
Root: HKCR; SubKey: ComicBookZIP\Shell\Open\Command; ValueType: string; ValueData: """{app}\Comical.exe"" ""%1"""; Flags: uninsdeletevalue
Root: HKCR; Subkey: ComicBookZIP\DefaultIcon; ValueType: string; ValueData: {app}\cbz.ico,-1; Flags: uninsdeletevalue

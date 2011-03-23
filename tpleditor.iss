; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Degica Template Editor"
#define MyAppVersion "1.0beta3"
#define MyAppPublisher "Degica"
#define MyAppURL "http://www.degica.com/"
#define MyAppExeName "tpleditor.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{F4778164-07D1-4A7D-A395-214F03466969}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=C:\Documents and Settings\Tester\Bureau\tpleditor-build-desktop
OutputBaseFilename=tpleditor
SetupIconFile=C:\Documents and Settings\Tester\Bureau\tpleditor\res\degica.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Documents and Settings\Tester\Bureau\tpleditor-build-desktop\release\tpleditor.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Tester\Bureau\tpleditor-build-desktop\release\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Tester\Bureau\tpleditor-build-desktop\release\libgcc_s_dw2-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Tester\Bureau\tpleditor-build-desktop\release\mingwm10.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Tester\Bureau\tpleditor-build-desktop\release\QtCore4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Tester\Bureau\tpleditor-build-desktop\release\QtGui4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Tester\Bureau\tpleditor-build-desktop\release\QtNetwork4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Tester\Bureau\tpleditor-build-desktop\release\QtXml4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Documents and Settings\Tester\Bureau\tpleditor-build-desktop\release\ssleay32.dll"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, "&", "&&")}}"; Flags: nowait postinstall skipifsilent


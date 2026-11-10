#define MyAppName "Aura Island"
#ifndef AppVersion
  #ifdef MyAppVersion
    #define AppVersion MyAppVersion
  #else
    #define AppVersion "1.0.0"
  #endif
#endif
#define MyAppPublisher "Aura"
#define MyAppExeName "AuraIsland.exe"
#define MyAppMutex "AuraIsland_SingleInstance_Mutex"

[Setup]
AppId={{C7B38F64-280E-4C2E-8E5A-7FE156A2B3C9}
AppName={#MyAppName}
AppVersion={#AppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputDir=..\dist-engine
OutputBaseFilename=AuraEngine
SetupIconFile=..\resources\icons\app-icon.ico
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog
AppMutex={#MyAppMutex}
CloseApplications=yes
RestartApplications=no
DisableProgramGroupPage=yes
UninstallDisplayIcon={app}\{#MyAppExeName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "autostart"; Description: "Start Aura Island when Windows starts"; GroupDescription: "Windows Integration:"

[Files]
Source: "..\dist\AuraIsland\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "AuraIsland"; ValueData: """{app}\{#MyAppExeName}"""; Tasks: autostart; Flags: uninsdeletevalue

[Code]
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  DataDir: String;
begin
  if CurUninstallStep = usUninstall then
  begin
    // Check %APPDATA%\AuraIsland
    DataDir := ExpandConstant('{userappdata}\AuraIsland');
    if DirExists(DataDir) then
    begin
      // Ask user whether to also delete notes, settings, and database (Default = NO / Keep)
      if MsgBox('Do you also want to delete your personal notes, settings, and clipboard database?' + #13#10 + #13#10 + '(Selecting No will preserve your notes and data for future reinstalls)', mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES then
      begin
        DelTree(DataDir, True, True, True);
      end;
    end;
  end;
end;

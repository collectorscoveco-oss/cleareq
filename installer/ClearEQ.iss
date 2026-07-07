#define MyAppName "ClearEQ"
#define MyAppVersion "0.1.1"
#define MyAppPublisher "Collectors Cove"
#ifndef SourceDir
#define SourceDir "..\build\ClearEQ_artefacts\Release"
#endif
#ifndef OutputDir
#define OutputDir "Output"
#endif

[Setup]
AppId={{D9C985A6-8C48-4D7E-B8B9-DB88966C1C8E}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir={#OutputDir}
OutputBaseFilename=ClearEQ-v{#MyAppVersion}-Windows-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
UninstallDisplayIcon={app}\ClearEQ.exe
SetupIconFile=

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut for the standalone ClearEQ app"; GroupDescription: "Additional shortcuts:"; Flags: unchecked

[Files]
; VST3 plugin bundle. This is the important DAW install path for Reaper, FL Studio, Ableton, Studio One, etc.
Source: "{#SourceDir}\VST3\ClearEQ.vst3\*"; DestDir: "{commoncf64}\VST3\ClearEQ.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

; Optional standalone test app.
Source: "{#SourceDir}\Standalone\ClearEQ.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\ClearEQ Standalone"; Filename: "{app}\ClearEQ.exe"
Name: "{autodesktop}\ClearEQ Standalone"; Filename: "{app}\ClearEQ.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\ClearEQ.exe"; Description: "Launch ClearEQ standalone"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\ClearEQ.vst3"

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
end;

[Setup]
AppName=Shine
AppVersion=0.1.0-BETA
DefaultDirName={localappdata}\Programs\Shine
DefaultGroupName=Shine
PrivilegesRequired=lowest
SetupIconFile=..\assets\logo.ico
UninstallDisplayIcon={app}\shinec.exe
OutputDir=..\dist
OutputBaseFilename=Shine-v0.1.0-BETA-Installer
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
DisableProgramGroupPage=yes
ChangesEnvironment=yes

[Files]
Source: "..\dist\shine-v0.1.0-BETA\shinec.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\dist\shine-v0.1.0-BETA\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\dist\shine-v0.1.0-BETA\examples\*"; DestDir: "{app}\examples"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "..\build\release-toolchain\*"; DestDir: "{app}\toolchain"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "..\assets\logo.ico"; DestDir: "{app}"; Flags: ignoreversion

[Registry]
Root: HKCU; Subkey: "Environment"; \
    ValueType: expandsz; ValueName: "Path"; \
    ValueData: "{olddata};{app}"; \
    Check: NeedsAddPath()

[Code]
function NeedsAddPath(): Boolean;
var
  Path: string;
begin
  if not RegQueryStringValue(HKEY_CURRENT_USER,
    'Environment',
    'Path', Path) then
  begin
    Result := True;
    exit;
  end;

  Result := Pos(';' + ExpandConstant('{app}') + ';', ';' + Path + ';') = 0;
end;

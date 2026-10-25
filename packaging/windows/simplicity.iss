; Stable per-user identity; upgrades replace this preview, never create versioned copies.
#ifndef StageDir
  #error StageDir is required
#endif
#ifndef OutputDir
  #error OutputDir is required
#endif
#ifndef AppVersion
  #error AppVersion is required
#endif
[Setup]
AppId={{A92718CD-EC5F-4B46-8DDC-39B8D853A2CC}
AppName=Simplicity (development preview)
AppVersion={#AppVersion}
AppPublisher=Simplicity
AppPublisherURL=https://github.com/carlaparlor/simplicity
DefaultDirName={localappdata}\Programs\Simplicity
DefaultGroupName=Simplicity
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
DisableProgramGroupPage=yes
DisableDirPage=yes
OutputDir={#OutputDir}
OutputBaseFilename=Simplicity-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\simplicity.exe
SetupLogging=yes
[Files]
Source: "{#StageDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
[Icons]
Name: "{autoprograms}\Simplicity"; Filename: "{app}\Launch Simplicity.cmd"; WorkingDir: "{app}"; IconFilename: "{app}\simplicity.exe"
[Run]
Filename: "{app}\Launch Simplicity.cmd"; Description: "Run Simplicity engine preview"; Flags: postinstall shellexec skipifsilent

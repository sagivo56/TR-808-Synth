; Inno Setup script for the TR-808 Synth plugin.
; Builds a one-click Windows installer that drops the VST3 into the system
; VST3 folder and the Standalone into Program Files (+ Start Menu shortcuts).
;
; Build it with:  build-installer.ps1   (after running package.ps1 to stage dist\)
; Requires Inno Setup 6  (https://jrsoftware.org/isdl.php).

#define MyName    "TR-808 Synth"
#define MyVersion "1.0.0"
#define MyPublisher "Sagiv"
; Folder staged by package.ps1 (relative to this .iss file):
#define Stage "..\dist\TR-808 Synth"

[Setup]
AppName={#MyName}
AppVersion={#MyVersion}
AppPublisher={#MyPublisher}
DefaultDirName={commonpf}\{#MyName}
DefaultGroupName={#MyName}
DisableProgramGroupPage=yes
OutputDir=..\dist
OutputBaseFilename=TR-808-Synth-Setup
Compression=lzma2
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
SetupIconFile=icon.ico
UninstallDisplayIcon={app}\TR-808 Synth.exe

[Components]
Name: "vst3"; Description: "VST3 plugin (for your DAW)"; Types: full custom
Name: "app";  Description: "Standalone app";             Types: full custom

[Files]
; VST3 bundle -> Common Files\VST3
Source: "{#Stage}\TR-808 Synth.vst3\*"; DestDir: "{commoncf64}\VST3\TR-808 Synth.vst3"; \
    Flags: ignoreversion recursesubdirs createallsubdirs; Components: vst3
; Standalone -> Program Files
Source: "{#Stage}\TR-808 Synth.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: app
Source: "{#Stage}\INSTALL.txt";      DestDir: "{app}"; Flags: ignoreversion;  Components: app

[Icons]
Name: "{group}\TR-808 Synth";           Filename: "{app}\TR-808 Synth.exe"; Components: app
Name: "{group}\Uninstall TR-808 Synth"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\TR-808 Synth.exe"; Description: "Launch TR-808 Synth"; Flags: nowait postinstall skipifsilent; Components: app

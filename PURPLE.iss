; Inno Setup script - Windows installer for PURPLE by Hdax
#define AppVer "0.1.0"
[Setup]
AppName=PURPLE by Hdax
AppVersion={#AppVer}
AppPublisher=Hdax
DefaultDirName={commonpf64}\Hdax\PURPLE
OutputDir=Output
OutputBaseFilename=PURPLE-by-Hdax-{#AppVer}-Windows-Setup
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
Compression=lzma2
DisableProgramGroupPage=yes

[Files]
Source: "build\PURPLE_artefacts\Release\VST3\PURPLE by Hdax.vst3\*"; DestDir: "{commoncf64}\VST3\PURPLE by Hdax.vst3"; Flags: recursesubdirs ignoreversion; Components: vst3
; AAX is only packaged if an AAX build exists (needs the Avid SDK + PACE/iLok signing for Pro Tools)
#if DirExists("build\PURPLE_artefacts\Release\AAX\PURPLE by Hdax.aaxplugin")
Source: "build\PURPLE_artefacts\Release\AAX\PURPLE by Hdax.aaxplugin\*"; DestDir: "{commoncf64}\Avid\Audio\Plug-Ins\PURPLE by Hdax.aaxplugin"; Flags: recursesubdirs ignoreversion; Components: aax
#endif

[Components]
Name: "vst3"; Description: "VST3 (FL Studio, Ableton Live, Cubase, Studio One, Reaper, Bitwig)"; Types: full custom; Flags: fixed
#if DirExists("build\PURPLE_artefacts\Release\AAX\PURPLE by Hdax.aaxplugin")
Name: "aax"; Description: "AAX (Pro Tools)"; Types: full
#endif

[Types]
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom"; Flags: iscustom

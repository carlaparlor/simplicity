# Windows installation and developer setup

## Platform and honest friction budget

Windows 10/11 **x64** only. Native ARM64 is not supported; do not rely on emulation
as a tested target. Windows 10 may be outside Microsoft's standard support;
Windows 11 is recommended. No macOS/Linux installer or product support.
The current application is a headless engine bootstrap, not yet a DAW editor.

The goal is download → install → launch, with no toolchain, JUCE account, Python,
PATH editing or separate Visual C++ runtime installation for end users. The
executable uses MSVC's static runtime; CI checks its imported DLL names.
Installation is per-user under `%LOCALAPPDATA%\Programs\Simplicity`, not Program
Files. The compiled installer requests `asInvoker`, adds a Start menu shortcut
and registers with Windows Settings > Apps. It never changes PATH or file
associations. No services, drivers, telemetry, background updater or auto-start.

**Remaining friction:** artifacts require a GitHub login and ZIP extraction;
unsigned programs can trigger SmartScreen/antivirus policy. No signing identity
or commercial distribution approval exists yet. Do not disable protection or
pretend a hash is a publisher signature. A public download/release and signed
installer are follow-up work after the owner confirms licensing/signing.

## Install / portable / remove

1. Choose a **successful main Quality run** linked from README. Its commit identifies
   exactly what you are testing. Download `Simplicity-Windows-x64-development`.
2. Extract the artifact ZIP. Optionally verify both payloads against SHA256SUMS.txt:
   `Get-FileHash .\Simplicity-Setup.exe -Algorithm SHA256`.
3. Run `Simplicity-Setup.exe`, then use **Simplicity** in the Start menu. No admin
   elevation or machine-wide install is needed. Silent install is supported:
   `Simplicity-Setup.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART`.
4. Alternatively extract `Simplicity-Portable.zip` into a writable folder and run
   `Launch Simplicity.cmd`. The launcher keeps the current console result visible.

Expected now: `Simplicity: C++/JUCE engine bootstrap ready (no DAW UI yet).`
No GUI/audio playback exists. If it fails, retain the exit/error text and artifact
commit. Start menu/portable launch pauses so failures don't disappear immediately.

Reinstall a newer preview to the same folder; its stable application ID replaces
the previous installation. There is no updater or rollback automation. Do not store
user projects/settings in the install directory; no persistence format exists yet.
Uninstall through **Windows Settings → Apps → Simplicity (development preview)**.
For portable builds just delete the extracted folder. Removing an installed copy
does not touch the portable copy. Never put personal files in either program folder.

## One-command development

Run `setup.cmd` from an extracted/cloned source tree, even outside a Developer
Command Prompt. It uses built-in Windows PowerShell 5.1 and process-local policy
bypass; it does not change global/user execution policy or require profile changes.
It checks Git, CMake, Python >=3.11 and Visual Studio 2022 C++/SDK components;
winget installs missing prerequisites, then CMake fetches the immutable JUCE pin,
builds the Windows x64 Release preset and runs all CTest suites.

Missing developer prerequisites can require many GB, internet, installer licence
acceptance, UAC/elevation and a reboot. This cost is for **developers only**, never
end users. The script prints that distinction before installing tools. Source
setup trusts Microsoft's winget catalogue and installed tools; it does not execute
an arbitrary downloaded shell script. Enterprise policy can block installation.
No GitHub credentials or JUCE account are requested. Git is needed for JUCE fetch.

```bat
setup.cmd
setup.cmd -NoInstall
setup.cmd -Configuration Debug
```

Manual equivalents once tools are present:

```bat
cmake --preset windows
cmake --build --preset windows
ctest --preset windows
build\windows\app\Release\simplicity.exe
```

The script may pass a discovered `Python3_EXECUTABLE` to avoid Store aliases.
Existing CMake must meet the 3.25 minimum; an older one fails with CMake's version
message (upgrade with `winget upgrade --id Kitware.CMake --exact`). Re-running setup
is safe: prerequisites are detected, the CMake cache is reused, and failures are
never swallowed. It does not delete source, projects, or a user's build directory.

## Packaging tools / reproducible packaging

Packaging is separate from normal developer setup. Install Inno Setup 6.4.3 from
its official upstream release (CI verifies SHA-256
`f3c42116542c4cc57263c5ba6c4feabfc49fe771f2f98a79d2f7628b8762723b`).
Then in Windows PowerShell:

```powershell
.\setup.cmd -NoInstall
.\tools\windows\package.ps1
.\tests\windows\installer_test.ps1
```

`package.ps1 -Iscc 'C:\path\to\ISCC.exe'` selects a nonstandard compiler location.
Packaging retests Release, stages only runtime files/notices, generates the
per-user installer, portable ZIP, and SHA256SUMS.txt under `build/packages`.
It fails rather than emitting a Debug or untested package. Staging/build outputs
are ignored by Git. No command publishes a release or modifies release tags.

CI pins windows-2022 to match the supported VS2022 preset (not the drifting
windows-latest image), installs packaging prerequisites on its disposable runner, exercises the real setup
command in Debug/Release, parses scripts with PowerShell 5.1, verifies command
failures propagate, compiles the installer, checks the compiled manifest/static
runtime, launches portable/installed builds, reinstalls and uninstalls. Tests use
paths with spaces and `&`. Lifecycle tests refuse to run over an existing Simplicity install/shortcut;
use a clean disposable account. Hosted runners are elevated; testing `asInvoker` and
per-user registration is not a substitute for a human standard-user desktop test.
SmartScreen behavior, visual wizard flow and restrictive enterprise accounts are
human acceptance items; they are not claimed proven by CI.

## Troubleshooting

- **winget missing:** install/update Microsoft's *App Installer* from the Store,
  reopen a terminal and rerun setup. Alternatively install the prerequisites above
  manually, then use `-NoInstall`.
- **Reboot requested / tool not found:** complete installation/reboot, reopen a
  terminal and rerun. Visual Studio needs *Desktop development with C++*, MSVC x64
  and Windows SDK. VS2022 Community also works; VS2022 Build Tools is sufficient.
- **PowerShell blocked by policy:** enterprise policy takes precedence. Ask the
  administrator; do not lower machine-wide security settings. Manual CMake presets
  work with an installed toolchain.
- **Proxy/offline:** first source build requires access to GitHub for JUCE and to
  tool providers if prerequisites are missing. Prepared runtime packages are offline.
- **Wrong generator/architecture in an old build cache:** preserve anything valuable,
  then remove only `build/windows` and rerun setup. Do not delete repository source.
- **Installer blocked/unknown publisher:** verify origin and checksums; consult your
  security policy. Signed releases are pending. Never disable antivirus/SmartScreen.

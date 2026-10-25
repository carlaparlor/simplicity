SIMPLICITY — Windows x64 development preview

Install: run Simplicity-Setup.exe. This installs only for the current user under
Local AppData, adds a Start menu shortcut, and does not require administrator rights.
Uninstall via Windows Settings > Apps > Simplicity (development preview).
Installing a newer preview replaces the previous one; user projects/settings must
never be stored in the install folder. No project/settings format exists yet.

Portable: extract the complete ZIP into a writable folder, then double-click
"Launch Simplicity.cmd". No installer, registry entries or developer tools needed.
Delete that extracted folder to remove the portable copy.

Expected today: a console readiness message, not a DAW window or audible playback.
The launcher keeps that message visible. This is not yet a usable workstation.

No compiler, JUCE setup, Python or Visual C++ runtime installation is required.
Windows 10/11 x64 only. Native ARM64, Linux and macOS are not supported.

These are unsigned development artifacts, not an approved commercial release.
Windows SmartScreen/security tools may warn or block them. Do not disable security
protections; only test artifacts from the trusted repository and verify checksums.
A checksum detects corruption; it does not establish publisher identity.

Commercial licensing/distribution terms and code signing remain pending owner
confirmation. Do not publish or redistribute these development artifacts as a
commercial product. Upstream JUCE licensing details are included in notices/.

Source, tests and issue reports: https://github.com/carlaparlor/simplicity

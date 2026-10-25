# Render evidence

`workspace.png` is the actual 1440x900 JUCE component snapshot from
`gui_controls_and_render`, Windows Release, source commit
`07481947676810a9f5eddf2471b5147dfc86b366`, run
https://github.com/carlaparlor/simplicity/actions/runs/40234936540 , check
`121294404184`. The 50,609-byte PNG was retrieved, its chunk lengths/CRCs verified
and visually reviewed. SHA256:
`f42333d878e41611dc18f65f7a2391e17473eaa8eac6266964c8943edc759228`.

It is not a mockup, AI image or a browser approximation. Native title bar controls
and application icon are outside this workspace component snapshot. The packaged
executable icon is independently extracted/verified by Windows lifecycle tests.
Workspace, browser and styling source are unchanged since this capture. The
final update explicitly registers the same icon with the native window peer;
that is outside the workspace component image.
Temporary retrieval diagnostics are removed; normal artifacts remain canonical.

Reproduce with `setup.cmd -NoInstall`: the Windows CTest GUI test writes
`build/windows/screenshots/workspace.png`. CI retains screenshots with the
Windows test artifacts. Pixel-perfect text may vary with fonts/Windows version;
regression assertions cover stable palette/geometry and interaction state, not
cross-machine font rasterization. Refresh this evidence when the design changes.

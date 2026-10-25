# Render evidence

`workspace.png` is the actual 1440x900 JUCE component snapshot from
`gui_controls_and_render`, Windows Release, source commit `b2197f3`, run
https://github.com/carlaparlor/simplicity/actions/runs/40230005414 .
The PNG was verified byte-for-byte by PNG chunk CRCs after retrieval and visually
reviewed. It is not a mockup, AI image or a browser approximation. Native title
bar controls are outside this component snapshot. No GUI code changed between
this capture and the documentation commit that added it.

Reproduce with `setup.cmd -NoInstall`: the Windows CTest GUI test writes
`build/windows/screenshots/workspace.png`. CI retains screenshots with the
Windows test artifacts. Pixel-perfect text may vary with fonts/Windows version;
regression assertions cover stable palette/geometry and interaction state, not
cross-machine font rasterization. Refresh this evidence when the design changes.

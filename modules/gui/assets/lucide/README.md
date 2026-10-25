# Lucide sources

Official Lucide 1.61.0, commit c29c276f16590cf017126671535ad8c792be64d8:
https://github.com/lucide-icons/lucide/tree/c29c276f16590cf017126671535ad8c792be64d8/icons
Only the used SVGs are vendored verbatim; SHA256s are in ../manifest.json.
LICENSE includes ISC and the applicable Feather/MIT notices. It is shipped in
installed and portable notices/LUCIDE-LICENSE.txt. No Node/web runtime is needed.

make_ui_assets.py embeds unchanged SVG text. JUCE parses it natively; currentColor
is replaced at load time and tinted per control. Standard 24x24 geometry, 2-unit
stroke and round joins/caps remain upstream-authentic, not hand-drawn imitations.

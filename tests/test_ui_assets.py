"""Exact Lucide sources, embedded RetroByte, font-derived icon and provenance."""
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import tempfile
import unittest
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]

def load(name):
    spec=importlib.util.spec_from_file_location(name,ROOT / f'tools/assets/{name}.py')
    module=importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
    return module

assets=load('make_ui_assets')
outline=load('retrobyte_outline')

class UIAssetsTests(unittest.TestCase):
    def test_pinned_sources_and_font_identity(self):
        root=ROOT / 'modules/gui/assets'
        manifest=json.loads((root/'manifest.json').read_text())
        self.assertEqual(manifest['lucide_commit'],'c29c276f16590cf017126671535ad8c792be64d8')
        self.assertEqual(hashlib.sha256((root/'retrobyte/RetroByte.ttf').read_bytes()).hexdigest(),manifest['font_sha256'])
        for name in assets.ICONS:
            data=(root/f'lucide/{name}.svg').read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(),manifest['icons'][name])
            svg=ET.fromstring(data)
            self.assertEqual(svg.attrib['viewBox'],'0 0 24 24')
            self.assertEqual(svg.attrib['stroke-width'],'2')
            self.assertEqual(svg.attrib['stroke'],'currentColor')
            self.assertEqual(svg.attrib['stroke-linecap'],'round')
            self.assertEqual(svg.attrib['stroke-linejoin'],'round')
            self.assertEqual(svg.attrib['fill'],'none')
        self.assertIn('ISC License',(root/'lucide/LICENSE').read_text())
        self.assertIn('Cole Bemis',(root/'lucide/LICENSE').read_text())

    def test_embedded_assets_match_sources_without_installing_a_font(self):
        with tempfile.TemporaryDirectory() as temp:
            assets.generate(Path(temp))
            for relative in ['modules/gui/include/simplicity/ui_asset_data.h','modules/gui/assets/manifest.json']:
                self.assertEqual((ROOT/relative).read_bytes(),(Path(temp)/relative).read_bytes())
        header=(ROOT/'modules/gui/include/simplicity/ui_asset_data.h').read_text()
        font=bytes(int(x,16) for x in re.findall(r'0x([0-9a-f]{2})',header.split('inline constexpr const char*')[0]))
        self.assertEqual(font,(ROOT/'modules/gui/assets/retrobyte/RetroByte.ttf').read_bytes())
        for name in assets.ICONS:
            self.assertIn((ROOT/f'modules/gui/assets/lucide/{name}.svg').read_text().strip(),header)

    def test_exact_pixel_font_s_polygons(self):
        polygons=outline.contours((ROOT/'modules/gui/assets/retrobyte/RetroByte.ttf').read_bytes())
        self.assertEqual([len(p) for p in polygons],[42,4])
        self.assertEqual(polygons[0][:5],[(256,0),(192,0),(128,0),(64,0),(64,64)])
        self.assertEqual(polygons[1],[(64,64),(0,64),(0,128),(64,128)])
        self.assertEqual({x for p in polygons for x,y in p},{0,64,128,192,256,320,384})

    def test_unsupported_font_and_missing_glyph_fail(self):
        with self.assertRaises(ValueError):outline.contours(b'not a font')
        with self.assertRaises(ValueError):outline.contours((ROOT/'modules/gui/assets/retrobyte/RetroByte.ttf').read_bytes(),'\u2603')

    def test_licensing_notices_are_packaged(self):
        package=(ROOT/'cmake/Packaging.cmake').read_text()
        self.assertIn('LUCIDE-LICENSE.txt',package)
        self.assertIn('RETROBYTE-NOTICE.txt',package)
        notice=(ROOT/'modules/gui/assets/retrobyte/NOTICE.txt').read_text()
        self.assertIn('personal-use',notice)
        self.assertIn('unresolved',notice)

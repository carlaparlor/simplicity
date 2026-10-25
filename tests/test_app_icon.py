"""Byte-level PNG/ICO and deterministic regeneration regressions; stdlib only."""
import importlib.util
from pathlib import Path
import re
import struct
import tempfile
import unittest
import zlib

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('make_icon', ROOT / 'tools/assets/make_app_icon.py')
icon = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(icon)


class AppIconTests(unittest.TestCase):
    def decode(self, data, size):
        self.assertEqual(data[:8], b'\x89PNG\r\n\x1a\n')
        offset = 8
        compressed = bytearray()
        kinds = []
        while offset < len(data):
            length = struct.unpack_from('>I', data, offset)[0]
            kind = data[offset+4:offset+8]
            body = data[offset+8:offset+8+length]
            checksum = struct.unpack_from('>I', data, offset+8+length)[0]
            self.assertEqual(checksum, zlib.crc32(kind+body))
            kinds.append(kind)
            if kind == b'IHDR':
                self.assertEqual(struct.unpack('>IIBBBBB', body), (size, size, 8, 6, 0, 0, 0))
            if kind == b'IDAT':
                compressed.extend(body)
            offset += length+12
        self.assertEqual(offset, len(data))
        self.assertEqual(kinds, [b'IHDR', b'IDAT', b'IEND'])
        rows = zlib.decompress(compressed)
        self.assertEqual(len(rows), size*(1+size*4))
        colours = set()
        transparent = 0
        for y in range(size):
            row = rows[y*(size*4+1):(y+1)*(size*4+1)]
            self.assertEqual(row[0], 0)
            for x in range(size):
                r, g, b, a = row[1+x*4:5+x*4]
                self.assertEqual(r, g)
                self.assertEqual(g, b)
                colours.add(r)
                transparent += a < 255
        self.assertTrue({91, 238}.issubset(colours))
        self.assertGreater(transparent, 0)
        self.assertLess(rows[4], 255)  # Small frames have antialiased, not necessarily clear, corners.
        if size >= 48:
            self.assertEqual(rows[4], 0)
        return data

    def test_every_ico_frame_is_valid_grayscale_png(self):
        data = (ROOT / 'app/assets/simplicity.ico').read_bytes()
        self.assertEqual(struct.unpack_from('<HHH', data), (0, 1, len(icon.SIZES)))
        end = 6+16*len(icon.SIZES)
        for index, size in enumerate(icon.SIZES):
            width, height, palette, reserved, planes, bits, length, offset = struct.unpack_from('<BBBBHHII', data, 6+16*index)
            self.assertEqual((width or 256, height or 256, palette, reserved, planes, bits), (size, size, 0, 0, 1, 32))
            self.assertEqual(offset, end)
            self.decode(data[offset:offset+length], size)
            end += length
        self.assertEqual(end, len(data))

    def test_runtime_png_matches_largest_resource_frame(self):
        header = (ROOT / 'modules/gui/include/simplicity/application_icon.h').read_text()
        embedded = bytes(int(b, 16) for b in re.findall(r'0x([0-9a-f]{2})', header))
        self.assertEqual(embedded, self.decode(icon.png(256), 256))

    def test_generator_reproduces_checked_in_assets(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            icon.generate(root)
            for path in ('app/assets/simplicity.ico', 'modules/gui/include/simplicity/application_icon.h'):
                self.assertEqual((ROOT / path).read_bytes(), (root / path).read_bytes())

    def test_resource_and_runtime_wiring_are_present(self):
        self.assertIn('1 ICON "@SIMPLICITY_ICON@"', (ROOT / 'app/resources.rc.in').read_text())
        self.assertIn('configure_file(resources.rc.in', (ROOT / 'app/CMakeLists.txt').read_text())
        self.assertIn('setIcon(juce::ImageFileFormat::loadFrom(simplicity::applicationIconPng', (ROOT / 'app/main.cpp').read_text())

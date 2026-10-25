import importlib.util
from pathlib import Path
import unittest

SPEC = importlib.util.spec_from_file_location('annotate_ctest', Path(__file__).resolve().parents[1] / 'tools/annotate_ctest.py')
DIAG = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(DIAG)


class CTestDiagnosticsTests(unittest.TestCase):
    def test_only_failed_sections_are_annotated(self):
        lines = DIAG.annotations('1/2 Testing: good\nTest Passed.\n2/2 Testing: bad\nAssertion failed\nTest Failed.\n')
        self.assertEqual(len(lines), 1)
        self.assertIn('Assertion failed', lines[0])
        self.assertNotIn('Testing: good', lines[0])

    def test_workflow_command_characters_are_escaped(self):
        line = DIAG.annotations('100%\r\n::error::payload')[0]
        self.assertIn('100%25%0D%0A::error::payload', line)
        self.assertNotIn('\n', line)
        self.assertNotIn('\r', line)

    def test_partial_crash_log_and_output_are_bounded(self):
        lines = DIAG.annotations('X' * 10000 + '\nAddressSanitizer:DEADLYSIGNAL')
        self.assertEqual(len(lines), 1)
        self.assertIn('AddressSanitizer:DEADLYSIGNAL', lines[0])
        self.assertLess(len(lines[0]), 3600)

    def test_many_failed_tests_do_not_overflow_annotations(self):
        lines = DIAG.annotations(''.join(f'{n}/20 Testing: test{n}\nTest Failed.\n' for n in range(20)))
        self.assertEqual(len(lines), 8)

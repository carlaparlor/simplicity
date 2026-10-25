#!/usr/bin/env python3
"""Expose failed CTest output through check annotations; never decides gate status."""
from pathlib import Path
import re
import sys


def annotations(text: str) -> list[str]:
    sections = re.split(r'(?m)(?=^\d+/\d+ Testing:)', text)
    failed = [section for section in sections if 'Test Failed.' in section]
    return ['::notice title=CTest failure details::' + section[-3500:].replace('%', '%25').replace('\r', '%0D').replace('\n', '%0A')
            for section in (failed or [text])[:8]]


if __name__ == '__main__':
    path = Path(sys.argv[1]) / 'Testing/Temporary/LastTest.log'
    text = path.read_text(errors='replace') if path.exists() else 'CTest log unavailable; inspect the preceding failed step.'
    print('\n'.join(annotations(text)))

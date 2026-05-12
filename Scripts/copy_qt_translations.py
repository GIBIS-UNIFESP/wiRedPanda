#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
"""Copy Qt translation .qm files into a destination directory, renaming
locale codes where the app's code differs from Qt's source basename.

Usage:
    copy_qt_translations.py <qt_translations_dir> <destination_dir>
"""

import shutil
import sys
from pathlib import Path

# Maps app language code -> Qt source basename (without .qm extension).
# Most languages: qtbase_<lang>.qm exists directly.
# Special cases where Qt uses a different locale code than the app.
LANGS = {
    **{lang: f"qtbase_{lang}" for lang in [
        "ar", "bg", "cs", "da", "de", "es", "fa", "fi", "fr",
        "he", "hr", "hu", "it", "ja", "ko", "lv", "nl", "pl",
        "pt_BR", "ru", "sk", "sv", "tr", "uk",
    ]},
    "zh_Hans": "qtbase_zh_CN",
    "zh_Hant": "qtbase_zh_TW",
    "pt":      "qtbase_pt_PT",
}


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <qt_translations_dir> <destination_dir>", file=sys.stderr)
        sys.exit(2)

    src_dir = Path(sys.argv[1])
    dst_dir = Path(sys.argv[2])
    dst_dir.mkdir(parents=True, exist_ok=True)

    for dest_code, src_base in LANGS.items():
        src = src_dir / f"{src_base}.qm"
        dst = dst_dir / f"qtbase_{dest_code}.qm"
        if src.exists():
            shutil.copy2(src, dst)
            print(f"  copied {src_base}.qm -> qtbase_{dest_code}.qm")
        else:
            print(f"  skip   {src_base}.qm (not found)")


if __name__ == "__main__":
    main()

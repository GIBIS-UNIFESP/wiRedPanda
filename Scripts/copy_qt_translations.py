#!/usr/bin/env python3
import sys
import shutil
from pathlib import Path

src_dir = Path(sys.argv[1])
dst_dir = Path(sys.argv[2])

LANGS = {
    **{lang: f"qtbase_{lang}" for lang in [
        "ar", "bg", "cs", "da", "de", "es", "fa", "fi", "fr",
        "he", "hr", "hu", "it", "ja", "ko", "lv", "nl", "pl",
        "pt_BR", "ru", "sk", "sv", "tr", "uk",
    ]},
    "zh_Hans": "qtbase_zh_CN",
    "zh_Hant": "qtbase_zh_TW",
    "pt":      "qt_pt_PT",
}

for dest_code, src_base in LANGS.items():
    src = src_dir / f"{src_base}.qm"
    dst = dst_dir / f"qt_{dest_code}.qm"
    if src.exists():
        shutil.copy2(src, dst)
        print(f"  copied {src_base}.qm -> qt_{dest_code}.qm")
    else:
        print(f"  skip   {src_base}.qm (not found)")

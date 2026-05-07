#!/usr/bin/env python3
import sys
import shutil
from pathlib import Path

src_dir = Path(sys.argv[1])
dst_dir = Path(sys.argv[2])
dst_dir.mkdir(parents=True, exist_ok=True)

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

for dest_code, src_base in LANGS.items():
    src = src_dir / f"{src_base}.qm"
    dst = dst_dir / f"qtbase_{dest_code}.qm"
    if src.exists():
        shutil.copy2(src, dst)
        print(f"  copied {src_base}.qm -> qtbase_{dest_code}.qm")
    else:
        print(f"  skip   {src_base}.qm (not found)")

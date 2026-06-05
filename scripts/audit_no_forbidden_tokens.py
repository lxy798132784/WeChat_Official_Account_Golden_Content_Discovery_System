#!/usr/bin/env python3
from pathlib import Path
import re, sys
root=Path(__file__).resolve().parents[1]
patterns=[re.compile(r"github_pat_[A-Za-z0-9_]+"), re.compile(r"ghp_[A-Za-z0-9_]+"), re.compile(r"sk-[A-Za-z0-9]{20,}")]
bad=[]
for p in root.rglob('*'):
    if '.git' in p.parts or 'build' in p.parts or not p.is_file():
        continue
    try: text=p.read_text(errors='ignore')
    except Exception: continue
    if any(x.search(text) for x in patterns): bad.append(str(p.relative_to(root)))
if bad:
    print('Forbidden tokens found:', bad); sys.exit(1)
print('OK: no forbidden tokens')

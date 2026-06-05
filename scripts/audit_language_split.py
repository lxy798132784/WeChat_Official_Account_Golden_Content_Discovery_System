#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
ENGLISH_DOCS = [
    ROOT / "README.md",
    ROOT / "docs" / "en" / "USER_GUIDE.md",
    ROOT / "docs" / "en" / "DEVELOPER_GUIDE.md",
    ROOT / "docs" / "en" / "INSTALL.md",
    ROOT / "docs" / "en" / "PLUGIN_GUIDE.md",
    ROOT / "docs" / "en" / "PRODUCTION_RUNBOOK.md",
    ROOT / "docs" / "en" / "LOCAL_PROXY_ADAPTER.md",
    ROOT / "CONTRIBUTING.md",
    ROOT / "SECURITY.md",
    ROOT / "CODE_OF_CONDUCT.md",
    ROOT / "CHANGELOG.md",
]
CHINESE_DOCS = [
    ROOT / "docs" / "README.zh-CN.md",
    ROOT / "docs" / "zh-CN" / "使用说明.md",
    ROOT / "docs" / "zh-CN" / "开发文档.md",
    ROOT / "docs" / "zh-CN" / "安装部署.md",
    ROOT / "docs" / "zh-CN" / "插件开发.md",
    ROOT / "docs" / "zh-CN" / "生产运行手册.md",
    ROOT / "docs" / "zh-CN" / "本地代理适配器方案A.md",
    ROOT / "docs" / "zh-CN" / "贡献指南.md",
    ROOT / "docs" / "zh-CN" / "安全说明.md",
    ROOT / "docs" / "zh-CN" / "行为准则.md",
    ROOT / "docs" / "zh-CN" / "更新日志.md",
]
CJK = re.compile(r"[\u3400-\u9fff]")
errors = []
for path in ENGLISH_DOCS:
    if not path.exists():
        errors.append(f"missing English doc: {path.relative_to(ROOT)}")
        continue
    text = path.read_text(encoding="utf-8")
    if CJK.search(text):
        errors.append(f"English doc contains CJK characters: {path.relative_to(ROOT)}")
for path in CHINESE_DOCS:
    if not path.exists():
        errors.append(f"missing Chinese doc: {path.relative_to(ROOT)}")
        continue
    text = path.read_text(encoding="utf-8")
    if len(CJK.findall(text)) < 80:
        errors.append(f"Chinese doc has insufficient Chinese content: {path.relative_to(ROOT)}")
if errors:
    for error in errors:
        print(error)
    sys.exit(1)
print("OK: language-separated documentation audit passed")

#!/usr/bin/env python3
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
ENGLISH_DOCS = [
    ROOT / "README.md",
    ROOT / "docs" / "en" / "USER_GUIDE.md",
    ROOT / "docs" / "en" / "DEVELOPER_GUIDE.md",
    ROOT / "docs" / "en" / "INSTALL.md",
    ROOT / "docs" / "en" / "PLUGIN_GUIDE.md",
    ROOT / "docs" / "en" / "PRODUCTION_RUNBOOK.md",
    ROOT / "docs" / "en" / "PRODUCTION_SUITE_RUNBOOK.md",
    ROOT / "docs" / "en" / "LOCAL_PROXY_ADAPTER.md",
    ROOT / "docs" / "en" / "WECHAT_INGESTION_OPTIONS_ABC.md",
    ROOT / "plugins" / "README.md",
    ROOT / ".github" / "ISSUE_TEMPLATE" / "bug_report.md",
    ROOT / ".github" / "ISSUE_TEMPLATE" / "feature_request.md",
    ROOT / ".github" / "pull_request_template.md",
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
    ROOT / "docs" / "zh-CN" / "生产中心运行手册.md",
    ROOT / "docs" / "zh-CN" / "本地代理适配器方案A.md",
    ROOT / "docs" / "zh-CN" / "微信数据接入方案ABC.md",
    ROOT / "docs" / "zh-CN" / "贡献指南.md",
    ROOT / "docs" / "zh-CN" / "安全说明.md",
    ROOT / "docs" / "zh-CN" / "行为准则.md",
    ROOT / "docs" / "zh-CN" / "更新日志.md",
    ROOT / "plugins" / "README.zh-CN.md",
    ROOT / ".github" / "ISSUE_TEMPLATE" / "bug_report.zh-CN.md",
    ROOT / ".github" / "ISSUE_TEMPLATE" / "feature_request.zh-CN.md",
    ROOT / ".github" / "pull_request_template.zh-CN.md",
]
CJK = re.compile(r"[㐀-鿿]")
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
    if len(CJK.findall(text)) < 20:
        errors.append(f"Chinese doc has insufficient Chinese content: {path.relative_to(ROOT)}")
# Guard against future mixed-language docs outside the explicit Chinese locations.
try:
    tracked = subprocess.check_output(["git", "ls-files", "-z"], cwd=ROOT).decode().split("\0")[:-1]
except Exception:
    tracked = []
for rel in tracked:
    path = ROOT / rel
    if path.suffix.lower() not in {".md", ".txt"}:
        continue
    is_chinese_path = "zh-CN" in path.parts or path.name.endswith(".zh-CN.md")
    if not is_chinese_path and CJK.search(path.read_text(encoding="utf-8", errors="ignore")):
        errors.append(f"non-Chinese doc contains CJK characters: {rel}")
if errors:
    for error in sorted(set(errors)):
        print(error)
    sys.exit(1)
print("OK: language-separated documentation audit passed")

#!/usr/bin/env python3
"""Audit Chinese UI text for accidental English leakage.

The allowlist keeps protocol/product/file-format terms that are intentionally visible
in a Chinese desktop application, while flagging untranslated English words in labels,
buttons, logs, tables, and dialogs.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"

ALLOW_SUBSTRINGS = [
    # Product/protocol names accepted in Chinese UI.
    "Windows", "Linux", "macOS", "USB", "Reqable", "mitmproxy", "Fiddler", "Charles", "whistle", "Proxyman",
    # File extensions / filters / sample filenames.
    "*.json", "*.csv", "*.db", "*.sqlite", ".json", ".csv", ".db", ".sqlite",
    "articles.csv", "articles.json", "seeds.csv", "phone-diagnostics.json", "auto-ingestion-queue.json",
    # URLs / endpoints / placeholders that are inherently ASCII.
    "http", "https", "mp.weixin.qq.com", "/mp/getappmsgext", "/mp/appmsg_comment", "weixin.qq.com", "__biz", "mid",
    # Qt/format internals that are not user-visible Chinese prose.
    "QStringLiteral", "UiLanguage", "Chinese", "English", "QFileDialog", "QMessageBox", "QUrl", "QJson", "QProcess",
    "yyyy", "HH", "mm", "ss", "ISODate", "UTF", "QObject", "QHash", "QRegularExpression",
    # Storage/config keys and command tokens that are not shown as Chinese prose.
    "zh", "en", "adb", "shell", "devices", "kill-server", "start-server", "android.intent.action.VIEW",
    "pending", "opening", "opened", "done", "failed", "pass", "warn", "fail", "ready", "warning", "blocked", "healthy",
    "Technology", "Finance", "Media", "Lifestyle", "Other", "unknown", "null", "true", "false",
]

# If a Chinese literal contains these English words, it is almost certainly a leak.
FORBIDDEN_WORDS = re.compile(
    r"\b(ADB|Provider|JSON|JSONL|CSV|URL|Cookie|Header|Token|Score|read|like|comment|old_like|original|"
    r"pass|fail|warn|blocked|ready|healthy|warning|pending|failed|opened|opening|done|"
    r"File|Plugins|Actions|Help|Control Center|Keyword|Auto|Export|Import|SQLite|localhost)\b"
)

STRING_RE = re.compile(r'QStringLiteral\("((?:[^"\\]|\\.)*)"\)|"((?:[^"\\]|\\.)*)"')

failures: list[str] = []
for path in sorted(SRC.glob("*.cpp")):
    text = path.read_text(encoding="utf-8", errors="ignore")
    for lineno, line in enumerate(text.splitlines(), 1):
        # Only inspect literals that contain Chinese; pure English dictionary entries are allowed.
        for match in STRING_RE.finditer(line):
            literal = match.group(1) or match.group(2) or ""
            if not re.search(r"[\u4e00-\u9fff]", literal):
                continue
            stripped = literal
            for allowed in ALLOW_SUBSTRINGS:
                stripped = stripped.replace(allowed, "")
            if FORBIDDEN_WORDS.search(stripped):
                failures.append(f"{path.relative_to(ROOT)}:{lineno}: {literal}")

if failures:
    print("Chinese UI English-leak audit failed:")
    for item in failures:
        print("  " + item)
    sys.exit(1)

print("OK: Chinese UI English-leak audit passed")

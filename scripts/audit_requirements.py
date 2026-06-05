#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
REQUIRED = {
    "include/IContentProvider.h": ["Q_DECLARE_INTERFACE", "IContentProvider_iid"],
    "include/PluginManager.h": ["QPluginLoader", "loadFromDirectory"],
    "include/ProxyTrafficBridge.h": ["frontBuffer_", "backBuffer_", "parsePayload"],
    "src/ProxyTrafficBridge.cpp": ["/mp/getappmsgext", "/mp/appmsg_comment", "read_num", "old_like_num", "comment_num"],
    "src/AdbAutomationEngine.cpp": ["QProcess", "bounded(3500", "input", "swipe"],
    "src/MainWindow.cpp": ["QShortcut", "previewSelectedArticle", "starSelectedSeed", "loadPlugins"],
    "Dockerfile": ["linux/amd64", "linux/arm64", "Mesa", "VNC"],
    "scripts/package-linux.sh": ["tar -czf", "SHA256SUMS"],
    "scripts/package-windows.ps1": ["Compress-Archive", "windows-x64"],
}

missing = []
for rel, needles in REQUIRED.items():
    text = (ROOT / rel).read_text(errors="ignore")
    for needle in needles:
        if needle not in text:
            missing.append(f"{rel}: {needle}")

if missing:
    print("Requirement audit failed:")
    for item in missing:
        print(" -", item)
    sys.exit(1)
print("OK: requirements audit passed")

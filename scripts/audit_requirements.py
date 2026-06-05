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
    "src/MainWindow.cpp": ["QShortcut", "previewSelectedArticle", "starSelectedSeed", "loadPlugins", "exportArticlesCsv", "SeedManagerWidget", "RuntimeLogWidget"],
    "src/ExportController.cpp": ["exportArticlesCsv", "exportArticlesJson", "exportSeedsCsv"],
    "src/SeedManagerWidget.cpp": ["addSeedRequested", "removeSeedRequested", "exportSeedsRequested"],
    "Dockerfile": ["linux/amd64", "linux/arm64", "Mesa", "VNC"],
    "scripts/package-linux.sh": ["tar -czf", "SHA256SUMS", "CHANGELOG.md"],
    "scripts/package-windows.ps1": ["Compress-Archive", "windows-x64", "CHANGELOG.md"],
    ".github/workflows/release.yml": ["softprops/action-gh-release", "SHA256SUMS", "source.tar.gz"],
    "README.md": ["docs/assets/preview.svg", "Seed pool management", "Local Bridge Payload"],
    "docs/INSTALL.md": ["Uninstall", "Build from source"],
    "docs/PLUGIN_GUIDE.md": ["IContentProvider", "Q_PLUGIN_METADATA"],
    "CONTRIBUTING.md": ["verify-all.sh"],
    "SECURITY.md": ["local-first"],
    "CHANGELOG.md": ["v1.0.0"],
    "CODE_OF_CONDUCT.md": ["respectful"],
}

missing = []
for rel, needles in REQUIRED.items():
    path = ROOT / rel
    if not path.exists():
        missing.append(f"{rel}: file missing")
        continue
    text = path.read_text(errors="ignore")
    for needle in needles:
        if needle not in text:
            missing.append(f"{rel}: {needle}")

if missing:
    print("Requirement audit failed:")
    for item in missing:
        print(" -", item)
    sys.exit(1)
print("OK: requirements audit passed")

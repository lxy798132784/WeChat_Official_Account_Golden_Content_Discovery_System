#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
REQUIRED = {
    "include/IContentProvider.h": ["Q_DECLARE_INTERFACE", "IContentProvider_iid"],
    "src/PluginManager.cpp": ["QPluginLoader", "loadFromDirectory"],
    "src/PremiumContentFilterProxyModel.cpp": ["engagement", "commentDensity", "frequencyScore"],
    "src/ProxyTrafficBridge.cpp": ["/mp/getappmsgext", "/mp/appmsg_comment", "sendPayloadToLocalBridge"],
    "src/AppSettings.cpp": ["settings.json", "database_path", "bridge_port"],
    "src/BridgePayloadClient.cpp": ["sampleMetricsPayload", "sampleCommentPayload"],
    "src/ExportController.cpp": ["exportArticlesCsv", "exportArticlesJson", "exportSeedsCsv"],
    "include/SeedManagerWidget.h": ["SeedManagerWidget", "exportSeedsRequested"],
    "src/MainWindow.cpp": ["testLocalBridgePayload", "saveRuntimeSettings", "exportArticlesJson"],
    "Dockerfile": ["--platform=$BUILDPLATFORM", "Mesa OpenGL", "x11vnc", "VNC"],
    ".github/workflows/ci.yml": ["package-linux.sh", "upload-artifact"],
    ".github/workflows/release.yml": ["softprops/action-gh-release", "sha256sum"],
    "scripts/audit_language_split.py": ["English doc contains CJK", "Chinese doc has insufficient Chinese content"],
    "README.md": ["User Guide", "Chinese documentation"],
    "docs/en/PRODUCTION_RUNBOOK.md": ["Production definition", "lawful local data adapter"],
    "docs/README.zh-CN.md": ["文档目录", "当前边界"],
}
errors = []
for rel, needles in REQUIRED.items():
    path = ROOT / rel
    if not path.exists():
        errors.append(f"missing {rel}")
        continue
    text = path.read_text(encoding="utf-8", errors="ignore")
    for needle in needles:
        if needle not in text:
            errors.append(f"{rel}: missing {needle!r}")
if errors:
    for error in errors:
        print(error)
    sys.exit(1)
print("OK: requirements audit passed")

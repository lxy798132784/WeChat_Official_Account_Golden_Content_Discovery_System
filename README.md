# Premium Content Radar

Premium Content Radar is a Qt 6 / C++20 desktop platform for discovering high-value WeChat Official Account content. It implements a plugin-driven host shell, an in-memory premium scoring model, SQLite storage, and a WeChat provider module with ADB orchestration plus a localhost decrypted-traffic bridge.

## 中文简介

全网黄金内容/高价值爆款雷达是一个 Qt 6 / C++20 桌面平台，用于发现微信公众号高价值内容。项目包含主程序壳、`IContentProvider` 插件接口、微信采集插件、内存评分模型、SQLite 批量写入、深色数据终端 UI、Docker Buildx 和 GitHub Actions。

## Features / 功能

- Plugin runtime: `PluginManager` scans the `plugins/` directory with `QPluginLoader`.
- Provider contract: `IContentProvider` with Qt metadata and lifecycle methods.
- WeChat provider module: ADB automation loop, resilient local bridge, and config widget.
- Traffic bridge: accepts only `/mp/getappmsgext` and `/mp/appmsg_comment` JSON payloads on localhost.
- Premium score: engagement rate, comment density, and 30-day frequency.
- SQLite schema: `gzh_seeds` and `articles`, with transactional batching.
- UI modules: main window, dashboard, control panel, data viewer, shortcuts, preview, and star seed action.
- Cross-platform automation: CMake, Linux/Windows package scripts, Dockerfile, and CI workflow.

## Build / 构建

```bash
./scripts/verify-all.sh
```

Manual form:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
ctest --test-dir build --output-on-failure
QT_QPA_PLATFORM=offscreen ./build/premium-content-radar --self-test
```

## Run / 运行

```bash
./build/premium-content-radar
```

The application auto-loads plugins from:

```text
./build/plugins
```

## Local Bridge Payload / 本地桥载荷

The bridge is intentionally local-first. A user-controlled local agent may send JSON to `127.0.0.1:9000`; unknown endpoints are ignored.

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "Example Article",
  "url": "https://example.local/article",
  "appmsgstat": {
    "read_num": 24000,
    "like_num": 1200,
    "old_like_num": 320
  }
}
```

Comment payload:

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "url": "https://example.local/article",
  "comment_count": 88
}
```

## Package / 打包

Linux:

```bash
VERSION=0.1.0 ./scripts/package-linux.sh
```

Windows PowerShell:

```powershell
$env:VERSION="0.1.0"
.\scripts\package-windows.ps1
```

## Docker / 容器

```bash
docker buildx build --platform linux/amd64,linux/arm64 -t premium-content-radar:latest .
```

Runtime image includes Mesa OpenGL, X11 helper tools, and x11vnc-based VNC support.

## Notes / 说明

Real WeChat data ingestion requires a lawful, user-controlled data source and local environment configuration. The repository does not embed credentials, private traffic data, or third-party account secrets. The bridge only defines the local IPC contract and rejects unrelated endpoints.

# Premium Content Radar

Premium Content Radar is a Qt 6 / C++20 desktop platform for discovering high-value WeChat Official Account content. It implements a plugin-driven host shell, an in-memory premium scoring model, SQLite storage, and a WeChat ingestion plugin skeleton with ADB orchestration and local traffic bridge components.

## 中文简介

全网黄金内容/高价值爆款雷达是一个 Qt 6 / C++20 桌面平台，用于发现微信公众号高价值内容。项目包含主程序壳、插件接口、微信采集插件、内存评分模型、SQLite 批量写入、深色数据终端 UI、Docker 和 GitHub Actions。

## Features / 功能

- Plugin contract: `IContentProvider` with Qt plugin metadata.
- WeChat provider module: ADB automation loop and localhost traffic bridge.
- Premium score: engagement rate, comment density, and 30-day frequency.
- SQLite schema: `gzh_seeds` and `articles`.
- UI modules: main window, dashboard, control panel, and data viewer.
- Cross-platform automation: CMake, Dockerfile, and CI workflow.

## Build / 构建

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
ctest --test-dir build --output-on-failure
./build/premium-content-radar --self-test
```

## Run / 运行

```bash
./build/premium-content-radar
```

## Notes / 说明

The repository provides a production-oriented foundation. Real WeChat data ingestion requires a lawful, user-controlled data source and local environment configuration. The plugin surfaces the architecture without embedding credentials or private traffic data.

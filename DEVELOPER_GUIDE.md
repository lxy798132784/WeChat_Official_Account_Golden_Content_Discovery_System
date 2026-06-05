# Developer Guide / 开发者指南

## Architecture / 架构

The host shell owns global UI, plugin lifecycle, in-memory scoring, and database persistence. Dynamic providers implement `IContentProvider` and can be loaded through Qt's plugin system.

主程序负责全局 UI、插件生命周期、内存评分和数据库持久化。动态供应商插件实现 `IContentProvider`，可通过 Qt 插件系统加载。

## Threading / 线程模型

`AdbAutomationEngine` runs in a `QThread` and communicates through queued signals. `ProxyTrafficBridge` accepts localhost TCP payloads and uses a mutex-protected queue to decouple network bursts from UI refresh.

`AdbAutomationEngine` 在独立 `QThread` 中运行，通过队列信号通信。`ProxyTrafficBridge` 接收本地 TCP 数据，并用互斥队列隔离网络峰值和 UI 刷新。

## Scoring / 评分

`PremiumContentFilterProxyModel` computes:

```text
Score = W_eng * EngagementRate + W_comm * CommentDensity + W_freq * FrequencyScore
```

## Plugin Expansion / 插件扩展

Create a shared library, implement `IContentProvider`, add `Q_PLUGIN_METADATA`, and install the library under `plugins/`.

# Developer Guide / 开发者指南

## Architecture / 架构

The host shell owns the global UI, plugin lifecycle, in-memory scoring, and SQLite persistence. Runtime plugins implement `IContentProvider` and are discovered by `PluginManager` through Qt's `QPluginLoader`.

主程序负责全局 UI、插件生命周期、内存评分和 SQLite 持久化。运行时插件实现 `IContentProvider`，并由 `PluginManager` 通过 Qt `QPluginLoader` 动态发现。

## Plugin Lifecycle / 插件生命周期

1. The application starts and computes `applicationDirPath()/plugins`.
2. `PluginManager::loadFromDirectory()` loads shared libraries.
3. Each plugin is cast to `IContentProvider`.
4. The host calls `start()` and then drains records every second.
5. The database batches incoming records and the table refreshes from the in-memory proxy model.

1. 应用启动后计算 `applicationDirPath()/plugins`。
2. `PluginManager::loadFromDirectory()` 加载共享库。
3. 每个插件实例被转换为 `IContentProvider`。
4. 主程序调用 `start()`，随后每秒拉取一次记录。
5. 数据库批量写入记录，表格通过内存代理模型刷新。

## Threading / 线程模型

`AdbAutomationEngine` runs in a `QThread`, drives non-blocking `QProcess` ADB commands, and applies randomized 3.5s-7.5s delays. `ProxyTrafficBridge` runs as a localhost `QTcpServer`; it parses payloads on socket readiness and pushes records into a mutex-protected back buffer. `drainRecords()` swaps back/front buffers so the UI thread holds the lock briefly.

`AdbAutomationEngine` 在独立 `QThread` 中运行，通过非阻塞 `QProcess` 发出 ADB 命令，并使用 3.5s-7.5s 随机延迟。`ProxyTrafficBridge` 是本地 `QTcpServer`；socket 可读时解析载荷并写入互斥保护的 back buffer。`drainRecords()` 交换 front/back buffer，使 UI 线程只短时间持锁。

## Traffic Contract / 流量桥协议

Accepted endpoints:

- `/mp/getappmsgext`
- `/mp/appmsg_comment`

Accepted metric keys:

- `read_num`
- `like_num`
- `old_like_num`
- `comment_num`
- `comment_count`
- nested `appmsgstat` and `comment_info`

Unknown endpoints are ignored deliberately. This keeps the bridge narrow, testable, and privacy-preserving.

只接受以上两个端点。未知端点会被主动忽略，保证本地桥的边界清晰、可测试并保护隐私。

## Scoring / 评分

`PremiumContentFilterProxyModel` computes:

```text
Score = W_eng * EngagementRate + W_comm * CommentDensity + W_freq * FrequencyScore
EngagementRate = (like_num + old_like_num) / read_num
CommentDensity = comment_num / read_num * 10000
FrequencyScore = min(100, article_count_30d / 30 * 100)
```

Rows pass only when `read_num >= MinReadThreshold` and `Score >= baseline`.

## UI Interaction / UI 交互

- Spacebar: preview selected article.
- Ctrl+S: star the selected publisher seed.
- Esc: clear search and reset slider defaults.
- Menu actions expose sample loading, plugin loading, preview, and star seed.

- 空格：预览选中文章。
- Ctrl+S：标星选中账号。
- Esc：清空搜索并重置滑杆。
- 菜单暴露示例加载、插件加载、预览和标星账号动作。

## Testing / 测试

Run all gates:

```bash
./scripts/verify-all.sh
```

The current test suite covers:

- SQLite seed/article persistence.
- Premium score filtering.
- `/mp/getappmsgext` parsing.
- `/mp/appmsg_comment` parsing.
- unknown endpoint rejection.
- secret scan and requirements audit.

当前测试覆盖 SQLite 持久化、评分过滤、两个微信指标端点解析、未知端点拒绝、密钥扫描和需求审计。

## Plugin Expansion / 插件扩展

Create a shared library, implement `IContentProvider`, add `Q_PLUGIN_METADATA`, link against `radar_core`, and install the library under `plugins/`.

创建共享库，实现 `IContentProvider`，添加 `Q_PLUGIN_METADATA`，链接 `radar_core`，并把插件安装到 `plugins/` 目录。

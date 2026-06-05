# Plugin Guide / 插件指南

Implement `IContentProvider`, add `Q_PLUGIN_METADATA`, add `Q_INTERFACES(IContentProvider)`, and install the shared library into `plugins/`.

实现 `IContentProvider`，添加 `Q_PLUGIN_METADATA` 和 `Q_INTERFACES(IContentProvider)`，并把共享库安装到 `plugins/`。

Required lifecycle:

- `providerId()`
- `displayName()`
- `start()`
- `stop()`
- `drainRecords()`

Host behavior:

- load plugins from `applicationDirPath()/plugins`
- keep loaders alive
- start providers
- drain records every second
- persist into SQLite

主程序行为：从 `applicationDirPath()/plugins` 加载插件，保持 loader 生命周期，启动供应商，每秒拉取记录并写入 SQLite。

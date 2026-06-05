# Plugins / 插件

Runtime plugins are loaded from the executable directory's `plugins/` folder with `QPluginLoader`.

运行时插件会从可执行文件同级 `plugins/` 目录通过 `QPluginLoader` 加载。

Current provider:

- `WeChatProviderPlugin`: local-first WeChat Official Account metrics provider. It starts the localhost bridge and ADB automation scaffold.

当前供应商：

- `WeChatProviderPlugin`：本地优先的微信公众号指标供应商，启动本地桥和 ADB 自动化骨架。

Unknown libraries or interface-mismatched plugins are skipped and logged.

未知库或接口不匹配插件会被跳过并写入日志。

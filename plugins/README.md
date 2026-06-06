# Plugins

Runtime plugins are loaded from the executable directory's `plugins/` folder with `QPluginLoader`.

Current provider:

- `WeChatProviderPlugin`: local-first WeChat Official Account metrics provider. It starts the localhost bridge and ADB automation scaffold.

Unknown libraries or interface-mismatched plugins are skipped and logged.

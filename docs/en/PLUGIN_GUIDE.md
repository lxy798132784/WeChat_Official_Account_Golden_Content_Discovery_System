# Plugin Guide

## Contract

A provider plugin must implement `IContentProvider`:

```cpp
class MyProvider final : public QObject, public IContentProvider {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID IContentProvider_iid)
  Q_INTERFACES(IContentProvider)
 public:
  QString providerId() const override;
  QString displayName() const override;
  bool start(QString* errorMessage) override;
  void stop() override;
  QVector<ContentRecord> drainRecords() override;
};
```

## Lifecycle

1. The host scans the plugin directory.
2. `QPluginLoader` creates an instance.
3. The host casts it to `IContentProvider`.
4. The host calls `start()`.
5. The host calls `drainRecords()` every second.
6. On shutdown, the host calls `stop()` through plugin object lifetime cleanup.

## Record fields

Populate as many fields as available:

- `title`
- `url`
- `accountName`
- `gzhId`
- `category`
- `readNum`
- `likeNum`
- `oldLikeNum`
- `commentNum`
- `articleCount30d`
- `publishedAt`

The URL is the de-duplication key in SQLite.

## Build integration

Link against `radar_core` and install the shared library into the runtime plugin directory. The bundled WeChat provider is the reference implementation.

## Safety expectations

- Keep credentials outside the plugin binary.
- Expose errors through `errorMessage` and logs.
- Do not block the UI thread.
- Use localhost or user-selected local paths by default.

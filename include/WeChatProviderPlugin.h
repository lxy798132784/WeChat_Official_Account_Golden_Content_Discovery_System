#pragma once

#include <QObject>

#include "AdbAutomationEngine.h"
#include "IContentProvider.h"
#include "ProxyTrafficBridge.h"

/**
 * @brief WeChat Official Account ingestion plugin.
 *
 * Runtime configuration is intentionally local and process-scoped:
 * - PREMIUM_RADAR_BRIDGE_PORT controls the localhost bridge port.
 * - PREMIUM_RADAR_ENABLE_ADB enables or disables the optional ADB helper loop.
 */
class WeChatProviderPlugin final : public QObject, public IContentProvider {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID IContentProvider_iid)
  Q_INTERFACES(IContentProvider)

 public:
  QString providerId() const override;
  QString displayName() const override;
  bool start(QString* errorMessage) override;
  void stop() override;
  QVector<ContentRecord> drainRecords() override;

 private:
  quint16 configuredBridgePort() const;
  bool adbEnabled() const;

  AdbAutomationEngine adbEngine_;
  ProxyTrafficBridge bridge_;
};

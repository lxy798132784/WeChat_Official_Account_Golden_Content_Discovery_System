#pragma once
#include <QObject>
#include "AdbAutomationEngine.h"
#include "IContentProvider.h"
#include "ProxyTrafficBridge.h"
/** @brief 微信公众号采集插件 / WeChat official account ingestion plugin */
class WeChatProviderPlugin final : public QObject, public IContentProvider { Q_OBJECT Q_PLUGIN_METADATA(IID IContentProvider_iid) Q_INTERFACES(IContentProvider)
 public: QString providerId() const override; QString displayName() const override; bool start(QString* errorMessage) override; void stop() override; QVector<ContentRecord> drainRecords() override;
 private: AdbAutomationEngine adbEngine_; ProxyTrafficBridge bridge_; };

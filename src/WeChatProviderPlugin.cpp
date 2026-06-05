#include "WeChatProviderPlugin.h"

#include <QByteArray>

namespace {
constexpr quint16 kDefaultBridgePort = 9000;
}

QString WeChatProviderPlugin::providerId() const {
  return QStringLiteral("wechat");
}

QString WeChatProviderPlugin::displayName() const {
  return QStringLiteral("WeChat Official Account Provider");
}

quint16 WeChatProviderPlugin::configuredBridgePort() const {
  bool ok = false;
  const int value = qEnvironmentVariableIntValue("PREMIUM_RADAR_BRIDGE_PORT", &ok);
  if (!ok || value < 1024 || value > 65535) {
    return kDefaultBridgePort;
  }
  return static_cast<quint16>(value);
}

bool WeChatProviderPlugin::adbEnabled() const {
  const QByteArray value = qgetenv("PREMIUM_RADAR_ENABLE_ADB").trimmed().toLower();
  return value == "1" || value == "true" || value == "yes" || value == "on";
}

bool WeChatProviderPlugin::start(QString* errorMessage) {
  const quint16 port = configuredBridgePort();
  if (!bridge_.isListening() && !bridge_.startBridge(port)) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("Bridge listen failed on 127.0.0.1:%1: %2")
                          .arg(port)
                          .arg(bridge_.errorString());
    }
    return false;
  }

  if (adbEnabled() && !adbEngine_.isRunning()) {
    adbEngine_.start();
  }
  return true;
}

void WeChatProviderPlugin::stop() {
  if (adbEngine_.isRunning()) {
    adbEngine_.requestStop();
    adbEngine_.wait(3000);
  }
  bridge_.close();
}

QVector<ContentRecord> WeChatProviderPlugin::drainRecords() {
  return bridge_.drainRecords();
}

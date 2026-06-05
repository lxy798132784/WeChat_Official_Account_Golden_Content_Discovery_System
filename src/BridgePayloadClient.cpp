#include "BridgePayloadClient.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "ProxyTrafficBridge.h"

bool BridgePayloadClient::sendPayload(const QString& host, quint16 port, const QByteArray& payload,
                                      QString* errorMessage) {
  if (host != QStringLiteral("127.0.0.1") && host != QStringLiteral("localhost")) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("Only localhost bridge targets are supported");
    }
    return false;
  }
  return ProxyTrafficBridge::sendPayloadToLocalBridge(port, payload, errorMessage);
}

QByteArray BridgePayloadClient::sampleMetricsPayload() {
  QJsonObject stat;
  stat.insert(QStringLiteral("read_num"), 36000);
  stat.insert(QStringLiteral("like_num"), 1600);
  stat.insert(QStringLiteral("old_like_num"), 500);
  QJsonObject object;
  object.insert(QStringLiteral("endpoint"), QStringLiteral("/mp/getappmsgext"));
  object.insert(QStringLiteral("title"), QStringLiteral("Local Bridge Production Smoke Article"));
  object.insert(QStringLiteral("url"), QStringLiteral("https://example.local/bridge-smoke"));
  object.insert(QStringLiteral("account_name"), QStringLiteral("Local Bridge Lab"));
  object.insert(QStringLiteral("gzh_id"), QStringLiteral("gh_bridge_smoke"));
  object.insert(QStringLiteral("category"), QStringLiteral("Operations"));
  object.insert(QStringLiteral("publish_time"), QStringLiteral("2026-06-05T09:30:00Z"));
  object.insert(QStringLiteral("article_count_30d"), 11);
  object.insert(QStringLiteral("appmsgstat"), stat);
  return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

QByteArray BridgePayloadClient::sampleCommentPayload() {
  QJsonObject object;
  object.insert(QStringLiteral("path"), QStringLiteral("/mp/appmsg_comment?action=getcomment"));
  object.insert(QStringLiteral("title"), QStringLiteral("Local Bridge Production Smoke Article"));
  object.insert(QStringLiteral("url"), QStringLiteral("https://example.local/bridge-smoke"));
  object.insert(QStringLiteral("comment_count"), 96);
  return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

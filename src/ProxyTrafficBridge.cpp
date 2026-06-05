#include "ProxyTrafficBridge.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QTcpSocket>

namespace {

QString endpointFromObject(const QJsonObject& object) {
  const QString endpoint = object.value(QStringLiteral("endpoint")).toString();
  if (!endpoint.isEmpty()) {
    return endpoint;
  }
  return object.value(QStringLiteral("path")).toString();
}

int metricInt(const QJsonObject& root, const QJsonObject& nested, const QString& key) {
  if (nested.contains(key)) {
    return nested.value(key).toInt();
  }
  return root.value(key).toInt();
}

}  // namespace

ProxyTrafficBridge::ProxyTrafficBridge(QObject* parent) : QTcpServer(parent) {
  connect(this, &QTcpServer::newConnection, this, &ProxyTrafficBridge::acceptConnection);
}

bool ProxyTrafficBridge::startBridge(quint16 port) {
  return listen(QHostAddress::LocalHost, port);
}

bool ProxyTrafficBridge::sendPayloadToLocalBridge(quint16 port, const QByteArray& payload,
                                                  QString* errorMessage) {
  QTcpSocket socket;
  socket.connectToHost(QHostAddress::LocalHost, port);
  if (!socket.waitForConnected(3000)) {
    if (errorMessage != nullptr) {
      *errorMessage = socket.errorString();
    }
    return false;
  }
  socket.write(payload);
  socket.flush();
  socket.waitForBytesWritten(3000);
  socket.disconnectFromHost();
  return true;
}

std::optional<ContentRecord> ProxyTrafficBridge::parsePayload(const QByteArray& payload) {
  const QJsonDocument document = QJsonDocument::fromJson(payload);
  if (!document.isObject()) {
    return std::nullopt;
  }

  const QJsonObject root = document.object();
  const QString endpoint = endpointFromObject(root);
  const bool isMetricsEndpoint = endpoint.contains(QStringLiteral("/mp/getappmsgext"));
  const bool isCommentEndpoint = endpoint.contains(QStringLiteral("/mp/appmsg_comment"));
  if (!isMetricsEndpoint && !isCommentEndpoint) {
    return std::nullopt;
  }

  ContentRecord record;
  record.title = root.value(QStringLiteral("title")).toString(QStringLiteral("Captured Article"));
  record.url = root.value(QStringLiteral("url")).toString();
  record.category = root.value(QStringLiteral("category")).toString();
  record.accountName = root.value(QStringLiteral("account_name")).toString();
  record.gzhId = root.value(QStringLiteral("gzh_id")).toString();
  record.articleCount30d = root.value(QStringLiteral("article_count_30d")).toInt();

  const QJsonObject appMsgStat = root.value(QStringLiteral("appmsgstat")).toObject();
  const QJsonObject commentInfo = root.value(QStringLiteral("comment_info")).toObject();
  record.readNum = metricInt(root, appMsgStat, QStringLiteral("read_num"));
  record.likeNum = metricInt(root, appMsgStat, QStringLiteral("like_num"));
  record.oldLikeNum = metricInt(root, appMsgStat, QStringLiteral("old_like_num"));
  record.commentNum = metricInt(root, commentInfo, QStringLiteral("comment_num"));
  if (record.commentNum == 0) {
    record.commentNum = metricInt(root, commentInfo, QStringLiteral("comment_count"));
  }

  if (record.url.isEmpty() && record.title == QStringLiteral("Captured Article")) {
    return std::nullopt;
  }
  return record;
}

void ProxyTrafficBridge::acceptConnection() {
  while (hasPendingConnections()) {
    QTcpSocket* socket = nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
      const std::optional<ContentRecord> parsed = parsePayload(socket->readAll());
      if (!parsed.has_value()) {
        return;
      }
      QMutexLocker lock(&mutex_);
      backBuffer_.enqueue(*parsed);
    });
    connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
  }
}

QVector<ContentRecord> ProxyTrafficBridge::drainRecords() {
  QVector<ContentRecord> output;
  QMutexLocker lock(&mutex_);
  frontBuffer_.swap(backBuffer_);
  while (!frontBuffer_.isEmpty()) {
    output.push_back(frontBuffer_.dequeue());
  }
  return output;
}

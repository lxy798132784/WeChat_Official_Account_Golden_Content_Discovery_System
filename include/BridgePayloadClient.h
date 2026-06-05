#pragma once

#include <QString>

class BridgePayloadClient final {
 public:
  static bool sendPayload(const QString& host, quint16 port, const QByteArray& payload,
                          QString* errorMessage = nullptr);
  static QByteArray sampleMetricsPayload();
  static QByteArray sampleCommentPayload();
};

#pragma once

#include <QMutex>
#include <QQueue>
#include <QTcpServer>
#include <optional>

#include "content_record.h"

/**
 * @brief 本地解密流量桥
 * Local decrypted traffic bridge
 *
 * @details 监听 127.0.0.1:9000，接收外部合法本地代理发送的 JSON 载荷，
 * 只接受 /mp/getappmsgext 与 /mp/appmsg_comment 两类微信公众平台指标端点。
 * The bridge listens on localhost, accepts JSON payloads from a user-controlled
 * local agent, and only parses /mp/getappmsgext plus /mp/appmsg_comment metric
 * endpoints.
 */
class ProxyTrafficBridge final : public QTcpServer {
  Q_OBJECT
 public:
  explicit ProxyTrafficBridge(QObject* parent = nullptr);

  /**
   * @brief 启动本地监听
   * Start the localhost listener
   *
   * @param port 监听端口 / Listening port
   * @return 是否启动成功 / Whether the listener started successfully
   */
  bool startBridge(quint16 port = 9000);
  static bool sendPayloadToLocalBridge(quint16 port, const QByteArray& payload,
                                       QString* errorMessage = nullptr);

  /**
   * @brief 解析单个 JSON 载荷
   * Parse one JSON payload
   *
   * @details 静态函数便于单元测试；未知端点、非法 JSON 或空 URL 会返回空。
   * Static for unit testing; unknown endpoints, invalid JSON, or empty URLs are
   * rejected.
   *
   * @param payload JSON 字节串 / JSON bytes
   * @return 解析后的记录或空 / Parsed record or empty optional
   */
  static std::optional<ContentRecord> parsePayload(const QByteArray& payload);

  /**
   * @brief 交换双缓冲并取出记录
   * Swap double buffers and drain records
   *
   * @return 当前批次记录 / Current batch records
   */
  QVector<ContentRecord> drainRecords();

 private slots:
  void acceptConnection();

 private:
  QMutex mutex_;
  QQueue<ContentRecord> frontBuffer_;
  QQueue<ContentRecord> backBuffer_;
};

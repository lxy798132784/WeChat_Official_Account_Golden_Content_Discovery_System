#pragma once
#include <QMutex>
#include <QQueue>
#include <QTcpServer>
#include "content_record.h"
/** @brief 本地流量桥 / Local traffic bridge */
class ProxyTrafficBridge final : public QTcpServer { Q_OBJECT
 public: explicit ProxyTrafficBridge(QObject* parent=nullptr); bool startBridge(quint16 port=9000); QVector<ContentRecord> drainRecords();
 private slots: void acceptConnection();
 private: QMutex mutex_; QQueue<ContentRecord> backBuffer_; };

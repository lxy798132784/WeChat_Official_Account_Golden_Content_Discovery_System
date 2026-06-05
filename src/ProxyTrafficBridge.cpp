#include "ProxyTrafficBridge.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QTcpSocket>
ProxyTrafficBridge::ProxyTrafficBridge(QObject* parent):QTcpServer(parent){connect(this,&QTcpServer::newConnection,this,&ProxyTrafficBridge::acceptConnection);}
bool ProxyTrafficBridge::startBridge(quint16 port){return listen(QHostAddress::LocalHost,port);}
void ProxyTrafficBridge::acceptConnection(){while(hasPendingConnections()){QTcpSocket* socket=nextPendingConnection(); connect(socket,&QTcpSocket::readyRead,this,[this,socket](){const auto doc=QJsonDocument::fromJson(socket->readAll()); if(!doc.isObject()) return; const auto o=doc.object(); ContentRecord r; r.title=o.value("title").toString("Captured Article"); r.url=o.value("url").toString(); r.readNum=o.value("read_num").toInt(); r.likeNum=o.value("like_num").toInt(); r.oldLikeNum=o.value("old_like_num").toInt(); r.commentNum=o.value("comment_num").toInt(); QMutexLocker lock(&mutex_); backBuffer_.enqueue(r);}); connect(socket,&QTcpSocket::disconnected,socket,&QObject::deleteLater);}}
QVector<ContentRecord> ProxyTrafficBridge::drainRecords(){QVector<ContentRecord> out; QMutexLocker lock(&mutex_); while(!backBuffer_.isEmpty()) out.push_back(backBuffer_.dequeue()); return out;}

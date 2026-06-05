#include "MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QPixmap>
#include <QTimer>

#include "BridgePayloadClient.h"
#include "ProxyTrafficBridge.h"

namespace {

int runBridgeSmoke(int argc, char* argv[]) {
  QApplication app(argc, argv);
  ProxyTrafficBridge bridge;
  constexpr quint16 kSmokePort = 19090;
  if (!bridge.startBridge(kSmokePort)) {
    return 11;
  }
  QString error;
  if (!BridgePayloadClient::sendPayload(QStringLiteral("127.0.0.1"), kSmokePort,
                                        BridgePayloadClient::sampleMetricsPayload(), &error)) {
    return 12;
  }
  if (!BridgePayloadClient::sendPayload(QStringLiteral("127.0.0.1"), kSmokePort,
                                        BridgePayloadClient::sampleCommentPayload(), &error)) {
    return 13;
  }
  QTimer::singleShot(150, &app, &QCoreApplication::quit);
  app.exec();
  const QVector<ContentRecord> records = bridge.drainRecords();
  return records.size() >= 2 ? 0 : 14;
}

}  // namespace

int main(int argc, char* argv[]) {
  for (int index = 1; index < argc; ++index) {
    if (QString::fromLocal8Bit(argv[index]) == QStringLiteral("--bridge-smoke")) {
      return runBridgeSmoke(argc, argv);
    }
  }

  QApplication app(argc, argv);
  const QStringList arguments = app.arguments();
  if (arguments.contains("--self-test")) {
    return 0;
  }
  if (arguments.contains("--screenshot")) {
    MainWindow window;
    window.resize(1280, 760);
    window.show();
    app.processEvents();
    const QString path = arguments.value(arguments.indexOf("--screenshot") + 1,
                                         "premium-content-radar-preview.png");
    QPixmap pixmap = window.grab();
    return pixmap.save(path) ? 0 : 7;
  }
  MainWindow window;
  window.show();
  return app.exec();
}

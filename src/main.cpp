#include "MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QPixmap>
#include <QTimer>

#include "BridgePayloadClient.h"
#include "ProxyTrafficBridge.h"
#include "WeChatSearchAutomationController.h"

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

QStringList commandLineArguments(int argc, char* argv[]) {
  QStringList args;
  args.reserve(argc);
  for (int i = 0; i < argc; ++i) {
    args.push_back(QString::fromLocal8Bit(argv[i]));
  }
  return args;
}

int optionValue(const QStringList& args, const QString& name, int fallback) {
  const int index = args.indexOf(name);
  if (index >= 0 && index + 1 < args.size()) {
    bool ok = false;
    const int value = args.at(index + 1).toInt(&ok);
    if (ok) {
      return value;
    }
  }
  return fallback;
}

}  // namespace

int main(int argc, char* argv[]) {
  for (int index = 1; index < argc; ++index) {
    if (QString::fromLocal8Bit(argv[index]) == QStringLiteral("--bridge-smoke")) {
      return runBridgeSmoke(argc, argv);
    }
    if (QString::fromLocal8Bit(argv[index]) == QStringLiteral("--wechat-collect-smoke")) {
      const QStringList args = commandLineArguments(argc, argv);
      WeChatSearchAutomationController::Options options;
      options.enabled = true;
      options.autoLocateSearch = true;
      options.tapNetworkResults = true;
      options.waitMs = optionValue(args, QStringLiteral("--wait-ms"), 3500);
      WeChatSearchAutomationController::CollectionCriteria criteria;
      criteria.maxArticles = optionValue(args, QStringLiteral("--max-articles"), 2);
      criteria.minRead = optionValue(args, QStringLiteral("--min-read"), 0);
      criteria.minLike = optionValue(args, QStringLiteral("--min-like"), 0);
      criteria.minOldLike = optionValue(args, QStringLiteral("--min-old-like"), 0);
      criteria.minComment = optionValue(args, QStringLiteral("--min-comment"), 0);
      criteria.perArticleWaitMs = optionValue(args, QStringLiteral("--per-article-wait-ms"), 14000);
      const QString keyword = index + 1 < argc ? QString::fromLocal8Bit(argv[index + 1]) : QStringLiteral("AI");
      const auto summary = WeChatSearchAutomationController::runCollection({keyword}, options, criteria);
      qInfo().noquote() << QStringLiteral(
                               "wechat_collect_smoke attempted=%1 opened=%2 captured=%3 accepted=%4 rejected_threshold=%5 rejected_duplicate=%6 failed=%7 reasons=%8")
                               .arg(summary.attempted)
                               .arg(summary.opened)
                               .arg(summary.captured)
                               .arg(summary.accepted)
                               .arg(summary.rejectedByThreshold)
                               .arg(summary.rejectedAsDuplicate)
                               .arg(summary.failed)
                               .arg(summary.failureReasons.join(QStringLiteral(",")));
      return summary.accepted >= criteria.maxArticles ? 0 : 16;
    }
    if (QString::fromLocal8Bit(argv[index]) == QStringLiteral("--wechat-search-smoke")) {
      WeChatSearchAutomationController::Options options;
      options.enabled = true;
      options.autoLocateSearch = true;
      options.tapNetworkResults = true;
      options.waitMs = 3500;
      const QString keyword = index + 1 < argc ? QString::fromLocal8Bit(argv[index + 1]) : QStringLiteral("AI");
      const auto result = WeChatSearchAutomationController::run({keyword}, options);
      qInfo().noquote() << QStringLiteral("wechat_search_smoke stage=%1 success=%2 message=%3")
                               .arg(result.stage, result.success ? QStringLiteral("true") : QStringLiteral("false"), result.message);
      return (result.stage == QStringLiteral("metrics_visible") || result.stage == QStringLiteral("article_visible")) ? 0 : 15;
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

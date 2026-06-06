#include <QStandardItemModel>
#include <QtTest/QtTest>

#include "AutoIngestionController.h"
#include "KeywordDiscoveryController.h"
#include "BridgePayloadClient.h"
#include "DatabaseController.h"
#include "ExportController.h"
#include "PremiumContentFilterProxyModel.h"
#include "PhoneDiagnosticsController.h"
#include "ProductionSuiteController.h"
#include "ProxyTrafficBridge.h"
#include "UiText.h"
#include "WeChatSearchAutomationController.h"

class RadarCoreTest : public QObject {
  Q_OBJECT

 private slots:
  void databaseStoresArticles();
  void seedPoolAddListRemove();
  void exportArticlesCsvAndJson();
  void proxyScoresAndFilters();
  void trafficBridgeParsesGetAppMsgExtPayload();
  void trafficBridgeParsesCommentPayload();
  void trafficBridgeIgnoresUnknownEndpoint();
  void bridgePayloadClientSamplesParse();
  void productionSuiteProxyReplayAndScoring();
  void productionSuiteQualityTrendAnalysisDelivery();
  void quickStartUiTextKeys();
  void weChatSearchAutomationPlan();
  void autoIngestionQueueAndAdbArgs();
};

void RadarCoreTest::databaseStoresArticles() {
  QTemporaryDir dir;
  DatabaseController db;
  QVERIFY(db.open(dir.filePath("radar.db")));
  QVERIFY(db.addSeed("gh_test", "Test Account", "Tech"));

  ContentRecord record;
  record.title = "Golden Article";
  record.url = "https://example.test/a";
  record.readNum = 10000;
  record.likeNum = 500;
  record.oldLikeNum = 200;
  record.commentNum = 80;
  record.articleCount30d = 12;
  record.publishTime = QDateTime::fromString("2026-06-05T08:00:00Z", Qt::ISODate);

  QVERIFY(db.enqueueArticle(record));
  QVERIFY(db.flush());
  QCOMPARE(db.articleCount(), 1);
  QCOMPARE(db.listArticles().first().title, QString("Golden Article"));
  QVERIFY(db.listArticles().first().publishTime.isValid());
}

void RadarCoreTest::seedPoolAddListRemove() {
  QTemporaryDir dir;
  DatabaseController db;
  QVERIFY(db.open(dir.filePath("radar.db")));
  QVERIFY(db.addSeed("gh_a", "Account A", "Tech"));
  QVERIFY(db.addSeed("gh_b", "Account B", "Media"));
  QCOMPARE(db.listSeeds().size(), 2);
  QVERIFY(db.removeSeed("gh_a"));
  QCOMPARE(db.listSeeds().size(), 1);
  QCOMPARE(db.listSeeds().first().gzhId, QString("gh_b"));
}

void RadarCoreTest::exportArticlesCsvAndJson() {
  QTemporaryDir dir;
  ContentRecord record;
  record.title = "Export Article";
  record.url = "https://example.test/export";
  record.accountName = "Export Lab";
  record.gzhId = "gh_export";
  record.category = "Tech";
  record.readNum = 123;
  record.likeNum = 12;
  record.publishTime = QDateTime::fromString("2026-06-05T08:00:00Z", Qt::ISODate);
  QVector<ContentRecord> records{record};
  const QString csvPath = dir.filePath("articles.csv");
  const QString jsonPath = dir.filePath("articles.json");
  QVERIFY(ExportController::exportArticlesCsv(records, csvPath));
  QVERIFY(ExportController::exportArticlesJson(records, jsonPath));
  QFile csv(csvPath);
  QFile json(jsonPath);
  QVERIFY(csv.open(QIODevice::ReadOnly));
  QVERIFY(json.open(QIODevice::ReadOnly));
  const QString csvText = QString::fromUtf8(csv.readAll());
  const QString jsonText = QString::fromUtf8(json.readAll());
  QVERIFY(csvText.contains("Export Article"));
  QVERIFY(csvText.contains("publish_time"));
  QVERIFY(jsonText.contains("read_num"));
  QVERIFY(jsonText.contains("publish_time"));
}

void RadarCoreTest::proxyScoresAndFilters() {
  QStandardItemModel model;
  model.setHorizontalHeaderLabels({"Title", "Account", "Category", "PublishTime", "Read", "Like",
                                   "OldLike", "Comment", "Freq30d", "Score", "URL"});
  model.appendRow({new QStandardItem("A"), new QStandardItem("Acct"),
                   new QStandardItem("Tech"), new QStandardItem("2026-06-05 08:00"),
                   new QStandardItem("10000"), new QStandardItem("500"),
                   new QStandardItem("100"), new QStandardItem("50"),
                   new QStandardItem("10"), new QStandardItem(""), new QStandardItem("u")});

  PremiumContentFilterProxyModel proxy;
  proxy.setSourceModel(&model);
  proxy.setMinimums(1000, 1.0);
  QVERIFY(proxy.rowCount() > 0);
  QVERIFY(proxy.scoreForSourceRow(0) > 0.0);
}

void RadarCoreTest::trafficBridgeParsesGetAppMsgExtPayload() {
  const QByteArray payload = R"({
    "endpoint": "/mp/getappmsgext",
    "title": "Metrics Article",
    "url": "https://example.test/article",
    "category": "Tech",
    "account_name": "Metric Lab",
    "gzh_id": "gh_metric",
    "article_count_30d": 16,
    "publish_time": "2026-06-05T08:30:00Z",
    "appmsgstat": {
      "read_num": 24000,
      "like_num": 1200,
      "old_like_num": 320
    }
  })";

  auto parsed = ProxyTrafficBridge::parsePayload(payload);
  QVERIFY(parsed.has_value());
  QCOMPARE(parsed->title, QString("Metrics Article"));
  QCOMPARE(parsed->readNum, 24000);
  QCOMPARE(parsed->likeNum, 1200);
  QCOMPARE(parsed->oldLikeNum, 320);
  QCOMPARE(parsed->articleCount30d, 16);
  QVERIFY(parsed->publishTime.isValid());
}

void RadarCoreTest::trafficBridgeParsesCommentPayload() {
  const QByteArray payload = R"({
    "path": "/mp/appmsg_comment?action=getcomment",
    "title": "Comment Article",
    "url": "https://example.test/comment",
    "comment_count": 88
  })";

  auto parsed = ProxyTrafficBridge::parsePayload(payload);
  QVERIFY(parsed.has_value());
  QCOMPARE(parsed->title, QString("Comment Article"));
  QCOMPARE(parsed->commentNum, 88);
}

void RadarCoreTest::trafficBridgeIgnoresUnknownEndpoint() {
  const QByteArray payload = R"({
    "endpoint": "/cgi-bin/unknown",
    "read_num": 999999
  })";

  QVERIFY(!ProxyTrafficBridge::parsePayload(payload).has_value());
}

void RadarCoreTest::bridgePayloadClientSamplesParse() {
  auto metrics = ProxyTrafficBridge::parsePayload(BridgePayloadClient::sampleMetricsPayload());
  QVERIFY(metrics.has_value());
  QCOMPARE(metrics->url, QString("https://example.local/bridge-smoke"));
  QCOMPARE(metrics->readNum, 36000);
  QVERIFY(metrics->publishTime.isValid());
  auto comments = ProxyTrafficBridge::parsePayload(BridgePayloadClient::sampleCommentPayload());
  QVERIFY(comments.has_value());
  QCOMPARE(comments->commentNum, 96);
}


void RadarCoreTest::productionSuiteProxyReplayAndScoring() {
  ProductionSuiteController controller;
  const auto steps = controller.buildProxyWizard(8080, 9000, true, true, false);
  QCOMPARE(steps.size(), 5);
  QVERIFY(controller.proxyWizardReport(steps, true).contains(QStringLiteral("代理适配器")));
  QString error;
  const QByteArray sample = R"({"title":"Replay","account_name":"Lab","read_num":1000,"like_num":20,"comment_num":5,"url":"https://mp.weixin.qq.com/s/replay","timestamp":"2026-06-05T10:00:00Z"})";
  const auto records = controller.parseReplaySamples(QString::fromUtf8(sample), &error);
  QCOMPARE(records.size(), 1);
  QVERIFY(records.first().timestamp.isValid());
  QVERIFY(!records.first().publishTime.isValid());
  ProductionSuiteController::ScoreProfile profile;
  QVERIFY(controller.scoreRecord(records.first(), profile) > 1000.0);
  QVERIFY(controller.classifyFailure(QStringLiteral("phone unauthorized")) == QStringLiteral("PHONE_UNAUTHORIZED"));
  QVERIFY(controller.privacyBoundaryText(true).contains(QStringLiteral("登录凭证")));
}


void RadarCoreTest::productionSuiteQualityTrendAnalysisDelivery() {
  ProductionSuiteController controller;
  ContentRecord first;
  first.title = QStringLiteral("AI工具增长清单");
  first.accountName = QStringLiteral("Growth Lab");
  first.url = QStringLiteral("https://mp.weixin.qq.com/s/growth");
  first.readNum = 1000;
  first.likeNum = 80;
  first.commentNum = 40;
  first.timestamp = QDateTime::fromString("2026-06-05T08:00:00Z", Qt::ISODate);
  first.publishTime = first.timestamp;
  ContentRecord latest = first;
  latest.readNum = 16000;
  latest.likeNum = 900;
  latest.commentNum = 220;
  latest.timestamp = QDateTime::fromString("2026-06-05T12:00:00Z", Qt::ISODate);
  ContentRecord bad;
  bad.likeNum = 10;
  ContentRecord oldLikeBad;
  oldLikeBad.url = QStringLiteral("https://mp.weixin.qq.com/s/old-like-bad");
  oldLikeBad.oldLikeNum = 5;
  QVector<ContentRecord> records{first, latest, bad, oldLikeBad};

  const auto quality = controller.dataQualityItems(records);
  QVERIFY(!quality.isEmpty());
  QVERIFY(std::any_of(quality.cbegin(), quality.cend(), [](const auto& item) {
    return item.name == QStringLiteral("missing_urls") && item.count == 1;
  }));
  QVERIFY(std::any_of(quality.cbegin(), quality.cend(), [](const auto& item) {
    return item.name == QStringLiteral("abnormal_metrics") && item.count >= 2;
  }));
  QVERIFY(std::any_of(quality.cbegin(), quality.cend(), [](const auto& item) {
    return item.name == QStringLiteral("invalid_publish_time") && item.count >= 2;
  }));
  const auto trends = controller.trendPoints(records, true);
  QCOMPARE(trends.size(), 1);
  QVERIFY(trends.first().growth > 10000);
  QVERIFY(trends.first().recommendation.contains(QStringLiteral("优先")));
  const auto analysis = controller.analyzeContentSignals(records, true);
  QVERIFY(!analysis.isEmpty());
  QVERIFY(analysis.first().reason.contains(QStringLiteral("触发词")) || analysis.first().reason.contains(QStringLiteral("互动密度")));
  QVERIFY(controller.releaseReadinessText(true).contains(QStringLiteral("校验和")));
  ProductionSuiteController::ScoreProfile freshnessProfile;
  freshnessProfile.readWeight = 0.0;
  freshnessProfile.likeWeight = 0.0;
  freshnessProfile.commentWeight = 0.0;
  freshnessProfile.oldLikeWeight = 0.0;
  freshnessProfile.freshnessWeight = 1.0;
  freshnessProfile.originalityWeight = 10.0;
  ContentRecord scored;
  scored.category = QStringLiteral("原创");
  scored.publishTime = QDateTime::currentDateTimeUtc();
  QVERIFY(controller.scoreRecord(scored, freshnessProfile) >= 10.0);
}

void RadarCoreTest::quickStartUiTextKeys() {
  const QStringList keys = {
      QStringLiteral("tab.quick"), QStringLiteral("quick.title"), QStringLiteral("quick.intro"),
      QStringLiteral("quick.start"), QStringLiteral("quick.step.phone"), QStringLiteral("quick.step.search"),
      QStringLiteral("quick.step.queue"), QStringLiteral("quick.step.metrics"), QStringLiteral("quick.summary"),
      QStringLiteral("quick.metrics_waiting"), QStringLiteral("quick.no_keywords"), QStringLiteral("tip.quick.start"),
      QStringLiteral("quick.supplemental_intro"), QStringLiteral("quick.use_supplemental"),
      QStringLiteral("quick.advanced_phone_search"), QStringLiteral("quick.suggestion_add_links")};
  for (const QString& key : keys) {
    const QString en = UiText::text(key, UiLanguage::English);
    const QString zh = UiText::text(key, UiLanguage::Chinese);
    QVERIFY2(en != key, qPrintable(QStringLiteral("missing English key %1").arg(key)));
    QVERIFY2(zh != key, qPrintable(QStringLiteral("missing Chinese key %1").arg(key)));
    QVERIFY2(!en.isEmpty() && !zh.isEmpty(), qPrintable(QStringLiteral("empty quick key %1").arg(key)));
  }
}

void RadarCoreTest::weChatSearchAutomationPlan() {
  QCOMPARE(WeChatSearchAutomationController::escapeInputText(QStringLiteral("AI tools")), QStringLiteral("AI%stools"));
  QVERIFY(WeChatSearchAutomationController::hasChineseInputRisk(QStringLiteral("人工智能工具")));
  WeChatSearchAutomationController::Options disabled;
  auto disabledPlan = WeChatSearchAutomationController::dryRunPlan({QStringLiteral("AI")}, disabled);
  QVERIFY(!disabledPlan.success);
  QCOMPARE(disabledPlan.message, QStringLiteral("advanced_wechat_search_disabled"));
  WeChatSearchAutomationController::Options options;
  options.enabled = true;
  options.searchTapX = 120;
  options.searchTapY = 80;
  options.resultTapX = 300;
  options.resultTapY = 600;
  auto plan = WeChatSearchAutomationController::dryRunPlan({QStringLiteral("AI tools")}, options);
  QVERIFY(plan.success);
  QVERIFY(plan.commands.join(QStringLiteral("\n")).contains(QStringLiteral("com.tencent.mm")));
  QVERIFY(plan.commands.join(QStringLiteral("\n")).contains(QStringLiteral("input text AI%stools")));
}

void RadarCoreTest::autoIngestionQueueAndAdbArgs() {
  AutoIngestionController controller;
  QString error;
  QVERIFY(controller.enqueueUrl("https://mp.weixin.qq.com/s/test_article", "Account", "Tech", &error));
  QCOMPARE(controller.tasks().size(), 1);
  QCOMPARE(controller.pendingCount(), 1);
  QVERIFY(!controller.enqueueUrl("https://example.com/not-wechat", QString(), QString(), &error));
  const QStringList args = AutoIngestionController::adbOpenUrlArguments("https://mp.weixin.qq.com/s/test_article");
  QVERIFY(args.contains("android.intent.action.VIEW"));
  QVERIFY(args.contains("https://mp.weixin.qq.com/s/test_article"));
  QTemporaryDir dir;
  const QString path = dir.filePath("queue.json");
  QVERIFY(controller.saveQueue(path, &error));
  AutoIngestionController loaded;
  QVERIFY(loaded.loadQueue(path, &error));
  QCOMPARE(loaded.tasks().size(), 1);
}

QTEST_MAIN(RadarCoreTest)
#include "test_core.moc"

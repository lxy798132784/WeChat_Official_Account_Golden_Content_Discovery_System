#include <QStandardItemModel>
#include <QtTest/QtTest>

#include "BridgePayloadClient.h"
#include "DatabaseController.h"
#include "ExportController.h"
#include "PremiumContentFilterProxyModel.h"
#include "ProxyTrafficBridge.h"

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

  QVERIFY(db.enqueueArticle(record));
  QVERIFY(db.flush());
  QCOMPARE(db.articleCount(), 1);
  QCOMPARE(db.listArticles().first().title, QString("Golden Article"));
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
  QVector<ContentRecord> records{record};
  const QString csvPath = dir.filePath("articles.csv");
  const QString jsonPath = dir.filePath("articles.json");
  QVERIFY(ExportController::exportArticlesCsv(records, csvPath));
  QVERIFY(ExportController::exportArticlesJson(records, jsonPath));
  QFile csv(csvPath);
  QFile json(jsonPath);
  QVERIFY(csv.open(QIODevice::ReadOnly));
  QVERIFY(json.open(QIODevice::ReadOnly));
  QVERIFY(QString::fromUtf8(csv.readAll()).contains("Export Article"));
  QVERIFY(QString::fromUtf8(json.readAll()).contains("read_num"));
}

void RadarCoreTest::proxyScoresAndFilters() {
  QStandardItemModel model;
  model.setHorizontalHeaderLabels({"Title", "Account", "Category", "Read", "Like",
                                   "OldLike", "Comment", "Freq30d", "Score", "URL"});
  model.appendRow({new QStandardItem("A"), new QStandardItem("Acct"),
                   new QStandardItem("Tech"), new QStandardItem("10000"),
                   new QStandardItem("500"), new QStandardItem("100"),
                   new QStandardItem("50"), new QStandardItem("10"),
                   new QStandardItem(""), new QStandardItem("u")});

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
  auto comments = ProxyTrafficBridge::parsePayload(BridgePayloadClient::sampleCommentPayload());
  QVERIFY(comments.has_value());
  QCOMPARE(comments->commentNum, 96);
}

QTEST_MAIN(RadarCoreTest)
#include "test_core.moc"

#include <QStandardItemModel>
#include <QtTest/QtTest>

#include "DatabaseController.h"
#include "PremiumContentFilterProxyModel.h"

class RadarCoreTest : public QObject {
  Q_OBJECT

 private slots:
  void databaseStoresArticles();
  void proxyScoresAndFilters();
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

QTEST_MAIN(RadarCoreTest)
#include "test_core.moc"

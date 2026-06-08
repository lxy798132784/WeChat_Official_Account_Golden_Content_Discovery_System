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
  void sogouRedirectCandidatesAreSupported();
  void weChatSearchAutomationPlan();
  void weChatSearchAutomationStableArticleSelection();
  void weChatSearchAutomationAdaptiveLayout();
  void weChatSearchAutomationWebViewStageDetection();
  void weChatCollectionCriteriaFiltersAndSummarizes();
  void keywordTargetCollectionPlanIsUserConfigurable();
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

void RadarCoreTest::sogouRedirectCandidatesAreSupported() {
  const QByteArray html = R"(<ul class="news-list"><li id="sogou_vr_11002601_box_0"><div class="txt-box"><h3><a target="_blank" href="/link?url=encrypted-candidate&amp;type=2&amp;query=AI%E5%B7%A5%E5%85%B7&amp;token=abc">2026免费<em>AI工具</em>推荐</a></h3><span class="all-time-y2">测试账号</span></div></li></ul>)";
  const auto results = KeywordDiscoveryController::parseSearchHtml(QStringLiteral("AI工具"), html);
  QCOMPARE(results.size(), 1);
  QVERIFY(results.first().url.startsWith(QStringLiteral("https://weixin.sogou.com/link?")));
  QVERIFY(AutoIngestionController::isSupportedArticleUrl(results.first().url));
  AutoIngestionController controller;
  QString error;
  QVERIFY(controller.enqueueUrl(results.first().url, QStringLiteral("测试账号"), QString(), &error));
  QCOMPARE(controller.pendingCount(), 1);
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
  options.autoLocateSearch = false;
  options.searchTapX = 120;
  options.searchTapY = 80;
  options.resultTapX = 300;
  options.resultTapY = 600;
  auto plan = WeChatSearchAutomationController::dryRunPlan({QStringLiteral("AI tools")}, options);
  QVERIFY(plan.success);
  QVERIFY(plan.commands.join(QStringLiteral("\n")).contains(QStringLiteral("com.tencent.mm")));
  QVERIFY(plan.commands.join(QStringLiteral("\n")).contains(QStringLiteral("input text AI%stools")));
  int x = 0;
  int y = 0;
  QVERIFY(WeChatSearchAutomationController::findNodeCenterByText(
      QStringLiteral(R"(<hierarchy><node text="搜索" content-desc="" bounds="[800,100][1000,220]" /></hierarchy>)"),
      {QStringLiteral("搜索")}, &x, &y));
  QCOMPARE(x, 900);
  QCOMPARE(y, 160);
  QVERIFY(WeChatSearchAutomationController::findArticleEntryCenter(
      QStringLiteral(R"(<hierarchy><node text="聊天记录" class="android.widget.TextView" bounds="[40,300][500,380]" /><node text="AI工具爆款文章 公众号 阅读10万+" class="android.widget.TextView" clickable="true" bounds="[40,520][1030,700]" /></hierarchy>)"),
      &x, &y));
  QVERIFY(y > 520);
  QVERIFY(WeChatSearchAutomationController::uiDumpLooksLikeArticlePage(
      QStringLiteral(R"(<hierarchy><node text="公众号" bounds="[1,1][2,2]" /><node text="阅读 100000" bounds="[1,1][2,2]" /><node text="赞" bounds="[1,1][2,2]" /></hierarchy>)")));
    PhoneDiagnosticReport report;
    report.items.push_back({QStringLiteral("adb_tool"), {}, QStringLiteral("pass"), {}, {}, {}});
    report.items.push_back({QStringLiteral("adb_server"), {}, QStringLiteral("pass"), {}, {}, {}});
    report.items.push_back({QStringLiteral("device_detected"), {}, QStringLiteral("pass"), {}, {}, {}});
    report.items.push_back({QStringLiteral("usb_authorization"), {}, QStringLiteral("pass"), {}, {}, {}});
    report.items.push_back({QStringLiteral("shell_control"), {}, QStringLiteral("pass"), {}, {}, {}});
    QString reason;
    QVERIFY(!PhoneDiagnosticsController::isCoreReady(report, &reason));
    QVERIFY(reason.contains(QStringLiteral("screen_unlocked")) || reason.contains(QStringLiteral("Missing diagnostic item")));
    report.items.push_back({QStringLiteral("screen_unlocked"), {}, QStringLiteral("pass"), {}, {}, {}});
    report.targetSerial = QStringLiteral("offline-test-serial");
    QVERIFY(!PhoneDiagnosticsController::isCoreReady(report, &reason));
    QVERIFY(reason.contains(QStringLiteral("locked"), Qt::CaseInsensitive) || reason.contains(QStringLiteral("screen"), Qt::CaseInsensitive));
}

void RadarCoreTest::weChatSearchAutomationStableArticleSelection() {
  int x = 0;
  int y = 0;
  const QString tabXml = QStringLiteral(
      R"(<hierarchy><node text="全部" bounds="[210,280][310,360]" /><node text="账号" bounds="[360,280][460,360]" /><node text="问一问" bounds="[520,280][650,360]" /><node text="文章" bounds="[740,280][850,360]" /><node text="视频" bounds="[900,280][1010,360]" /></hierarchy>)");
  QVERIFY(WeChatSearchAutomationController::findArticlesTabCenter(tabXml, &x, &y));
  QCOMPARE(x, 795);
  QCOMPARE(y, 320);

  const QString resultsXml = QStringLiteral(R"(<hierarchy>
    <node text="广告 一键查询AI搜索排名" class="android.widget.TextView" clickable="true" bounds="[30,520][1150,700]" />
    <node text="元宝 小程序 腾讯混元Bot" class="android.widget.TextView" clickable="true" bounds="[30,720][1150,900]" />
    <node text="AI前线 公众号 2379篇原创文章" class="android.widget.TextView" clickable="true" bounds="[30,920][1150,1100]" />
    <node text="OpenAI曝光自进化AI 6周准确率翻三倍 Bug全自己修 新智元 19小时前 公众号 阅读 181人" class="android.widget.TextView" clickable="true" bounds="[30,1120][1150,1380]" />
  </hierarchy>)");
  QVERIFY(WeChatSearchAutomationController::findOfficialAccountArticleResultCenter(resultsXml, &x, &y));
  QVERIFY(y > 1120);
  QVERIFY(y < 1380);

  const QString articleXml = QStringLiteral(R"(<hierarchy>
    <node text="OpenAI曝光「自进化」AI！6周准确率翻三倍，Bug全自己修" bounds="[20,200][1100,420]" />
    <node text="新智元" bounds="[20,430][200,500]" />
    <node text="181人" bounds="[230,430][330,500]" />
    <node text="赞 203" bounds="[500,2500][620,2600]" />
    <node text="转发 1158" bounds="[650,2500][820,2600]" />
    <node text="收藏 87" bounds="[850,2500][980,2600]" />
    <node text="评论 9" bounds="[1000,2500][1150,2600]" />
  </hierarchy>)");
  QVERIFY(WeChatSearchAutomationController::uiDumpLooksLikeMetricsVisible(articleXml));
}

void RadarCoreTest::weChatSearchAutomationAdaptiveLayout() {
  const auto tall = WeChatSearchAutomationController::adaptivePoints(1264, 2780);
  QCOMPARE(tall.searchX, 1045);
  QCOMPARE(tall.searchY, 209);
  QCOMPARE(tall.networkResultX, 506);
  QCOMPARE(tall.networkResultY, 1696);
  QCOMPARE(tall.articlesTabX, 809);
  QCOMPARE(tall.articlesTabY, 320);
  QCOMPARE(tall.firstArticleX, 594);
  QCOMPARE(tall.firstArticleY, 1960);

  const auto common = WeChatSearchAutomationController::adaptivePoints(1080, 2400);
  QCOMPARE(common.searchX, 893);
  QCOMPARE(common.searchY, 180);
  QCOMPARE(common.networkResultX, 432);
  QCOMPARE(common.articlesTabX, 691);

  int x = 0;
  int y = 0;
  QVERIFY(WeChatSearchAutomationController::findNodeCenterByText(
      QStringLiteral(R"(<hierarchy><node text="AI搜索" bounds="[120,90][300,180]" /></hierarchy>)"),
      WeChatSearchAutomationController::networkSearchTextHints(), &x, &y));
  QCOMPARE(x, 210);
  QVERIFY(WeChatSearchAutomationController::findArticlesTabCenter(
      QStringLiteral(R"(<hierarchy><node text="公众号文章" bounds="[600,260][820,360]" /></hierarchy>)"), &x, &y));
  QCOMPARE(x, 710);

  const QString emptyXml = QStringLiteral(R"(<hierarchy rotation="0"><node bounds="[0,0][0,0]" /></hierarchy>)");
  QVERIFY(!WeChatSearchAutomationController::findArticlesTabCenter(emptyXml, &x, &y));
}

void RadarCoreTest::weChatSearchAutomationWebViewStageDetection() {
  QVERIFY(WeChatSearchAutomationController::windowFocusLooksLikeArticleContainer(
      QStringLiteral("mCurrentFocus=Window{d8e7b67 u0 com.tencent.mm/com.tencent.mm.plugin.webview.ui.tools.MMWebViewUI}")));
  QVERIFY(WeChatSearchAutomationController::windowFocusLooksLikeArticleContainer(
      QStringLiteral("mCurrentFocus=Window{d8e7b67 u0 com.tencent.mm/com.tencent.mm.plugin.brandservice.ui.timeline.preload.ui.TmplWebViewMMUI}")));
  QVERIFY(WeChatSearchAutomationController::windowFocusLooksLikeArticleContainer(
      QStringLiteral("mCurrentFocus=Window{b9560eb u0 com.tencent.mm/com.tencent.mm.plugin.lite.ui.WxaLiteAppLiteUI}")));
  QVERIFY(!WeChatSearchAutomationController::windowFocusLooksLikeArticleContainer(
      QStringLiteral("mCurrentFocus=Window{63f41e0 u0 com.tencent.mm/com.tencent.mm.ui.LauncherUI}")));
  QVERIFY(WeChatSearchAutomationController::windowFocusLooksLikeRejectedContent(
      QStringLiteral("mCurrentFocus=Window{d7e0886 u0 com.tencent.mm/com.tencent.mm.plugin.finder.ui.FinderShareFeedRelUI}")));
  QVERIFY(!WeChatSearchAutomationController::windowFocusLooksLikeArticleContainer(
      QStringLiteral("mCurrentFocus=Window{d7e0886 u0 com.tencent.mm/com.tencent.mm.plugin.finder.ui.FinderShareFeedRelUI}")));
}

void RadarCoreTest::weChatCollectionCriteriaFiltersAndSummarizes() {
  WeChatSearchAutomationController::CollectionCriteria criteria;
  criteria.maxArticles = 2;
  criteria.minRead = 10000;
  criteria.minLike = 100;
  criteria.minOldLike = 20;
  criteria.minComment = 3;

  WeChatSearchAutomationController::CandidateMetrics good;
  good.url = QStringLiteral("https://mp.weixin.qq.com/s/good");
  good.read = 12000;
  good.like = 180;
  good.oldLike = 30;
  good.comment = 8;
  auto decision = WeChatSearchAutomationController::evaluateCandidate(good, criteria);
  QVERIFY(decision.accepted);
  QCOMPARE(decision.reason, QStringLiteral("accepted"));

  WeChatSearchAutomationController::CandidateMetrics lowRead = good;
  lowRead.url = QStringLiteral("https://mp.weixin.qq.com/s/low-read");
  lowRead.read = 9999;
  decision = WeChatSearchAutomationController::evaluateCandidate(lowRead, criteria);
  QVERIFY(!decision.accepted);
  QCOMPARE(decision.reason, QStringLiteral("min_read"));

  QSet<QString> seen;
  seen.insert(good.url);
  decision = WeChatSearchAutomationController::evaluateCandidate(good, criteria, seen);
  QVERIFY(!decision.accepted);
  QCOMPARE(decision.reason, QStringLiteral("duplicate_url"));

  WeChatSearchAutomationController::CandidateMetrics lowLike = good;
  lowLike.url = QStringLiteral("https://mp.weixin.qq.com/s/low-like");
  lowLike.like = 99;
  WeChatSearchAutomationController::CandidateMetrics secondGood = good;
  secondGood.url = QStringLiteral("https://mp.weixin.qq.com/s/good-2");
  WeChatSearchAutomationController::CandidateMetrics thirdGood = good;
  thirdGood.url = QStringLiteral("https://mp.weixin.qq.com/s/good-3");

  const auto summary = WeChatSearchAutomationController::summarizeCollection(
      QVector<WeChatSearchAutomationController::CandidateMetrics>{good, lowRead, lowLike, secondGood, thirdGood}, criteria);
  QCOMPARE(summary.attempted, 4);
  QCOMPARE(summary.opened, 4);
  QCOMPARE(summary.captured, 4);
  QCOMPARE(summary.accepted, 2);
  QCOMPARE(summary.rejectedByThreshold, 2);
  QCOMPARE(summary.rejectedAsDuplicate, 0);
  QCOMPARE(summary.failed, 0);
}

void RadarCoreTest::keywordTargetCollectionPlanIsUserConfigurable() {
  const QDate today(2026, 6, 8);
  const auto plan = KeywordDiscoveryController::buildRecentMonthEmotionCollectionPlan(
      today, QStringLiteral("情感, 婚姻\n恋爱｜亲密关系"), 30000, 50000, 20, 7, 120);
  QCOMPARE(plan.keywords, QStringList({QStringLiteral("情感"), QStringLiteral("婚姻"), QStringLiteral("恋爱"), QStringLiteral("亲密关系")}));
  QCOMPARE(plan.startDate, QDate(2026, 5, 8));
  QCOMPARE(plan.endDate, today);
  QCOMPARE(plan.minRead, 30000);
  QCOMPARE(plan.maxRead, 50000);
  QCOMPARE(plan.targetCount, 20);
  QCOMPARE(plan.maxCandidatesPerKeyword, 7);
  QCOMPARE(plan.maxScanCount, 120);

  QVector<KeywordDiscoveryResult> candidates;
  KeywordDiscoveryResult good;
  good.keyword = QStringLiteral("情感");
  good.url = QStringLiteral("https://mp.weixin.qq.com/s/good");
  good.readNum = 35000;
  good.publishDate = QDate(2026, 5, 20);
  candidates.push_back(good);
  KeywordDiscoveryResult tooLow = good;
  tooLow.url = QStringLiteral("https://mp.weixin.qq.com/s/low");
  tooLow.readNum = 29999;
  candidates.push_back(tooLow);
  KeywordDiscoveryResult tooHigh = good;
  tooHigh.url = QStringLiteral("https://mp.weixin.qq.com/s/high");
  tooHigh.readNum = 50001;
  candidates.push_back(tooHigh);
  KeywordDiscoveryResult tooOld = good;
  tooOld.url = QStringLiteral("https://mp.weixin.qq.com/s/old");
  tooOld.publishDate = QDate(2026, 5, 1);
  candidates.push_back(tooOld);
  candidates.push_back(good);

  const auto filtered = KeywordDiscoveryController::filterTargetCollectionResults(candidates, plan);
  QCOMPARE(filtered.size(), 1);
  QCOMPARE(filtered.first().url, QStringLiteral("https://mp.weixin.qq.com/s/good"));
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

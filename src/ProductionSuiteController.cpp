#include "ProductionSuiteController.h"

#include <algorithm>

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QTextStream>

namespace {
QString statusFromBool(bool ok, const QString& pass = QStringLiteral("pass"),
                      const QString& fail = QStringLiteral("fail")) {
  return ok ? pass : fail;
}

QString jsonString(const QJsonObject& object, const QStringList& keys) {
  for (const QString& key : keys) {
    const QJsonValue value = object.value(key);
    if (value.isString()) return value.toString().trimmed();
  }
  return QString();
}

int jsonInt(const QJsonObject& object, const QStringList& keys) {
  for (const QString& key : keys) {
    const QJsonValue value = object.value(key);
    if (value.isDouble()) return value.toInt();
    if (value.isString()) return value.toString().toInt();
  }
  return 0;
}

ContentRecord recordFromJson(const QJsonObject& object) {
  ContentRecord record;
  record.title = jsonString(object, {QStringLiteral("title"), QStringLiteral("article_title")});
  record.accountName = jsonString(object, {QStringLiteral("account_name"), QStringLiteral("account"), QStringLiteral("publisher")});
  record.category = jsonString(object, {QStringLiteral("category"), QStringLiteral("industry"), QStringLiteral("tag")});
  record.url = jsonString(object, {QStringLiteral("url"), QStringLiteral("article_url"), QStringLiteral("link")});
  record.readNum = jsonInt(object, {QStringLiteral("read_num"), QStringLiteral("read"), QStringLiteral("reads")});
  record.likeNum = jsonInt(object, {QStringLiteral("like_num"), QStringLiteral("like"), QStringLiteral("likes")});
  record.oldLikeNum = jsonInt(object, {QStringLiteral("old_like_num"), QStringLiteral("old_like")});
  record.commentNum = jsonInt(object, {QStringLiteral("comment_num"), QStringLiteral("comment"), QStringLiteral("comments")});
  record.publishTime = QDateTime::fromString(jsonString(object, {QStringLiteral("publish_time"), QStringLiteral("datetime")}), Qt::ISODate);
  if (!record.publishTime.isValid()) record.publishTime = QDateTime::currentDateTimeUtc();
  return record;
}
}  // namespace

ProductionSuiteController::ProductionSuiteController(QObject* parent) : QObject(parent) {}

QVector<ProductionSuiteController::ProxyStep> ProductionSuiteController::buildProxyWizard(
    int proxyPort, int bridgePort, bool phoneReachable, bool metricHit, bool commentHit) const {
  QVector<ProxyStep> steps;
  steps.push_back({QStringLiteral("proxy_port"), statusFromBool(proxyPort > 0, QStringLiteral("pass"), QStringLiteral("warn")),
                   proxyPort > 0 ? QStringLiteral("127.0.0.1:%1").arg(proxyPort) : QStringLiteral("Proxy port is not configured"),
                   QStringLiteral("Set the local proxy listening port. Use 0 only when you intentionally skip proxy checks.")});
  steps.push_back({QStringLiteral("bridge_port"), statusFromBool(bridgePort > 0),
                   bridgePort > 0 ? QStringLiteral("Local bridge port: %1").arg(bridgePort) : QStringLiteral("Bridge port is invalid"),
                   QStringLiteral("Open WeChat Integration and set a valid local bridge port.")});
  steps.push_back({QStringLiteral("phone_to_pc"), statusFromBool(phoneReachable, QStringLiteral("pass"), QStringLiteral("warn")),
                   phoneReachable ? QStringLiteral("Phone can reach this computer") : QStringLiteral("Phone reachability is not verified"),
                   QStringLiteral("Configure the phone Wi-Fi proxy and open the local ping/check URL from the phone.")});
  steps.push_back({QStringLiteral("metrics_hit"), statusFromBool(metricHit, QStringLiteral("pass"), QStringLiteral("warn")),
                   metricHit ? QStringLiteral("/mp/getappmsgext has been observed") : QStringLiteral("No metric interface hit yet"),
                   QStringLiteral("Open a WeChat article on the test phone and check whether the proxy adapter forwards compact JSON.")});
  steps.push_back({QStringLiteral("comments_hit"), statusFromBool(commentHit, QStringLiteral("pass"), QStringLiteral("warn")),
                   commentHit ? QStringLiteral("/mp/appmsg_comment has been observed") : QStringLiteral("No comment interface hit yet"),
                   QStringLiteral("Open the comment area if comment density is required for scoring.")});
  return steps;
}

QString ProductionSuiteController::proxyWizardReport(const QVector<ProxyStep>& steps, bool chinese) const {
  QString out;
  QTextStream stream(&out);
  stream << (chinese ? QStringLiteral("代理适配器向导报告\n") : QStringLiteral("Proxy Adapter Wizard Report\n"));
  for (const ProxyStep& step : steps) {
    if (chinese) {
      QString status = step.status;
      if (status == QStringLiteral("pass")) status = QStringLiteral("通过");
      else if (status == QStringLiteral("warn")) status = QStringLiteral("提醒");
      else if (status == QStringLiteral("fail")) status = QStringLiteral("失败");
      QString name = step.name;
      if (name == QStringLiteral("proxy_port")) name = QStringLiteral("本地代理端口");
      else if (name == QStringLiteral("bridge_port")) name = QStringLiteral("本地桥端口");
      else if (name == QStringLiteral("phone_to_pc")) name = QStringLiteral("手机访问电脑");
      else if (name == QStringLiteral("metrics_hit")) name = QStringLiteral("指标接口命中");
      else if (name == QStringLiteral("comments_hit")) name = QStringLiteral("评论接口命中");
      QString detail = step.detail;
      QString fix = step.fixHint;
      detail.replace(QStringLiteral("Proxy port is not configured"), QStringLiteral("未配置代理端口"));
      detail.replace(QStringLiteral("Local bridge port:"), QStringLiteral("本地桥端口："));
      detail.replace(QStringLiteral("Bridge port is invalid"), QStringLiteral("本地桥端口无效"));
      detail.replace(QStringLiteral("Phone can reach this computer"), QStringLiteral("手机可以访问这台电脑"));
      detail.replace(QStringLiteral("Phone reachability is not verified"), QStringLiteral("尚未验证手机能否访问这台电脑"));
      detail.replace(QStringLiteral("/mp/getappmsgext has been observed"), QStringLiteral("已观察到文章指标接口"));
      detail.replace(QStringLiteral("No metric interface hit yet"), QStringLiteral("尚未命中文章指标接口"));
      detail.replace(QStringLiteral("/mp/appmsg_comment has been observed"), QStringLiteral("已观察到评论接口"));
      detail.replace(QStringLiteral("No comment interface hit yet"), QStringLiteral("尚未命中评论接口"));
      fix.replace(QStringLiteral("Set the local proxy listening port. Use 0 only when you intentionally skip proxy checks."), QStringLiteral("设置本地代理监听端口。只有明确跳过代理检测时才使用 0。"));
      fix.replace(QStringLiteral("Open WeChat Integration and set a valid local bridge port."), QStringLiteral("打开微信接入页，设置有效的本地桥端口。"));
      fix.replace(QStringLiteral("Configure the phone Wi-Fi proxy and open the local ping/check URL from the phone."), QStringLiteral("配置手机无线网络代理，并从手机打开本地检测地址。"));
      fix.replace(QStringLiteral("Open a WeChat article on the test phone and check whether the proxy adapter forwards compact JSON."), QStringLiteral("在测试手机打开一篇微信文章，确认代理适配器会转发精简数据。"));
      fix.replace(QStringLiteral("Open the comment area if comment density is required for scoring."), QStringLiteral("如果评分需要评论密度，请打开文章评论区。"));
      stream << QStringLiteral("- [%1] %2：%3\n  %4\n").arg(status, name, detail, fix);
    } else {
      stream << QStringLiteral("- [%1] %2: %3\n  %4\n").arg(step.status, step.name, step.detail, step.fixHint);
    }
  }
  return out;
}

QVector<ContentRecord> ProductionSuiteController::parseReplaySamples(const QString& text, QString* errorMessage) const {
  QVector<ContentRecord> records;
  const QJsonDocument document = QJsonDocument::fromJson(text.toUtf8());
  if (document.isArray()) {
    for (const QJsonValue& value : document.array()) {
      if (value.isObject()) records.push_back(recordFromJson(value.toObject()));
    }
    return records;
  }
  if (document.isObject()) {
    records.push_back(recordFromJson(document.object()));
    return records;
  }
  const QStringList lines = text.split('\n', Qt::SkipEmptyParts);
  for (const QString& line : lines) {
    const QJsonDocument lineDoc = QJsonDocument::fromJson(line.trimmed().toUtf8());
    if (lineDoc.isObject()) records.push_back(recordFromJson(lineDoc.object()));
  }
  if (records.isEmpty() && errorMessage != nullptr) {
    *errorMessage = QStringLiteral("Replay input must be JSON, JSON array, or JSONL with sanitized article records.");
  }
  return records;
}

QVector<ProductionSuiteController::HealthItem> ProductionSuiteController::buildHealthItems(
    int pendingTasks, int failedTasks, bool phoneReady, bool proxyReady, bool bridgeReady, bool databaseReady) const {
  return {{QStringLiteral("phone"), phoneReady ? QStringLiteral("healthy") : QStringLiteral("blocked"), phoneReady ? QStringLiteral("Phone preflight passed") : QStringLiteral("Phone preflight is not ready")},
          {QStringLiteral("proxy"), proxyReady ? QStringLiteral("healthy") : QStringLiteral("warning"), proxyReady ? QStringLiteral("Proxy adapter is reachable") : QStringLiteral("Proxy adapter is not fully verified")},
          {QStringLiteral("bridge"), bridgeReady ? QStringLiteral("healthy") : QStringLiteral("blocked"), bridgeReady ? QStringLiteral("Local bridge is reachable") : QStringLiteral("Local bridge is not reachable")},
          {QStringLiteral("database"), databaseReady ? QStringLiteral("healthy") : QStringLiteral("blocked"), databaseReady ? QStringLiteral("Database is writable") : QStringLiteral("Database is not writable")},
          {QStringLiteral("queue"), failedTasks > 0 ? QStringLiteral("warning") : QStringLiteral("healthy"), QStringLiteral("%1 pending / %2 failed").arg(pendingTasks).arg(failedTasks)}};
}

QString ProductionSuiteController::classifyFailure(const QString& rawError) const {
  const QString lower = rawError.toLower();
  if (lower.contains(QStringLiteral("adb")) && lower.contains(QStringLiteral("not"))) return QStringLiteral("ADB_TOOL_MISSING");
  if (lower.contains(QStringLiteral("unauthorized"))) return QStringLiteral("PHONE_UNAUTHORIZED");
  if (lower.contains(QStringLiteral("offline"))) return QStringLiteral("PHONE_OFFLINE");
  if (lower.contains(QStringLiteral("more than one"))) return QStringLiteral("MULTIPLE_DEVICES");
  if (lower.contains(QStringLiteral("proxy"))) return QStringLiteral("PROXY_NOT_READY");
  if (lower.contains(QStringLiteral("bridge"))) return QStringLiteral("BRIDGE_NOT_READY");
  if (lower.contains(QStringLiteral("database")) || lower.contains(QStringLiteral("sqlite"))) return QStringLiteral("DB_WRITE_FAILED");
  if (lower.contains(QStringLiteral("metric")) || lower.contains(QStringLiteral("getappmsgext"))) return QStringLiteral("METRICS_MISSING");
  return QStringLiteral("UNKNOWN_FAILURE");
}


QVector<ProductionSuiteController::QualityItem> ProductionSuiteController::dataQualityItems(const QVector<ContentRecord>& records) const {
  QHash<QString, int> byUrl;
  int missingTitle = 0;
  int missingAccount = 0;
  int missingUrl = 0;
  int abnormalMetric = 0;
  int stalePublishTime = 0;
  for (const ContentRecord& record : records) {
    const QString url = record.url.trimmed();
    if (!url.isEmpty()) byUrl[url]++;
    if (record.title.trimmed().isEmpty()) ++missingTitle;
    if (record.accountName.trimmed().isEmpty()) ++missingAccount;
    if (url.isEmpty()) ++missingUrl;
    if ((record.readNum <= 0 && (record.likeNum > 0 || record.commentNum > 0)) ||
        record.likeNum > record.readNum || record.commentNum > record.readNum) {
      ++abnormalMetric;
    }
    if (!record.publishTime.isValid()) ++stalePublishTime;
  }
  int duplicateUrls = 0;
  for (auto it = byUrl.cbegin(); it != byUrl.cend(); ++it) {
    if (it.value() > 1) duplicateUrls += it.value() - 1;
  }
  auto statusFor = [](int count, const QString& warn = QStringLiteral("warning")) {
    return count == 0 ? QStringLiteral("healthy") : warn;
  };
  return {{QStringLiteral("duplicate_urls"), statusFor(duplicateUrls), QStringLiteral("%1 duplicate URL row(s)").arg(duplicateUrls), duplicateUrls},
          {QStringLiteral("missing_titles"), statusFor(missingTitle), QStringLiteral("%1 record(s) without title").arg(missingTitle), missingTitle},
          {QStringLiteral("missing_accounts"), statusFor(missingAccount), QStringLiteral("%1 record(s) without account").arg(missingAccount), missingAccount},
          {QStringLiteral("missing_urls"), statusFor(missingUrl, QStringLiteral("blocked")), QStringLiteral("%1 record(s) without URL").arg(missingUrl), missingUrl},
          {QStringLiteral("abnormal_metrics"), statusFor(abnormalMetric), QStringLiteral("%1 record(s) with suspicious metric ratios").arg(abnormalMetric), abnormalMetric},
          {QStringLiteral("invalid_publish_time"), statusFor(stalePublishTime), QStringLiteral("%1 record(s) without valid publish time").arg(stalePublishTime), stalePublishTime}};
}

QVector<ProductionSuiteController::TrendPoint> ProductionSuiteController::trendPoints(const QVector<ContentRecord>& records, bool chinese) const {
  QHash<QString, QVector<ContentRecord>> byUrl;
  for (const ContentRecord& record : records) {
    if (!record.url.trimmed().isEmpty()) byUrl[record.url.trimmed()].push_back(record);
  }
  QVector<TrendPoint> out;
  for (auto it = byUrl.begin(); it != byUrl.end(); ++it) {
    auto rows = it.value();
    std::sort(rows.begin(), rows.end(), [](const ContentRecord& a, const ContentRecord& b) { return a.timestamp < b.timestamp; });
    if (rows.size() < 2) continue;
    const ContentRecord& first = rows.first();
    const ContentRecord& latest = rows.last();
    TrendPoint point;
    point.title = latest.title.isEmpty() ? first.title : latest.title;
    point.account = latest.accountName.isEmpty() ? first.accountName : latest.accountName;
    point.firstReads = first.readNum;
    point.latestReads = latest.readNum;
    point.growth = latest.readNum - first.readNum;
    point.growthRate = first.readNum > 0 ? static_cast<double>(point.growth) / first.readNum : 0.0;
    if (point.growthRate >= 1.0 || point.growth >= 10000) {
      point.recommendation = chinese ? QStringLiteral("高速增长，优先拆解") : QStringLiteral("Fast growth; review first");
    } else if (point.growth > 0) {
      point.recommendation = chinese ? QStringLiteral("稳定增长，继续观察") : QStringLiteral("Steady growth; keep watching");
    } else {
      point.recommendation = chinese ? QStringLiteral("暂无增长，降低优先级") : QStringLiteral("No growth; lower priority");
    }
    out.push_back(point);
  }
  std::sort(out.begin(), out.end(), [](const TrendPoint& a, const TrendPoint& b) { return a.growth > b.growth; });
  return out;
}

QVector<ProductionSuiteController::AnalysisItem> ProductionSuiteController::analyzeContentSignals(const QVector<ContentRecord>& records, bool chinese) const {
  QVector<AnalysisItem> out;
  const QStringList tensionWords = {QStringLiteral("爆"), QStringLiteral("突然"), QStringLiteral("真相"), QStringLiteral("避坑"), QStringLiteral("增长"), QStringLiteral("赚钱"), QStringLiteral("必看"), QStringLiteral("秘密"), QStringLiteral("指南"), QStringLiteral("清单")};
  for (const ContentRecord& record : records) {
    AnalysisItem item;
    item.title = record.title;
    item.titleLength = record.title.size();
    for (const QString& word : tensionWords) {
      if (record.title.contains(word, Qt::CaseInsensitive)) item.tensionScore += 10;
    }
    item.engagementDensity = record.readNum > 0 ? static_cast<double>(record.likeNum + record.commentNum + record.oldLikeNum) / record.readNum : 0.0;
    QStringList reasons;
    if (item.titleLength >= 12 && item.titleLength <= 32) reasons << (chinese ? QStringLiteral("标题长度适中") : QStringLiteral("balanced title length"));
    if (item.tensionScore > 0) reasons << (chinese ? QStringLiteral("包含冲突/收益/清单类触发词") : QStringLiteral("contains tension/value trigger words"));
    if (item.engagementDensity >= 0.03) reasons << (chinese ? QStringLiteral("互动密度高") : QStringLiteral("high engagement density"));
    if (reasons.isEmpty()) reasons << (chinese ? QStringLiteral("规则信号较弱，建议人工复核") : QStringLiteral("weak rule signals; review manually"));
    item.reason = reasons.join(chinese ? QStringLiteral("，") : QStringLiteral(", "));
    out.push_back(item);
  }
  std::sort(out.begin(), out.end(), [](const AnalysisItem& a, const AnalysisItem& b) {
    if (a.tensionScore != b.tensionScore) return a.tensionScore > b.tensionScore;
    return a.engagementDensity > b.engagementDensity;
  });
  return out;
}

QString ProductionSuiteController::releaseReadinessText(bool chinese) const {
  if (chinese) {
    return QStringLiteral("交付检查清单：\n1. 下载对应系统安装包并核对校验和文件。\n2. 首次启动先进入微信接入页，确认数据库、插件目录和本地桥端口。\n3. 运行本地桥测试载荷，再运行手机诊断和代理向导。\n4. 只导入脱敏数据文件或表格文件，不保存登录凭证、请求头、访问令牌、证书或原始抓包。\n5. 生产前生成诊断快照和内容发现报告，留档用于复盘。\n6. 微软系统用户优先使用对应压缩包；桌面系统按处理器架构选择对应压缩包。");
  }
  return QStringLiteral("Delivery checklist:\n1. Download the package for the target OS and verify SHA256SUMS.\n2. On first launch, open WeChat Integration and confirm the database, plugin directory, and local bridge port.\n3. Run the bridge smoke payload, then phone diagnostics and the proxy wizard.\n4. Import sanitized JSON/CSV only; never store cookies, headers, tokens, certificates, or raw captures.\n5. Generate a diagnostic snapshot and content discovery report before production decisions.\n6. Windows users should use windows-x64.zip; Linux desktop users should choose amd64 or arm64 tar.gz.");
}

double ProductionSuiteController::scoreRecord(const ContentRecord& record, const ScoreProfile& profile) const {
  const double originality = 0.0;
  return record.readNum * profile.readWeight + record.likeNum * profile.likeWeight +
         record.commentNum * profile.commentWeight + record.oldLikeNum * profile.oldLikeWeight +
         originality * profile.originalityWeight;
}

QStringList ProductionSuiteController::accountInsights(const QVector<ContentRecord>& records, bool chinese) const {
  QHash<QString, QVector<ContentRecord>> byAccount;
  for (const ContentRecord& record : records) byAccount[record.accountName.isEmpty() ? chinese ? QStringLiteral("未知账号") : QStringLiteral("Unknown") : record.accountName].push_back(record);
  QStringList lines;
  for (auto it = byAccount.cbegin(); it != byAccount.cend(); ++it) {
    int reads = 0;
    int comments = 0;
    for (const ContentRecord& r : it.value()) { reads += r.readNum; comments += r.commentNum; }
    const int n = std::max(1, static_cast<int>(it.value().size()));
    lines.push_back(chinese ? QStringLiteral("%1：%2 篇，平均阅读 %3，平均评论 %4").arg(it.key()).arg(n).arg(reads / n).arg(comments / n)
                            : QStringLiteral("%1: %2 article(s), avg reads %3, avg comments %4").arg(it.key()).arg(n).arg(reads / n).arg(comments / n));
  }
  return lines;
}

QStringList ProductionSuiteController::keywordInsights(const QVector<ContentRecord>& records, bool chinese) const {
  QHash<QString, int> byCategory;
  for (const ContentRecord& record : records) byCategory[record.category.isEmpty() ? chinese ? QStringLiteral("未分类") : QStringLiteral("Uncategorized") : record.category]++;
  QStringList lines;
  for (auto it = byCategory.cbegin(); it != byCategory.cend(); ++it) {
    lines.push_back(chinese ? QStringLiteral("%1：%2 条候选/采集记录").arg(it.key()).arg(it.value())
                            : QStringLiteral("%1: %2 candidate/ingested record(s)").arg(it.key()).arg(it.value()));
  }
  return lines;
}

QString ProductionSuiteController::generateMarkdownReport(const QVector<ContentRecord>& records, const QString& workspace,
                                                          const ScoreProfile& profile, bool chinese) const {
  QString out;
  QTextStream stream(&out);
  stream << (chinese ? QStringLiteral("# 内容发现报告\n\n") : QStringLiteral("# Content Discovery Report\n\n"));
  stream << (chinese ? QStringLiteral("工作区：") : QStringLiteral("Workspace: ")) << workspace << QStringLiteral("\n\n");
  stream << (chinese ? QStringLiteral("记录数：") : QStringLiteral("Records: ")) << records.size() << QStringLiteral("\n\n");
  stream << (chinese ? QStringLiteral("评分 = 阅读*%1 + 点赞*%2 + 评论*%3 + 在看*%4 + 原创*%5\n\n")
                     : QStringLiteral("Score = read*%1 + like*%2 + comment*%3 + old_like*%4 + original*%5\n\n"))
                .arg(profile.readWeight).arg(profile.likeWeight).arg(profile.commentWeight).arg(profile.oldLikeWeight).arg(profile.originalityWeight);
  for (const QString& line : accountInsights(records, chinese)) stream << QStringLiteral("- ") << line << '\n';
  return out;
}

QString ProductionSuiteController::privacyBoundaryText(bool chinese) const {
  if (chinese) {
    return QStringLiteral("隐私边界：软件不保存登录凭证、请求头、访问令牌、证书或原始抓包；手机调试只用于打开用户队列里的文章链接；本地桥只接收脱敏数据；导出报告不应包含账号密码或私密凭证。");
  }
  return QStringLiteral("Privacy boundary: the app does not store cookies, headers, tokens, certificates, or raw packet captures. ADB only opens article URLs in the user queue. The local bridge accepts sanitized JSON only. Reports must not contain credentials.");
}

QJsonObject ProductionSuiteController::diagnosticSnapshot(const QVector<ProxyStep>& proxySteps,
                                                          const QVector<HealthItem>& healthItems,
                                                          const ScoreProfile& profile) const {
  QJsonArray proxy;
  for (const ProxyStep& step : proxySteps) proxy.push_back(QJsonObject{{QStringLiteral("name"), step.name}, {QStringLiteral("status"), step.status}, {QStringLiteral("detail"), step.detail}, {QStringLiteral("fix_hint"), step.fixHint}});
  QJsonArray health;
  for (const HealthItem& item : healthItems) health.push_back(QJsonObject{{QStringLiteral("name"), item.name}, {QStringLiteral("status"), item.status}, {QStringLiteral("detail"), item.detail}});
  return {{QStringLiteral("generated_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
          {QStringLiteral("proxy_steps"), proxy},
          {QStringLiteral("health_items"), health},
          {QStringLiteral("score_profile"), QJsonObject{{QStringLiteral("read_weight"), profile.readWeight}, {QStringLiteral("like_weight"), profile.likeWeight}, {QStringLiteral("comment_weight"), profile.commentWeight}, {QStringLiteral("old_like_weight"), profile.oldLikeWeight}, {QStringLiteral("freshness_weight"), profile.freshnessWeight}, {QStringLiteral("originality_weight"), profile.originalityWeight}}}};
}

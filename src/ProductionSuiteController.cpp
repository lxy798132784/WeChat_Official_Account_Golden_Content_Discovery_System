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
    stream << QStringLiteral("- [%1] %2: %3\n  %4\n").arg(step.status, step.name, step.detail, step.fixHint);
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

double ProductionSuiteController::scoreRecord(const ContentRecord& record, const ScoreProfile& profile) const {
  const double originality = 0.0;
  return record.readNum * profile.readWeight + record.likeNum * profile.likeWeight +
         record.commentNum * profile.commentWeight + record.oldLikeNum * profile.oldLikeWeight +
         originality * profile.originalityWeight;
}

QStringList ProductionSuiteController::accountInsights(const QVector<ContentRecord>& records, bool chinese) const {
  QHash<QString, QVector<ContentRecord>> byAccount;
  for (const ContentRecord& record : records) byAccount[record.accountName.isEmpty() ? QStringLiteral("Unknown") : record.accountName].push_back(record);
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
  for (const ContentRecord& record : records) byCategory[record.category.isEmpty() ? QStringLiteral("Uncategorized") : record.category]++;
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
  stream << QStringLiteral("Score = read*%1 + like*%2 + comment*%3 + old_like*%4 + original*%5\n\n")
                .arg(profile.readWeight).arg(profile.likeWeight).arg(profile.commentWeight).arg(profile.oldLikeWeight).arg(profile.originalityWeight);
  for (const QString& line : accountInsights(records, chinese)) stream << QStringLiteral("- ") << line << '\n';
  return out;
}

QString ProductionSuiteController::privacyBoundaryText(bool chinese) const {
  if (chinese) {
    return QStringLiteral("隐私边界：软件不保存 Cookie、Header、Token、证书或原始抓包；ADB 只用于打开用户队列里的文章链接；本地桥只接收脱敏 JSON；导出报告不应包含账号密码或私密凭证。");
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

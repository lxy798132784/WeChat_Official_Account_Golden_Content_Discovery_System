#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

#include "content_record.h"

/**
 * @brief End-to-end production helper controller.
 * @brief 生产落地辅助控制器。
 *
 * @details This controller intentionally keeps the high-value P0/P1/P2/P3 product
 * features local-first and deterministic. It does not capture private traffic by
 * itself; it models readiness, replay samples, scoring profiles, business workspaces,
 * reports, and privacy explanations so the UI can guide operators clearly.
 *
 * 这个控制器刻意保持“本地优先、可复盘、可解释”：它不会主动抓取隐私流量，
 * 只负责代理接入向导、样本回放、评分画像、项目工作区、报告和隐私边界说明，
 * 让用户知道每一步在做什么、哪里没准备好、下一步该修什么。
 */
class ProductionSuiteController final : public QObject {
  Q_OBJECT
 public:
  struct ProxyStep {
    QString name;
    QString status;  // pass / warn / fail / unknown
    QString detail;
    QString fixHint;
  };

  struct HealthItem {
    QString name;
    QString status;  // healthy / warning / blocked / unknown
    QString detail;
  };

  struct ScoreProfile {
    double readWeight = 1.0;
    double likeWeight = 20.0;
    double commentWeight = 50.0;
    double oldLikeWeight = 10.0;
    double freshnessWeight = 5.0;
    double originalityWeight = 8.0;
  };

  explicit ProductionSuiteController(QObject* parent = nullptr);

  /** Build the P0 proxy adapter wizard status from visible operator inputs. */
  QVector<ProxyStep> buildProxyWizard(int proxyPort, int bridgePort, bool phoneReachable,
                                      bool metricHit, bool commentHit) const;

  /** Convert wizard steps into a readable, bilingual operator report. */
  QString proxyWizardReport(const QVector<ProxyStep>& steps, bool chinese) const;

  /** Parse sanitized JSON/JSONL replay samples into ContentRecord rows. */
  QVector<ContentRecord> parseReplaySamples(const QString& text, QString* errorMessage) const;

  /** Summarize queue, phone, proxy, bridge, and database readiness for the health monitor. */
  QVector<HealthItem> buildHealthItems(int pendingTasks, int failedTasks, bool phoneReady,
                                       bool proxyReady, bool bridgeReady, bool databaseReady) const;

  /** Classify a raw failure message into a stable user-facing error code. */
  QString classifyFailure(const QString& rawError) const;

  /** Apply a configurable scoring profile to one content record. */
  double scoreRecord(const ContentRecord& record, const ScoreProfile& profile) const;

  /** Produce compact account intelligence lines from article records. */
  QStringList accountInsights(const QVector<ContentRecord>& records, bool chinese) const;

  /** Produce compact keyword intelligence lines from article records. */
  QStringList keywordInsights(const QVector<ContentRecord>& records, bool chinese) const;

  /** Generate a Markdown decision report for operators and managers. */
  QString generateMarkdownReport(const QVector<ContentRecord>& records, const QString& workspace,
                                 const ScoreProfile& profile, bool chinese) const;

  /** Generate local privacy boundary text shown in the Privacy Center. */
  QString privacyBoundaryText(bool chinese) const;

  /** Export structured state for support/debugging without private tokens or raw captures. */
  QJsonObject diagnosticSnapshot(const QVector<ProxyStep>& proxySteps,
                                 const QVector<HealthItem>& healthItems,
                                 const ScoreProfile& profile) const;
};

#pragma once

#include <QWidget>

#include "ProductionSuiteController.h"
#include "UiText.h"

class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QTableWidget;
class QTextEdit;

/**
 * @brief Product-grade P0/P1/P2/P3 feature hub.
 * @brief 面向生产落地的 P0/P1/P2/P3 功能中心。
 *
 * @details The widget groups proxy onboarding, replay, health monitoring, scoring,
 * intelligence, workspaces, reports, and privacy controls in one bilingual UI.
 * It is intentionally explicit: every production concept has a visible control so
 * users can understand status and next steps without reading developer logs.
 *
 * 这个控件把代理接入、样本回放、健康监控、评分画像、项目工作区、报告和隐私中心
 * 放在一个双语界面里。设计目标是“用户能看懂”：每个生产概念都有对应控件、状态、
 * 说明和报告输出，不需要用户去读开发日志。
 */
class ProductionSuiteWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit ProductionSuiteWidget(QWidget* parent = nullptr);
  void setLanguage(UiLanguage language);
  void setRecords(const QVector<ContentRecord>& records);
  void setQueueStats(int pendingTasks, int failedTasks);
  void setReadiness(bool phoneReady, bool bridgeReady, bool databaseReady);

 signals:
  void logMessage(const QString& message);

 private slots:
  void runProxyWizard();
  void replaySamples();
  void refreshHealth();
  void updateScorePreview();
  void generateReport();
  void exportSnapshot();

 private:
  void buildProxyTab();
  void buildReplayTab();
  void buildHealthTab();
  void buildScoringTab();
  void buildWorkspaceTab();
  void buildPrivacyTab();
  void fillTable(QTableWidget* table, const QStringList& headers, const QList<QStringList>& rows);
  ProductionSuiteController::ScoreProfile scoreProfile() const;

  ProductionSuiteController controller_;
  UiLanguage language_ = UiLanguage::English;
  QVector<ContentRecord> records_;
  int pendingTasks_ = 0;
  int failedTasks_ = 0;
  bool phoneReady_ = false;
  bool bridgeReady_ = false;
  bool databaseReady_ = true;
  QVector<ProductionSuiteController::ProxyStep> lastProxySteps_;
  QVector<ProductionSuiteController::HealthItem> lastHealthItems_;

  QTabWidget* tabs_ = nullptr;
  QLabel* proxyIntro_ = nullptr;
  QLabel* proxyPortLabel_ = nullptr;
  QLabel* bridgePortLabel_ = nullptr;
  QSpinBox* proxyPort_ = nullptr;
  QSpinBox* bridgePort_ = nullptr;
  QCheckBox* phoneReachable_ = nullptr;
  QCheckBox* metricHit_ = nullptr;
  QCheckBox* commentHit_ = nullptr;
  QPushButton* runProxyButton_ = nullptr;
  QTableWidget* proxyTable_ = nullptr;
  QPlainTextEdit* proxyReport_ = nullptr;

  QLabel* replayIntro_ = nullptr;
  QTextEdit* replayInput_ = nullptr;
  QPushButton* replayButton_ = nullptr;
  QTableWidget* replayTable_ = nullptr;

  QLabel* healthIntro_ = nullptr;
  QPushButton* refreshHealthButton_ = nullptr;
  QTableWidget* healthTable_ = nullptr;

  QLabel* scoringIntro_ = nullptr;
  QLabel* readWeightLabel_ = nullptr;
  QLabel* likeWeightLabel_ = nullptr;
  QLabel* commentWeightLabel_ = nullptr;
  QLabel* oldLikeWeightLabel_ = nullptr;
  QLabel* originalWeightLabel_ = nullptr;
  QDoubleSpinBox* readWeight_ = nullptr;
  QDoubleSpinBox* likeWeight_ = nullptr;
  QDoubleSpinBox* commentWeight_ = nullptr;
  QDoubleSpinBox* oldLikeWeight_ = nullptr;
  QDoubleSpinBox* originalWeight_ = nullptr;
  QPushButton* scorePreviewButton_ = nullptr;
  QPlainTextEdit* scorePreview_ = nullptr;

  QLabel* workspaceIntro_ = nullptr;
  QLineEdit* workspaceName_ = nullptr;
  QPushButton* reportButton_ = nullptr;
  QPushButton* snapshotButton_ = nullptr;
  QPlainTextEdit* reportOutput_ = nullptr;

  QLabel* privacyIntro_ = nullptr;
  QPlainTextEdit* privacyText_ = nullptr;
};

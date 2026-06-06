#pragma once

#include <QMainWindow>
#include <QTimer>

#include "AppSettings.h"
#include "AutoIngestionController.h"
#include "DatabaseController.h"
#include "KeywordDiscoveryController.h"
#include "KeywordDiscoveryWidget.h"
#include "PhoneDiagnosticsController.h"
#include "PluginManager.h"
#include "QuickStartWidget.h"
#include "UiText.h"

class QAction;
class QDockWidget;
class QMenu;
class QTabWidget;
class AutoIngestionWidget;
class ControlPanelWidget;
class DashboardWidget;
class DataViewerWidget;
class KeywordDiscoveryWidget;
class PhoneDiagnosticsWidget;
class ProductionSuiteWidget;
class RuntimeLogWidget;
class SeedManagerWidget;
class ManualWidget;
class WeChatConfigWidget;

/**
 * @brief 主窗口 / Main window
 *
 * @details 组装仪表盘、控制面板、数据表、种子池、本地数据库和动态插件运行时。
 * Composes dashboard, control panel, data viewer, seed pool, database, and plugin runtime.
 */
class MainWindow final : public QMainWindow {
  Q_OBJECT
 public:
  explicit MainWindow(QWidget* parent = nullptr);

 private slots:
  void refreshData();
  void loadSampleData();
  void loadPlugins();
  void drainPluginRecords();
  void previewSelectedArticle();
  void starSelectedSeed();
  void resetControls();
  void addSeedFromWidget(const QString& gzhId, const QString& name, const QString& category);
  void removeSeedFromWidget(const QString& gzhId);
  void exportArticlesCsv();
  void exportArticlesJson();
  void exportSeedsCsv();
  void showAboutDialog();
  void saveRuntimeSettings(const AppSettings& settings);
  void testLocalBridgePayload();
  void runPhoneDiagnostics();
  void restartAdbServer();
  void testPhoneOpenLink();
  void copyPhoneDiagnosticsReport();
  void exportPhoneDiagnosticsJson();
  void browseDatabasePath();
  void browsePluginDirectory();
  void toggleLanguage();
  void showControlCenter();
  void startQuickOneClick(const QString& keywords, int maxCandidatesPerKeyword, int intervalSeconds, const KeywordHotCriteria& criteria);
  void startQuickOneClick(const QString& keywords, int maxCandidatesPerKeyword, int intervalSeconds, const KeywordHotCriteria& criteria,
                          const QString& supplementalText, bool useSupplemental, bool useAdvancedPhoneSearch,
                          int searchTapX, int searchTapY, int resultTapX, int resultTapY);
  void stopQuickOneClick();
  void openQuickArticles();
  void openQuickReports();
  void generateKeywordSearchUrls(const QString& keywords);
  void autoSearchKeywords(const QString& keywords, int maxCandidatesPerKeyword);
  void startKeywordAutoIngestion(const QString& keywords, int maxCandidatesPerKeyword, const KeywordHotCriteria& criteria);
  void onKeywordSearchStarted();
  void onKeywordSearchFinished(const QVector<KeywordDiscoveryResult>& results);
  void importKeywordDiscoveryResults();
  void enqueueKeywordDiscoveryResults(const KeywordHotCriteria& criteria);
  void addAutoIngestionUrls(const QString& text);
  void startAutoIngestion();
  void stopAutoIngestion();
  void runNextAutoIngestionTask();
  void clearCompletedAutoIngestionTasks();
  void clearAllAutoIngestionTasks();
  void saveAutoIngestionQueue();
  void loadAutoIngestionQueue();

 private:
  void applyTheme();
  void appendLog(const QString& message);
  void refreshSeeds();
  QString pluginDirectory() const;
  bool reopenDatabase(const QString& path);
  void applyLanguage();
  UiLanguage currentLanguage() const;
  QString trLog(const QString& key, const QString& value = QString()) const;
  QString localizedRuntimeMessage(const QString& message) const;
  QString localizedPhonePreflightReason(const QString& reason) const;
  void appendLogKey(const QString& key, const QString& value = QString());
  void refreshAutoIngestionQueue();
  void refreshQuickStartStatus();

  AppSettings settings_;
  DatabaseController database_;
  PluginManager pluginManager_;
  AutoIngestionController autoIngestion_;
  KeywordDiscoveryController keywordDiscovery_;
  PhoneDiagnosticsController phoneDiagnostics_;
  PhoneDiagnosticReport lastPhoneReport_;
  QVector<KeywordDiscoveryResult> keywordResults_;
  QVector<KeywordDiscoveryResult> supplementalKeywordResults_;
  bool startAutoAfterKeywordSearch_ = false;
  bool quickStartActive_ = false;
  int quickLastEnqueued_ = 0;
  KeywordHotCriteria pendingKeywordCriteria_;
  QTimer pluginDrainTimer_;
  UiLanguage language_ = UiLanguage::English;
  QTabWidget* dockTabs_ = nullptr;
  QDockWidget* dock_ = nullptr;
  QMenu* fileMenu_ = nullptr;
  QMenu* pluginMenu_ = nullptr;
  QMenu* actionMenu_ = nullptr;
  QMenu* helpMenu_ = nullptr;
  QAction* loadSamplesAction_ = nullptr;
  QAction* exportArticlesCsvAction_ = nullptr;
  QAction* exportArticlesJsonAction_ = nullptr;
  QAction* exportSeedsCsvAction_ = nullptr;
  QAction* loadPluginsAction_ = nullptr;
  QAction* previewAction_ = nullptr;
  QAction* starSeedAction_ = nullptr;
  QAction* bridgeSmokeAction_ = nullptr;
  QAction* resetAction_ = nullptr;
  QAction* languageAction_ = nullptr;
  QAction* showControlCenterAction_ = nullptr;
  QAction* aboutAction_ = nullptr;
  QuickStartWidget* quickStartWidget_ = nullptr;
  DashboardWidget* dashboard_ = nullptr;
  ControlPanelWidget* controls_ = nullptr;
  KeywordDiscoveryWidget* keywordDiscoveryWidget_ = nullptr;
  PhoneDiagnosticsWidget* phoneDiagnosticsWidget_ = nullptr;
  ProductionSuiteWidget* productionSuiteWidget_ = nullptr;
  AutoIngestionWidget* autoIngestionWidget_ = nullptr;
  DataViewerWidget* viewer_ = nullptr;
  SeedManagerWidget* seeds_ = nullptr;
  RuntimeLogWidget* logs_ = nullptr;
  ManualWidget* manual_ = nullptr;
  WeChatConfigWidget* wechatConfig_ = nullptr;
};

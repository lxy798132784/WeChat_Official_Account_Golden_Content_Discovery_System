#pragma once

#include <QMainWindow>
#include <QTimer>

#include "DatabaseController.h"
#include "PluginManager.h"
#include "AppSettings.h"

class ControlPanelWidget;
class DashboardWidget;
class DataViewerWidget;
class RuntimeLogWidget;
class SeedManagerWidget;
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
  void browseDatabasePath();
  void browsePluginDirectory();

 private:
  void applyTheme();
  void appendLog(const QString& message);
  void refreshSeeds();
  QString pluginDirectory() const;
  bool reopenDatabase(const QString& path);

  AppSettings settings_;
  DatabaseController database_;
  PluginManager pluginManager_;
  QTimer pluginDrainTimer_;
  DashboardWidget* dashboard_ = nullptr;
  ControlPanelWidget* controls_ = nullptr;
  DataViewerWidget* viewer_ = nullptr;
  SeedManagerWidget* seeds_ = nullptr;
  RuntimeLogWidget* logs_ = nullptr;
  WeChatConfigWidget* wechatConfig_ = nullptr;
};

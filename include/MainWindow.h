#pragma once

#include <QMainWindow>
#include <QTimer>

#include "DatabaseController.h"
#include "PluginManager.h"

class DashboardWidget;
class ControlPanelWidget;
class DataViewerWidget;

/**
 * @brief 主窗口
 * Main window
 *
 * @details 组装仪表盘、控制面板、数据表、本地数据库和动态插件运行时。
 * Composes dashboard, control panel, data viewer, local database, and dynamic
 * plugin runtime.
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

 private:
  void applyTheme();
  QString pluginDirectory() const;

  DatabaseController database_;
  PluginManager pluginManager_;
  QTimer pluginDrainTimer_;
  DashboardWidget* dashboard_ = nullptr;
  ControlPanelWidget* controls_ = nullptr;
  DataViewerWidget* viewer_ = nullptr;
};

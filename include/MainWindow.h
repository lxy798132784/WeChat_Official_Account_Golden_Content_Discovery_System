#pragma once

#include <QMainWindow>

#include "DatabaseController.h"

class DashboardWidget;
class ControlPanelWidget;
class DataViewerWidget;

/**
 * @brief 主窗口 / Main window
 *
 * @details 组装仪表盘、控制面板、数据表和本地数据库。
 * Composes dashboard, control panel, data viewer, and local database.
 */
class MainWindow final : public QMainWindow {
  Q_OBJECT
 public:
  explicit MainWindow(QWidget* parent = nullptr);

 private slots:
  void refreshData();
  void loadSampleData();

 private:
  void applyTheme();

  DatabaseController database_;
  DashboardWidget* dashboard_ = nullptr;
  ControlPanelWidget* controls_ = nullptr;
  DataViewerWidget* viewer_ = nullptr;
};

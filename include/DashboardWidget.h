#pragma once

#include <QWidget>

class QLabel;

/**
 * @brief 仪表盘组件 / Dashboard widget
 *
 * @details 展示扫描账号数、高价值发现数和最高分。
 * Shows scanned account count, premium detection count, and top score.
 */
class DashboardWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit DashboardWidget(QWidget* parent = nullptr);
  void setMetrics(int accounts, int detections, double topScore);

 private:
  QLabel* accounts_ = nullptr;
  QLabel* detections_ = nullptr;
  QLabel* topScore_ = nullptr;
};

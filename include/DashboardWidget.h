#pragma once

#include <QWidget>

#include "UiText.h"

class QLabel;

class DashboardWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit DashboardWidget(QWidget* parent = nullptr);
  void setMetrics(int accounts, int detections, double topScore);
  void setLanguage(UiLanguage language);

 private:
  UiLanguage language_ = UiLanguage::English;
  QLabel* accountsTitle_ = nullptr;
  QLabel* detectionsTitle_ = nullptr;
  QLabel* topScoreTitle_ = nullptr;
  QLabel* accounts_ = nullptr;
  QLabel* detections_ = nullptr;
  QLabel* topScore_ = nullptr;
};

#pragma once

#include <QWidget>

class QComboBox;
class QLineEdit;
class QSlider;

/**
 * @brief 控制面板组件 / Control panel widget
 *
 * @details 提供行业、关键词和权重阈值控件。
 * Provides industry, keyword, and scoring-threshold controls.
 */
class ControlPanelWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit ControlPanelWidget(QWidget* parent = nullptr);
  double engagementWeight() const;
  double commentWeight() const;
  double frequencyWeight() const;
  int minimumRead() const;
  void resetDefaults();
  void clearSearch();

 signals:
  void filtersChanged();

 private:
  QComboBox* category_ = nullptr;
  QLineEdit* keyword_ = nullptr;
  QSlider* engagement_ = nullptr;
  QSlider* comment_ = nullptr;
  QSlider* frequency_ = nullptr;
  QSlider* minRead_ = nullptr;
};

#pragma once

#include <QWidget>

#include "UiText.h"

class QComboBox;
class QLabel;
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
  void setLanguage(UiLanguage language);

 signals:
  void filtersChanged();

 private:
  QString currentCategoryCode() const;
  void rebuildCategories();

  UiLanguage language_ = UiLanguage::English;
  QLabel* categoryLabel_ = nullptr;
  QLabel* keywordLabel_ = nullptr;
  QLabel* engagementLabel_ = nullptr;
  QLabel* commentLabel_ = nullptr;
  QLabel* frequencyLabel_ = nullptr;
  QLabel* minReadLabel_ = nullptr;
  QComboBox* category_ = nullptr;
  QLineEdit* keyword_ = nullptr;
  QSlider* engagement_ = nullptr;
  QSlider* comment_ = nullptr;
  QSlider* frequency_ = nullptr;
  QSlider* minRead_ = nullptr;
};

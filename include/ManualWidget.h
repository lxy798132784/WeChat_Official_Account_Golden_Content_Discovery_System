#pragma once

#include <QWidget>

#include "UiText.h"

class QLabel;
class QTextEdit;

class ManualWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit ManualWidget(QWidget* parent = nullptr);
  void setLanguage(UiLanguage language);

 private:
  UiLanguage language_ = UiLanguage::English;
  QLabel* title_ = nullptr;
  QTextEdit* body_ = nullptr;
};

#pragma once

#include <QWidget>

#include "UiText.h"

class QPushButton;
class QTextEdit;

/**
 * @brief 运行日志组件 / Runtime log widget
 */
class RuntimeLogWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit RuntimeLogWidget(QWidget* parent = nullptr);
  void appendLog(const QString& message);
  QString plainText() const;
  void clear();
  void setLanguage(UiLanguage language);

 private:
  UiLanguage language_ = UiLanguage::English;
  QPushButton* clearButton_ = nullptr;
  QTextEdit* logView_ = nullptr;
};

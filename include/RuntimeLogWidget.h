#pragma once

#include <QWidget>

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

 private:
  QTextEdit* logView_ = nullptr;
};

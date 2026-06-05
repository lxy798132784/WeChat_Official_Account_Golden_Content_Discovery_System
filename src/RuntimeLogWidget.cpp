#include "RuntimeLogWidget.h"

#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

RuntimeLogWidget::RuntimeLogWidget(QWidget* parent) : QWidget(parent), logView_(new QTextEdit(this)) {
  auto* layout = new QVBoxLayout(this);
  auto* clearButton = new QPushButton("Clear Log / 清空日志", this);
  logView_->setReadOnly(true);
  logView_->setMinimumHeight(220);
  layout->addWidget(clearButton);
  layout->addWidget(logView_);
  connect(clearButton, &QPushButton::clicked, this, &RuntimeLogWidget::clear);
}

void RuntimeLogWidget::appendLog(const QString& message) {
  logView_->append(message);
}

QString RuntimeLogWidget::plainText() const {
  return logView_->toPlainText();
}

void RuntimeLogWidget::clear() {
  logView_->clear();
}

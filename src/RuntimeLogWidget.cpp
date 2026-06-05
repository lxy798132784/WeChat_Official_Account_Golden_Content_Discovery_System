#include "RuntimeLogWidget.h"

#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

RuntimeLogWidget::RuntimeLogWidget(QWidget* parent)
    : QWidget(parent), clearButton_(new QPushButton(this)), logView_(new QTextEdit(this)) {
  auto* layout = new QVBoxLayout(this);
  logView_->setReadOnly(true);
  logView_->setMinimumHeight(220);
  layout->addWidget(clearButton_);
  layout->addWidget(logView_);
  connect(clearButton_, &QPushButton::clicked, this, &RuntimeLogWidget::clear);
  setLanguage(language_);
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

void RuntimeLogWidget::setLanguage(UiLanguage language) {
  language_ = language;
  clearButton_->setText(language_ == UiLanguage::Chinese ? QStringLiteral("清空日志") : QStringLiteral("Clear Log"));
  clearButton_->setToolTip(language_ == UiLanguage::Chinese ? QStringLiteral("清空当前运行日志视图，不删除数据库。") : QStringLiteral("Clear the current runtime log view without deleting database data."));
  logView_->setToolTip(language_ == UiLanguage::Chinese ? QStringLiteral("脱敏运行日志。") : QStringLiteral("Sanitized runtime log."));
}

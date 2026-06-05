#include "ManualWidget.h"

#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>

ManualWidget::ManualWidget(QWidget* parent)
    : QWidget(parent), title_(new QLabel(this)), body_(new QTextEdit(this)) {
  auto* layout = new QVBoxLayout(this);
  title_->setObjectName(QStringLiteral("manualTitle"));
  body_->setReadOnly(true);
  body_->setMinimumHeight(420);
  layout->addWidget(title_);
  layout->addWidget(body_);
  setLanguage(language_);
}

void ManualWidget::setLanguage(UiLanguage language) {
  language_ = language;
  title_->setText(UiText::text("manual.title", language_));
  body_->setPlainText(UiText::text("manual.body", language_));
  body_->setToolTip(language_ == UiLanguage::Chinese ? QStringLiteral("软件内置中文使用说明书。") : QStringLiteral("Built-in English user manual."));
}

#include "ControlPanelWidget.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>

ControlPanelWidget::ControlPanelWidget(QWidget* parent)
    : QWidget(parent),
      categoryLabel_(new QLabel(this)),
      keywordLabel_(new QLabel(this)),
      engagementLabel_(new QLabel(this)),
      commentLabel_(new QLabel(this)),
      frequencyLabel_(new QLabel(this)),
      minReadLabel_(new QLabel(this)),
      category_(new QComboBox(this)),
      keyword_(new QLineEdit(this)),
      engagement_(new QSlider(Qt::Horizontal, this)),
      comment_(new QSlider(Qt::Horizontal, this)),
      frequency_(new QSlider(Qt::Horizontal, this)),
      minRead_(new QSlider(Qt::Horizontal, this)) {
  auto* layout = new QFormLayout(this);
  connect(keyword_, &QLineEdit::textChanged, this, &ControlPanelWidget::filtersChanged);
  connect(category_, &QComboBox::currentTextChanged, this, &ControlPanelWidget::filtersChanged);

  for (auto* slider : {engagement_, comment_, frequency_}) {
    slider->setRange(0, 100);
    slider->setValue(33);
    connect(slider, &QSlider::valueChanged, this, &ControlPanelWidget::filtersChanged);
  }
  minRead_->setRange(0, 100000);
  minRead_->setValue(1000);
  connect(minRead_, &QSlider::valueChanged, this, &ControlPanelWidget::filtersChanged);

  layout->addRow(categoryLabel_, category_);
  layout->addRow(keywordLabel_, keyword_);
  layout->addRow(engagementLabel_, engagement_);
  layout->addRow(commentLabel_, comment_);
  layout->addRow(frequencyLabel_, frequency_);
  layout->addRow(minReadLabel_, minRead_);
  setLanguage(language_);
}

double ControlPanelWidget::engagementWeight() const {
  return engagement_->value() / 100.0;
}

double ControlPanelWidget::commentWeight() const {
  return comment_->value() / 100.0;
}

double ControlPanelWidget::frequencyWeight() const {
  return frequency_->value() / 100.0;
}

int ControlPanelWidget::minimumRead() const {
  return minRead_->value();
}

void ControlPanelWidget::resetDefaults() {
  category_->setCurrentIndex(0);
  engagement_->setValue(55);
  comment_->setValue(30);
  frequency_->setValue(15);
  minRead_->setValue(1000);
  emit filtersChanged();
}

void ControlPanelWidget::clearSearch() {
  keyword_->clear();
  emit filtersChanged();
}

QString ControlPanelWidget::currentCategoryCode() const {
  return category_->currentData().toString();
}

void ControlPanelWidget::rebuildCategories() {
  const QString selected = currentCategoryCode();
  category_->blockSignals(true);
  category_->clear();
  category_->addItem(UiText::text("filter.all", language_), QStringLiteral("all"));
  category_->addItem(UiText::text("filter.technology", language_), QStringLiteral("technology"));
  category_->addItem(UiText::text("filter.finance", language_), QStringLiteral("finance"));
  category_->addItem(UiText::text("filter.lifestyle", language_), QStringLiteral("lifestyle"));
  const int index = category_->findData(selected.isEmpty() ? QStringLiteral("all") : selected);
  category_->setCurrentIndex(index >= 0 ? index : 0);
  category_->blockSignals(false);
}

void ControlPanelWidget::setLanguage(UiLanguage language) {
  language_ = language;
  categoryLabel_->setText(UiText::text("filter.industry", language_));
  keywordLabel_->setText(UiText::text("filter.keyword", language_));
  engagementLabel_->setText(UiText::text("filter.engagement", language_));
  commentLabel_->setText(UiText::text("filter.comment", language_));
  frequencyLabel_->setText(UiText::text("filter.frequency", language_));
  minReadLabel_->setText(UiText::text("filter.min_read", language_));
  keyword_->setPlaceholderText(UiText::text("filter.keyword.placeholder", language_));
  category_->setToolTip(UiText::text("tip.filter.industry", language_));
  keyword_->setToolTip(UiText::text("tip.filter.keyword", language_));
  engagement_->setToolTip(UiText::text("tip.filter.engagement", language_));
  comment_->setToolTip(UiText::text("tip.filter.comment", language_));
  frequency_->setToolTip(UiText::text("tip.filter.frequency", language_));
  minRead_->setToolTip(UiText::text("tip.filter.min_read", language_));
  rebuildCategories();
}

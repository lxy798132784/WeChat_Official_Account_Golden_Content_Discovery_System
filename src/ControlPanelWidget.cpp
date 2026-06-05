#include "ControlPanelWidget.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSlider>

ControlPanelWidget::ControlPanelWidget(QWidget* parent)
    : QWidget(parent),
      category_(new QComboBox(this)),
      keyword_(new QLineEdit(this)),
      engagement_(new QSlider(Qt::Horizontal, this)),
      comment_(new QSlider(Qt::Horizontal, this)),
      frequency_(new QSlider(Qt::Horizontal, this)),
      minRead_(new QSlider(Qt::Horizontal, this)) {
  auto* layout = new QFormLayout(this);
  category_->addItems({"All / 全部", "Technology / 科技", "Finance / 财经", "Lifestyle / 生活"});
  keyword_->setPlaceholderText("Keyword / 关键词");
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

  layout->addRow("Industry / 行业", category_);
  layout->addRow("Keyword / 关键词", keyword_);
  layout->addRow("Engagement / 互动权重", engagement_);
  layout->addRow("Comment / 评论权重", comment_);
  layout->addRow("Frequency / 频率权重", frequency_);
  layout->addRow("Min Read / 最低阅读", minRead_);
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

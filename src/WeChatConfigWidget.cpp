#include "WeChatConfigWidget.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

WeChatConfigWidget::WeChatConfigWidget(QWidget* parent) : QWidget(parent) {
  auto* rootLayout = new QVBoxLayout(this);
  auto* formLayout = new QFormLayout();

  portSpinBox_ = new QSpinBox(this);
  portSpinBox_->setRange(1024, 65535);
  portSpinBox_->setValue(9000);
  adbCheckBox_ = new QCheckBox(QStringLiteral("Enable ADB automation / 启用 ADB 自动化"), this);
  adbCheckBox_->setChecked(false);
  logView_ = new QTextEdit(this);
  logView_->setReadOnly(true);
  logView_->setMinimumHeight(180);

  formLayout->addRow(QStringLiteral("Bridge Port / 本地桥端口"), portSpinBox_);
  rootLayout->addLayout(formLayout);
  rootLayout->addWidget(adbCheckBox_);
  rootLayout->addWidget(logView_);
}

quint16 WeChatConfigWidget::bridgePort() const {
  return static_cast<quint16>(portSpinBox_->value());
}

bool WeChatConfigWidget::adbAutomationEnabled() const {
  return adbCheckBox_->isChecked();
}

void WeChatConfigWidget::appendLog(const QString& message) {
  logView_->append(message);
}

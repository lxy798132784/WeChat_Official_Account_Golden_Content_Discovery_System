#include "WeChatConfigWidget.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

WeChatConfigWidget::WeChatConfigWidget(QWidget* parent) : QWidget(parent) {
  auto* rootLayout = new QVBoxLayout(this);
  auto* formLayout = new QFormLayout();

  databasePathEdit_ = new QLineEdit(this);
  auto* databaseBrowseButton = new QPushButton(QStringLiteral("Browse"), this);
  auto* databaseRow = new QHBoxLayout();
  databaseRow->addWidget(databasePathEdit_);
  databaseRow->addWidget(databaseBrowseButton);

  pluginDirectoryEdit_ = new QLineEdit(this);
  auto* pluginBrowseButton = new QPushButton(QStringLiteral("Browse"), this);
  auto* pluginRow = new QHBoxLayout();
  pluginRow->addWidget(pluginDirectoryEdit_);
  pluginRow->addWidget(pluginBrowseButton);

  portSpinBox_ = new QSpinBox(this);
  portSpinBox_->setRange(1024, 65535);
  portSpinBox_->setValue(9000);
  adbCheckBox_ = new QCheckBox(QStringLiteral("Enable ADB automation"), this);
  adbCheckBox_->setChecked(false);
  sampleCheckBox_ = new QCheckBox(QStringLiteral("Load sample data on startup"), this);
  sampleCheckBox_->setChecked(false);

  saveButton_ = new QPushButton(QStringLiteral("Save runtime settings"), this);
  testBridgeButton_ = new QPushButton(QStringLiteral("Send local bridge smoke payload"), this);
  auto* actionRow = new QHBoxLayout();
  actionRow->addWidget(saveButton_);
  actionRow->addWidget(testBridgeButton_);

  logView_ = new QTextEdit(this);
  logView_->setReadOnly(true);
  logView_->setMinimumHeight(180);

  formLayout->addRow(QStringLiteral("SQLite database"), databaseRow);
  formLayout->addRow(QStringLiteral("Plugin directory"), pluginRow);
  formLayout->addRow(QStringLiteral("Local bridge port"), portSpinBox_);
  rootLayout->addLayout(formLayout);
  rootLayout->addWidget(adbCheckBox_);
  rootLayout->addWidget(sampleCheckBox_);
  rootLayout->addLayout(actionRow);
  rootLayout->addWidget(logView_);

  connect(saveButton_, &QPushButton::clicked, this, [this]() { emit settingsSaveRequested(settings()); });
  connect(testBridgeButton_, &QPushButton::clicked, this, &WeChatConfigWidget::testBridgeRequested);
  connect(databaseBrowseButton, &QPushButton::clicked, this, &WeChatConfigWidget::browseDatabaseRequested);
  connect(pluginBrowseButton, &QPushButton::clicked, this, &WeChatConfigWidget::browsePluginDirectoryRequested);
}

quint16 WeChatConfigWidget::bridgePort() const {
  return static_cast<quint16>(portSpinBox_->value());
}

bool WeChatConfigWidget::adbAutomationEnabled() const {
  return adbCheckBox_->isChecked();
}

QString WeChatConfigWidget::databasePath() const {
  return databasePathEdit_->text().trimmed();
}

QString WeChatConfigWidget::pluginDirectory() const {
  return pluginDirectoryEdit_->text().trimmed();
}

bool WeChatConfigWidget::autoLoadSamples() const {
  return sampleCheckBox_->isChecked();
}

AppSettings WeChatConfigWidget::settings() const {
  AppSettings value;
  value.databasePath = databasePath();
  value.pluginDirectory = pluginDirectory();
  value.bridgePort = bridgePort();
  value.adbAutomationEnabled = adbAutomationEnabled();
  value.autoLoadSamples = autoLoadSamples();
  return value;
}

void WeChatConfigWidget::setSettings(const AppSettings& settings) {
  databasePathEdit_->setText(settings.databasePath);
  pluginDirectoryEdit_->setText(settings.pluginDirectory);
  portSpinBox_->setValue(settings.bridgePort);
  adbCheckBox_->setChecked(settings.adbAutomationEnabled);
  sampleCheckBox_->setChecked(settings.autoLoadSamples);
}

void WeChatConfigWidget::appendLog(const QString& message) {
  logView_->append(message);
}

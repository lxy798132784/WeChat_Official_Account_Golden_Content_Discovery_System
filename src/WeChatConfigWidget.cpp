#include "WeChatConfigWidget.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

WeChatConfigWidget::WeChatConfigWidget(QWidget* parent)
    : QWidget(parent),
      databaseLabel_(new QLabel(this)),
      pluginLabel_(new QLabel(this)),
      portLabel_(new QLabel(this)),
      logLabel_(new QLabel(this)),
      databasePathEdit_(new QLineEdit(this)),
      pluginDirectoryEdit_(new QLineEdit(this)),
      portSpinBox_(new QSpinBox(this)),
      adbCheckBox_(new QCheckBox(this)),
      sampleCheckBox_(new QCheckBox(this)),
      databaseBrowseButton_(new QPushButton(this)),
      pluginBrowseButton_(new QPushButton(this)),
      saveButton_(new QPushButton(this)),
      testBridgeButton_(new QPushButton(this)),
      logView_(new QTextEdit(this)) {
  auto* rootLayout = new QVBoxLayout(this);
  auto* formLayout = new QFormLayout();

  auto* databaseRow = new QHBoxLayout();
  databaseRow->addWidget(databasePathEdit_);
  databaseRow->addWidget(databaseBrowseButton_);

  auto* pluginRow = new QHBoxLayout();
  pluginRow->addWidget(pluginDirectoryEdit_);
  pluginRow->addWidget(pluginBrowseButton_);

  portSpinBox_->setRange(1024, 65535);
  portSpinBox_->setValue(9000);

  auto* actionRow = new QHBoxLayout();
  actionRow->addWidget(saveButton_);
  actionRow->addWidget(testBridgeButton_);

  logView_->setReadOnly(true);
  logView_->setMinimumHeight(180);

  formLayout->addRow(databaseLabel_, databaseRow);
  formLayout->addRow(pluginLabel_, pluginRow);
  formLayout->addRow(portLabel_, portSpinBox_);
  rootLayout->addLayout(formLayout);
  rootLayout->addWidget(adbCheckBox_);
  rootLayout->addWidget(sampleCheckBox_);
  rootLayout->addLayout(actionRow);
  rootLayout->addWidget(logLabel_);
  rootLayout->addWidget(logView_);

  connect(saveButton_, &QPushButton::clicked, this, [this]() { emit settingsSaveRequested(settings()); });
  connect(testBridgeButton_, &QPushButton::clicked, this, &WeChatConfigWidget::testBridgeRequested);
  connect(databaseBrowseButton_, &QPushButton::clicked, this, &WeChatConfigWidget::browseDatabaseRequested);
  connect(pluginBrowseButton_, &QPushButton::clicked, this, &WeChatConfigWidget::browsePluginDirectoryRequested);
  setLanguage(language_);
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
  value.language = language_ == UiLanguage::Chinese ? QStringLiteral("zh") : QStringLiteral("en");
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
  setLanguage(settings.language == QStringLiteral("zh") ? UiLanguage::Chinese : UiLanguage::English);
}

void WeChatConfigWidget::appendLog(const QString& message) {
  logView_->append(message);
}

void WeChatConfigWidget::setLanguage(UiLanguage language) {
  language_ = language;
  databaseLabel_->setText(UiText::text("wechat.database", language_));
  pluginLabel_->setText(UiText::text("wechat.plugin_dir", language_));
  portLabel_->setText(UiText::text("wechat.port", language_));
  logLabel_->setText(UiText::text("wechat.log", language_));
  databaseBrowseButton_->setText(UiText::text("wechat.browse", language_));
  pluginBrowseButton_->setText(UiText::text("wechat.browse", language_));
  adbCheckBox_->setText(UiText::text("wechat.adb", language_));
  sampleCheckBox_->setText(UiText::text("wechat.samples", language_));
  saveButton_->setText(UiText::text("wechat.save", language_));
  testBridgeButton_->setText(UiText::text("wechat.test", language_));
  databasePathEdit_->setToolTip(UiText::text("tip.wechat.database", language_));
  pluginDirectoryEdit_->setToolTip(UiText::text("tip.wechat.plugin_dir", language_));
  portSpinBox_->setToolTip(UiText::text("tip.wechat.port", language_));
  adbCheckBox_->setToolTip(UiText::text("tip.wechat.adb", language_));
  sampleCheckBox_->setToolTip(UiText::text("tip.wechat.samples", language_));
  saveButton_->setToolTip(UiText::text("tip.wechat.save", language_));
  testBridgeButton_->setToolTip(UiText::text("tip.wechat.test", language_));
  databaseBrowseButton_->setToolTip(UiText::text("tip.wechat.database", language_));
  pluginBrowseButton_->setToolTip(UiText::text("tip.wechat.plugin_dir", language_));
  logView_->setToolTip(UiText::text("tip.wechat.log", language_));
}

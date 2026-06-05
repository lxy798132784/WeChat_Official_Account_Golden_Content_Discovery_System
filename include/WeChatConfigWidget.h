#pragma once

#include <QWidget>

#include "AppSettings.h"
#include "UiText.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTextEdit;

/** Widget for local WeChat ingestion runtime settings and logs. */
class WeChatConfigWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit WeChatConfigWidget(QWidget* parent = nullptr);
  quint16 bridgePort() const;
  bool adbAutomationEnabled() const;
  QString databasePath() const;
  QString pluginDirectory() const;
  bool autoLoadSamples() const;
  AppSettings settings() const;
  void setSettings(const AppSettings& settings);
  void appendLog(const QString& message);
  void setLanguage(UiLanguage language);

 signals:
  void settingsSaveRequested(const AppSettings& settings);
  void testBridgeRequested();
  void browseDatabaseRequested();
  void browsePluginDirectoryRequested();

 private:
  UiLanguage language_ = UiLanguage::English;
  QLabel* databaseLabel_ = nullptr;
  QLabel* pluginLabel_ = nullptr;
  QLabel* portLabel_ = nullptr;
  QLabel* logLabel_ = nullptr;
  QLineEdit* databasePathEdit_ = nullptr;
  QLineEdit* pluginDirectoryEdit_ = nullptr;
  QSpinBox* portSpinBox_ = nullptr;
  QCheckBox* adbCheckBox_ = nullptr;
  QCheckBox* sampleCheckBox_ = nullptr;
  QPushButton* databaseBrowseButton_ = nullptr;
  QPushButton* pluginBrowseButton_ = nullptr;
  QPushButton* saveButton_ = nullptr;
  QPushButton* testBridgeButton_ = nullptr;
  QTextEdit* logView_ = nullptr;
};

#pragma once

#include <QWidget>

#include "AppSettings.h"

class QCheckBox;
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

 signals:
  void settingsSaveRequested(const AppSettings& settings);
  void testBridgeRequested();
  void browseDatabaseRequested();
  void browsePluginDirectoryRequested();

 private:
  QLineEdit* databasePathEdit_ = nullptr;
  QLineEdit* pluginDirectoryEdit_ = nullptr;
  QSpinBox* portSpinBox_ = nullptr;
  QCheckBox* adbCheckBox_ = nullptr;
  QCheckBox* sampleCheckBox_ = nullptr;
  QPushButton* saveButton_ = nullptr;
  QPushButton* testBridgeButton_ = nullptr;
  QTextEdit* logView_ = nullptr;
};

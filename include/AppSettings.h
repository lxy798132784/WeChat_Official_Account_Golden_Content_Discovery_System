#pragma once

#include <QString>

struct AppSettings {
  QString databasePath;
  QString pluginDirectory;
  QString language = QStringLiteral("en");
  quint16 bridgePort = 9000;
  bool adbAutomationEnabled = false;
  bool autoLoadSamples = false;
};

class AppSettingsController final {
 public:
  static AppSettings load();
  static bool save(const AppSettings& settings, QString* errorMessage = nullptr);
  static QString defaultDatabasePath();
  static QString defaultPluginDirectory();
  static QString settingsFilePath();
};

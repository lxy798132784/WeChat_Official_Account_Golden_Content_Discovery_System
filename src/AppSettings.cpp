#include "AppSettings.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace {
constexpr quint16 kDefaultBridgePort = 9000;
}

QString AppSettingsController::defaultDatabasePath() {
  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (dataDir.isEmpty()) {
    dataDir = QDir::home().absoluteFilePath(".premium-content-radar");
  }
  QDir().mkpath(dataDir);
  return QDir(dataDir).absoluteFilePath("wechat_radar.db");
}

QString AppSettingsController::defaultPluginDirectory() {
  return QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("plugins");
}

QString AppSettingsController::settingsFilePath() {
  QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
  if (configDir.isEmpty()) {
    configDir = QDir::home().absoluteFilePath(".config/premium-content-radar");
  }
  QDir().mkpath(configDir);
  return QDir(configDir).absoluteFilePath("settings.json");
}

AppSettings AppSettingsController::load() {
  AppSettings settings;
  settings.databasePath = defaultDatabasePath();
  settings.pluginDirectory = defaultPluginDirectory();
  settings.language = QStringLiteral("zh");
  settings.bridgePort = kDefaultBridgePort;
  settings.adbAutomationEnabled = false;
  settings.autoLoadSamples = false;

  QFile file(settingsFilePath());
  if (!file.open(QIODevice::ReadOnly)) {
    return settings;
  }
  const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
  if (!document.isObject()) {
    return settings;
  }
  const QJsonObject object = document.object();
  settings.databasePath = object.value(QStringLiteral("database_path")).toString(settings.databasePath);
  settings.pluginDirectory = object.value(QStringLiteral("plugin_directory")).toString(settings.pluginDirectory);
  const QString language = object.value(QStringLiteral("language")).toString(settings.language);
  if (language == QStringLiteral("zh") || language == QStringLiteral("en")) {
    settings.language = language;
  }
  const int port = object.value(QStringLiteral("bridge_port")).toInt(settings.bridgePort);
  if (port >= 1024 && port <= 65535) {
    settings.bridgePort = static_cast<quint16>(port);
  }
  settings.adbAutomationEnabled = object.value(QStringLiteral("adb_automation_enabled")).toBool(false);
  settings.autoLoadSamples = object.value(QStringLiteral("auto_load_samples")).toBool(false);
  return settings;
}

bool AppSettingsController::save(const AppSettings& settings, QString* errorMessage) {
  QJsonObject object;
  object.insert(QStringLiteral("database_path"), settings.databasePath);
  object.insert(QStringLiteral("plugin_directory"), settings.pluginDirectory);
  object.insert(QStringLiteral("language"), settings.language);
  object.insert(QStringLiteral("bridge_port"), static_cast<int>(settings.bridgePort));
  object.insert(QStringLiteral("adb_automation_enabled"), settings.adbAutomationEnabled);
  object.insert(QStringLiteral("auto_load_samples"), settings.autoLoadSamples);

  QFile file(settingsFilePath());
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    if (errorMessage != nullptr) {
      *errorMessage = file.errorString();
    }
    return false;
  }
  file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
  return true;
}

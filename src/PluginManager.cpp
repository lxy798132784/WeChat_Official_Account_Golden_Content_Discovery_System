#include "PluginManager.h"

#include <QCoreApplication>
#include <QFileInfo>

PluginManager::PluginManager(QObject* parent) : QObject(parent) {}

int PluginManager::loadFromDirectory(const QString& directoryPath) {
  QDir directory(directoryPath);
  if (!directory.exists()) {
    logLines_.append(QStringLiteral("Plugin directory not found: %1").arg(directoryPath));
    return providers_.size();
  }

  const QStringList entries = directory.entryList(QDir::Files | QDir::NoDotAndDotDot);
  for (const QString& entry : entries) {
    const QString absolutePath = directory.absoluteFilePath(entry);
    auto loader = std::make_unique<QPluginLoader>(absolutePath);
    QObject* instance = loader->instance();
    if (instance == nullptr) {
      logLines_.append(QStringLiteral("Skipped %1: %2").arg(entry, loader->errorString()));
      continue;
    }

    auto* provider = qobject_cast<IContentProvider*>(instance);
    if (provider == nullptr) {
      logLines_.append(QStringLiteral("Skipped %1: interface mismatch").arg(entry));
      loader->unload();
      continue;
    }

    providers_.append(provider);
    logLines_.append(QStringLiteral("Loaded %1 (%2)").arg(provider->displayName(), provider->providerId()));
    loaders_.push_back(std::move(loader));
  }
  return providers_.size();
}

QVector<IContentProvider*> PluginManager::providers() const {
  return providers_;
}

QStringList PluginManager::logLines() const {
  return logLines_;
}

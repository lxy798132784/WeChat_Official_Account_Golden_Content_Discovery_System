#pragma once

#include <QDir>
#include <QObject>
#include <QPluginLoader>
#include <QStringList>
#include <memory>
#include <vector>

#include "IContentProvider.h"

/**
 * @brief 插件运行时管理器
 * Plugin runtime manager
 *
 * @details 使用 QPluginLoader 扫描插件目录并持有加载器生命周期，避免插件实例悬空。
 * Scans a plugin directory with QPluginLoader and owns loader lifetimes so plugin
 * instances remain valid while the application is running.
 */
class PluginManager final : public QObject {
  Q_OBJECT
 public:
  explicit PluginManager(QObject* parent = nullptr);

  /**
   * @brief 扫描并加载目录中的内容插件
   * Scan and load content plugins from a directory
   *
   * @param directoryPath 插件目录 / Plugin directory
   * @return 成功识别的供应商数量 / Number of recognized providers
   */
  int loadFromDirectory(const QString& directoryPath);

  /**
   * @brief 已加载供应商
   * Loaded providers
   *
   * @return 插件接口指针列表 / Provider interface pointers
   */
  QVector<IContentProvider*> providers() const;

  /**
   * @brief 加载日志
   * Loading log
   *
   * @return 人类可读日志 / Human-readable log lines
   */
  QStringList logLines() const;

 private:
  std::vector<std::unique_ptr<QPluginLoader>> loaders_;
  QVector<IContentProvider*> providers_;
  QStringList logLines_;
};

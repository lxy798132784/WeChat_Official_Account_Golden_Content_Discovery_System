#pragma once

#include <QSqlDatabase>
#include <QVector>

#include "content_record.h"

/**
 * @brief 中央存储控制器 / Central storage controller
 *
 * @details 封装 SQLite schema、种子池、文章指标批量写入和导出。
 * Encapsulates SQLite schema, seed storage, transactional article batching, and export.
 */
class DatabaseController final {
 public:
  bool open(const QString& databasePath);
  bool initialize();
  bool addSeed(const QString& gzhId, const QString& name, const QString& category);
  bool removeSeed(const QString& gzhId);
  QVector<SeedRecord> listSeeds() const;
  bool enqueueArticle(const ContentRecord& record);
  bool flush();
  QVector<ContentRecord> listArticles() const;
  int articleCount() const;
  QString lastError() const;

 private:
  void setError(const QString& error) const;

  QSqlDatabase database_;
  QVector<ContentRecord> pendingRecords_;
  mutable QString lastError_;
};

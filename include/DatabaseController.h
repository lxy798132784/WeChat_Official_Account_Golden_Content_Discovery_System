#pragma once

#include <QSqlDatabase>
#include <QVector>
#include "content_record.h"

/**
 * @brief 中央存储控制器 / Central storage controller
 *
 * @details 封装 SQLite schema、种子池和文章指标批量写入。
 * Encapsulates SQLite schema, seed storage, and transactional article metric batching.
 */
class DatabaseController final {
 public:
  bool open(const QString& databasePath);
  bool initialize();
  bool addSeed(const QString& gzhId, const QString& name, const QString& category);
  bool enqueueArticle(const ContentRecord& record);
  bool flush();
  QVector<ContentRecord> listArticles() const;
  int articleCount() const;

 private:
  QSqlDatabase database_;
  QVector<ContentRecord> pendingRecords_;
};

#pragma once

#include <QVector>

#include "content_record.h"

/**
 * @brief 导出控制器 / Export controller
 *
 * @details 将文章和种子池导出为 CSV 或 JSON 文件。
 * Exports articles and seeds to CSV or JSON files.
 */
class ExportController final {
 public:
  static bool exportArticlesCsv(const QVector<ContentRecord>& records, const QString& path,
                                QString* errorMessage = nullptr);
  static bool exportArticlesJson(const QVector<ContentRecord>& records, const QString& path,
                                 QString* errorMessage = nullptr);
  static bool exportSeedsCsv(const QVector<SeedRecord>& records, const QString& path,
                             QString* errorMessage = nullptr);
};

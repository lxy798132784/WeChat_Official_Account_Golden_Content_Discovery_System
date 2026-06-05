#pragma once

#include <QDateTime>
#include <QString>

/**
 * @brief 内容记录 / Content record
 *
 * @details 保存公众号文章的核心指标，用于数据库、模型和插件之间传输。
 * Stores article metrics shared by the database, model, and provider plugins.
 */
struct ContentRecord {
  int id = 0;
  QString title;
  QString url;
  QString category;
  QString accountName;
  QString gzhId;
  int readNum = 0;
  int likeNum = 0;
  int oldLikeNum = 0;
  int commentNum = 0;
  int articleCount30d = 0;
  QDateTime timestamp = QDateTime::currentDateTimeUtc();
};

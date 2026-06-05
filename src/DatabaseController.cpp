#include "DatabaseController.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

void DatabaseController::setError(const QString& error) const {
  lastError_ = error;
}

bool DatabaseController::open(const QString& databasePath) {
  const QString name = QStringLiteral("radar_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
  database_ = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), name);
  database_.setDatabaseName(databasePath);
  if (!database_.open()) {
    setError(database_.lastError().text());
    return false;
  }
  return initialize();
}

bool DatabaseController::initialize() {
  QSqlQuery query(database_);
  const bool seedsOk = query.exec(QStringLiteral(
      "CREATE TABLE IF NOT EXISTS gzh_seeds ("
      "id INTEGER PRIMARY KEY, gzh_id TEXT UNIQUE, name TEXT, category TEXT, "
      "article_count_30d INT DEFAULT 0, account_score REAL DEFAULT 0.0)"));
  if (!seedsOk) {
    setError(query.lastError().text());
    return false;
  }
  const bool articlesOk = query.exec(QStringLiteral(
      "CREATE TABLE IF NOT EXISTS articles ("
      "id INTEGER PRIMARY KEY, title TEXT, url TEXT UNIQUE, category TEXT, account_name TEXT, "
      "gzh_id TEXT, read_num INT, like_num INT, old_like_num INT, comment_num INT DEFAULT 0, "
      "article_count_30d INT DEFAULT 0, publish_time DATETIME, timestamp DATETIME)"));
  if (!articlesOk) {
    setError(query.lastError().text());
    return false;
  }
  const bool publishTimeColumnOk = query.exec(QStringLiteral(
      "ALTER TABLE articles ADD COLUMN publish_time DATETIME"));
  if (!publishTimeColumnOk && !query.lastError().text().contains(QStringLiteral("duplicate column name"))) {
    setError(query.lastError().text());
    return false;
  }
  return true;
}

bool DatabaseController::addSeed(const QString& gzhId, const QString& name, const QString& category) {
  if (gzhId.trimmed().isEmpty()) {
    setError(QStringLiteral("gzh_id is required"));
    return false;
  }
  QSqlQuery query(database_);
  query.prepare(QStringLiteral(
      "INSERT INTO gzh_seeds(gzh_id,name,category) VALUES(?,?,?) "
      "ON CONFLICT(gzh_id) DO UPDATE SET name=excluded.name, category=excluded.category"));
  query.addBindValue(gzhId.trimmed());
  query.addBindValue(name.trimmed());
  query.addBindValue(category.trimmed());
  if (!query.exec()) {
    setError(query.lastError().text());
    return false;
  }
  return true;
}

bool DatabaseController::removeSeed(const QString& gzhId) {
  QSqlQuery query(database_);
  query.prepare(QStringLiteral("DELETE FROM gzh_seeds WHERE gzh_id=?"));
  query.addBindValue(gzhId);
  if (!query.exec()) {
    setError(query.lastError().text());
    return false;
  }
  return true;
}

QVector<SeedRecord> DatabaseController::listSeeds() const {
  QVector<SeedRecord> rows;
  QSqlQuery query(database_);
  if (!query.exec(QStringLiteral(
          "SELECT id,gzh_id,name,category,article_count_30d,account_score FROM gzh_seeds ORDER BY name ASC"))) {
    setError(query.lastError().text());
    return rows;
  }
  while (query.next()) {
    SeedRecord seed;
    seed.id = query.value(0).toInt();
    seed.gzhId = query.value(1).toString();
    seed.name = query.value(2).toString();
    seed.category = query.value(3).toString();
    seed.articleCount30d = query.value(4).toInt();
    seed.accountScore = query.value(5).toDouble();
    rows.push_back(seed);
  }
  return rows;
}

bool DatabaseController::enqueueArticle(const ContentRecord& record) {
  if (record.url.trimmed().isEmpty()) {
    setError(QStringLiteral("article url is required"));
    return false;
  }
  pendingRecords_.push_back(record);
  if (pendingRecords_.size() >= 50) {
    return flush();
  }
  return true;
}

bool DatabaseController::flush() {
  if (pendingRecords_.isEmpty()) {
    return true;
  }
  if (!database_.transaction()) {
    setError(database_.lastError().text());
    return false;
  }
  QSqlQuery query(database_);
  query.prepare(QStringLiteral(
      "INSERT INTO articles(title,url,category,account_name,gzh_id,read_num,like_num,old_like_num,"
      "comment_num,article_count_30d,publish_time,timestamp) VALUES(?,?,?,?,?,?,?,?,?,?,?,?) "
      "ON CONFLICT(url) DO UPDATE SET title=excluded.title, category=excluded.category, "
      "account_name=excluded.account_name, gzh_id=excluded.gzh_id, read_num=excluded.read_num, "
      "like_num=excluded.like_num, old_like_num=excluded.old_like_num, comment_num=excluded.comment_num, "
      "article_count_30d=excluded.article_count_30d, publish_time=excluded.publish_time, "
      "timestamp=excluded.timestamp"));
  for (const auto& record : pendingRecords_) {
    query.addBindValue(record.title);
    query.addBindValue(record.url);
    query.addBindValue(record.category);
    query.addBindValue(record.accountName);
    query.addBindValue(record.gzhId);
    query.addBindValue(record.readNum);
    query.addBindValue(record.likeNum);
    query.addBindValue(record.oldLikeNum);
    query.addBindValue(record.commentNum);
    query.addBindValue(record.articleCount30d);
    query.addBindValue(record.publishTime.isValid() ? record.publishTime.toString(Qt::ISODate) : QString());
    query.addBindValue(record.timestamp.toString(Qt::ISODate));
    if (!query.exec()) {
      setError(query.lastError().text());
      database_.rollback();
      return false;
    }
  }
  pendingRecords_.clear();
  if (!database_.commit()) {
    setError(database_.lastError().text());
    return false;
  }
  return true;
}

QVector<ContentRecord> DatabaseController::listArticles() const {
  QVector<ContentRecord> rows;
  QSqlQuery query(database_);
  if (!query.exec(QStringLiteral(
          "SELECT id,title,url,category,account_name,gzh_id,read_num,like_num,old_like_num,comment_num,"
          "article_count_30d,publish_time,timestamp FROM articles ORDER BY publish_time DESC, read_num DESC"))) {
    setError(query.lastError().text());
    return rows;
  }
  while (query.next()) {
    ContentRecord record;
    record.id = query.value(0).toInt();
    record.title = query.value(1).toString();
    record.url = query.value(2).toString();
    record.category = query.value(3).toString();
    record.accountName = query.value(4).toString();
    record.gzhId = query.value(5).toString();
    record.readNum = query.value(6).toInt();
    record.likeNum = query.value(7).toInt();
    record.oldLikeNum = query.value(8).toInt();
    record.commentNum = query.value(9).toInt();
    record.articleCount30d = query.value(10).toInt();
    record.publishTime = QDateTime::fromString(query.value(11).toString(), Qt::ISODate);
    record.timestamp = QDateTime::fromString(query.value(12).toString(), Qt::ISODate);
    rows.push_back(record);
  }
  return rows;
}

int DatabaseController::articleCount() const {
  QSqlQuery query(database_);
  if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM articles")) || !query.next()) {
    return 0;
  }
  return query.value(0).toInt();
}

QString DatabaseController::lastError() const {
  return lastError_;
}

#include "DatabaseController.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

bool DatabaseController::open(const QString& databasePath) {
  const QString name = QStringLiteral("radar_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
  database_ = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), name);
  database_.setDatabaseName(databasePath);
  return database_.open() && initialize();
}

bool DatabaseController::initialize() {
  QSqlQuery query(database_);
  const bool seedsOk = query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS gzh_seeds (id INTEGER PRIMARY KEY, gzh_id TEXT UNIQUE, name TEXT, category TEXT, article_count_30d INT DEFAULT 0, account_score REAL DEFAULT 0.0)"));
  const bool articlesOk = query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS articles (id INTEGER PRIMARY KEY, title TEXT, url TEXT UNIQUE, category TEXT, account_name TEXT, gzh_id TEXT, read_num INT, like_num INT, old_like_num INT, comment_num INT DEFAULT 0, article_count_30d INT DEFAULT 0, timestamp DATETIME)"));
  return seedsOk && articlesOk;
}

bool DatabaseController::addSeed(const QString& gzhId, const QString& name, const QString& category) {
  QSqlQuery query(database_);
  query.prepare(QStringLiteral("INSERT INTO gzh_seeds(gzh_id,name,category) VALUES(?,?,?) ON CONFLICT(gzh_id) DO UPDATE SET name=excluded.name, category=excluded.category"));
  query.addBindValue(gzhId);
  query.addBindValue(name);
  query.addBindValue(category);
  return query.exec();
}

bool DatabaseController::enqueueArticle(const ContentRecord& record) {
  pendingRecords_.push_back(record);
  if (pendingRecords_.size() >= 50) return flush();
  return true;
}

bool DatabaseController::flush() {
  if (pendingRecords_.isEmpty()) return true;
  if (!database_.transaction()) return false;
  QSqlQuery query(database_);
  query.prepare(QStringLiteral("INSERT INTO articles(title,url,category,account_name,gzh_id,read_num,like_num,old_like_num,comment_num,article_count_30d,timestamp) VALUES(?,?,?,?,?,?,?,?,?,?,?) ON CONFLICT(url) DO UPDATE SET title=excluded.title, category=excluded.category, account_name=excluded.account_name, gzh_id=excluded.gzh_id, read_num=excluded.read_num, like_num=excluded.like_num, old_like_num=excluded.old_like_num, comment_num=excluded.comment_num, article_count_30d=excluded.article_count_30d, timestamp=excluded.timestamp"));
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
    query.addBindValue(record.timestamp.toString(Qt::ISODate));
    if (!query.exec()) {
      database_.rollback();
      return false;
    }
  }
  pendingRecords_.clear();
  return database_.commit();
}

QVector<ContentRecord> DatabaseController::listArticles() const {
  QVector<ContentRecord> rows;
  QSqlQuery query(database_);
  if (!query.exec(QStringLiteral("SELECT id,title,url,category,account_name,gzh_id,read_num,like_num,old_like_num,comment_num,article_count_30d,timestamp FROM articles ORDER BY read_num DESC"))) return rows;
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
    record.timestamp = QDateTime::fromString(query.value(11).toString(), Qt::ISODate);
    rows.push_back(record);
  }
  return rows;
}

int DatabaseController::articleCount() const {
  QSqlQuery query(database_);
  if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM articles")) || !query.next()) return 0;
  return query.value(0).toInt();
}

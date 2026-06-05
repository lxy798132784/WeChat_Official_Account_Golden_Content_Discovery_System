#include "ExportController.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

namespace {
QString csvEscape(const QString& value) {
  QString escaped = value;
  escaped.replace('"', "\"\"");
  return QStringLiteral("\"%1\"").arg(escaped);
}

void setFileError(QString* errorMessage, const QFile& file) {
  if (errorMessage != nullptr) {
    *errorMessage = file.errorString();
  }
}
}  // namespace

bool ExportController::exportArticlesCsv(const QVector<ContentRecord>& records, const QString& path,
                                         QString* errorMessage) {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    setFileError(errorMessage, file);
    return false;
  }
  QTextStream out(&file);
  out << "title,account,gzh_id,category,read_num,like_num,old_like_num,comment_num,article_count_30d,timestamp,url\n";
  for (const auto& record : records) {
    out << csvEscape(record.title) << ',' << csvEscape(record.accountName) << ',' << csvEscape(record.gzhId)
        << ',' << csvEscape(record.category) << ',' << record.readNum << ',' << record.likeNum << ','
        << record.oldLikeNum << ',' << record.commentNum << ',' << record.articleCount30d << ','
        << csvEscape(record.timestamp.toString(Qt::ISODate)) << ',' << csvEscape(record.url) << '\n';
  }
  return true;
}

bool ExportController::exportArticlesJson(const QVector<ContentRecord>& records, const QString& path,
                                          QString* errorMessage) {
  QJsonArray array;
  for (const auto& record : records) {
    QJsonObject object;
    object["title"] = record.title;
    object["account_name"] = record.accountName;
    object["gzh_id"] = record.gzhId;
    object["category"] = record.category;
    object["read_num"] = record.readNum;
    object["like_num"] = record.likeNum;
    object["old_like_num"] = record.oldLikeNum;
    object["comment_num"] = record.commentNum;
    object["article_count_30d"] = record.articleCount30d;
    object["timestamp"] = record.timestamp.toString(Qt::ISODate);
    object["url"] = record.url;
    array.push_back(object);
  }
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    setFileError(errorMessage, file);
    return false;
  }
  file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
  return true;
}

bool ExportController::exportSeedsCsv(const QVector<SeedRecord>& records, const QString& path,
                                      QString* errorMessage) {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    setFileError(errorMessage, file);
    return false;
  }
  QTextStream out(&file);
  out << "gzh_id,name,category,article_count_30d,account_score\n";
  for (const auto& seed : records) {
    out << csvEscape(seed.gzhId) << ',' << csvEscape(seed.name) << ',' << csvEscape(seed.category) << ','
        << seed.articleCount30d << ',' << seed.accountScore << '\n';
  }
  return true;
}

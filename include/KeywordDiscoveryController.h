#pragma once

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

struct KeywordDiscoveryResult {
  QString keyword;
  QString title;
  QString accountName;
  QString category;
  QString url;
  int readNum = 0;
  int likeNum = 0;
  int commentNum = 0;
  double hotScore = 0.0;
};

/**
 * Keyword-first discovery module.
 *
 * This module does not depend on seed accounts. It creates lawful external search
 * URLs for operator keywords and can import sanitized search-result JSON exported
 * by a local adapter. The imported result URLs can then be appended to the ADB
 * auto-ingestion queue.
 */
class KeywordDiscoveryController final : public QObject {
  Q_OBJECT
 public:
  explicit KeywordDiscoveryController(QObject* parent = nullptr);

  bool isSearching() const;
  QVector<KeywordDiscoveryResult> results() const;
  int maxResultsPerKeyword() const;
  void setMaxResultsPerKeyword(int count);

  static QStringList parseKeywords(const QString& text);
  static bool matchesHotCriteria(const KeywordDiscoveryResult& result, int minimumReadCount,
                                 int minimumLikeCount, int minimumCommentCount,
                                 int minimumHotScore);
  static QString searchUrlForKeyword(const QString& keyword);
  static QVector<KeywordDiscoveryResult> parseResultsJson(const QByteArray& data, QString* errorMessage = nullptr);
  static QVector<KeywordDiscoveryResult> parseSearchHtml(const QString& keyword, const QByteArray& html);
  static QString resultsToQueueText(const QVector<KeywordDiscoveryResult>& results, int minimumReadCount);
  static QString resultsToQueueText(const QVector<KeywordDiscoveryResult>& results, int minimumReadCount,
                                    int minimumLikeCount, int minimumCommentCount,
                                    int minimumHotScore);
  static double estimateHotScore(int readNum, int likeNum, int commentNum);

 public slots:
  void searchKeywords(const QString& text);
  void cancelSearch();

 signals:
  void searchStarted();
  void searchFinished(const QVector<KeywordDiscoveryResult>& results);
  void logMessage(const QString& message);

 private:
  void startNextKeyword();
  void finishSearch();
  void handleReply(QNetworkReply* reply, const QString& keyword);

  QNetworkAccessManager network_;
  QStringList pendingKeywords_;
  QVector<KeywordDiscoveryResult> results_;
  int maxResultsPerKeyword_ = 10;
  bool searching_ = false;
};

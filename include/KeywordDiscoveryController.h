#pragma once

#include <QDate>
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
  QDate publishDate;
  int readNum = 0;
  int likeNum = 0;
  int commentNum = 0;
  double hotScore = 0.0;
};

struct KeywordTargetCollectionPlan {
  QStringList keywords;
  QDate startDate;
  QDate endDate;
  int minRead = 0;
  int maxRead = 100000000;
  int targetCount = 20;
  int maxCandidatesPerKeyword = 10;
  int maxScanCount = 200;
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
  static KeywordTargetCollectionPlan buildTargetCollectionPlan(
      const QString& keywordsText, const QDate& startDate, const QDate& endDate, int minRead, int maxRead,
      int targetCount, int maxCandidatesPerKeyword, int maxScanCount);
  static KeywordTargetCollectionPlan buildRecentMonthEmotionCollectionPlan(
      const QDate& today, const QString& keywordsText, int minRead, int maxRead, int targetCount,
      int maxCandidatesPerKeyword, int maxScanCount);
  static bool matchesHotCriteria(const KeywordDiscoveryResult& result, int minimumReadCount,
                                 int minimumLikeCount, int minimumCommentCount,
                                 int minimumHotScore);
  static bool matchesTargetCollectionPlan(const KeywordDiscoveryResult& result,
                                          const KeywordTargetCollectionPlan& plan);
  static QVector<KeywordDiscoveryResult> filterTargetCollectionResults(
      const QVector<KeywordDiscoveryResult>& results, const KeywordTargetCollectionPlan& plan);
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

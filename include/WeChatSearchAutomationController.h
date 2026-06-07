#pragma once

#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

/**
 * Advanced, opt-in phone-side WeChat search helper.
 *
 * This class deliberately does not bypass WeChat or collect credentials. It only
 * builds and, when explicitly enabled, runs conservative ADB commands that open
 * WeChat, dump the visible UI tree for operator-side troubleshooting, and input a
 * search keyword through normal Android input events. It is a fallback for cases
 * where desktop search sources are unavailable; the primary path remains desktop
 * candidate search plus ADB article opening.
 */
class WeChatSearchAutomationController final {
 public:
  struct Options {
    bool enabled = false;
    bool autoLocateSearch = true;
    bool tapNetworkResults = true;
    int searchTapX = 0;
    int searchTapY = 0;
    int resultTapX = 0;
    int resultTapY = 0;
    int waitMs = 1200;
  };

  struct AdaptivePoints {
    int searchX = 0;
    int searchY = 0;
    int networkResultX = 0;
    int networkResultY = 0;
    int articlesTabX = 0;
    int articlesTabY = 0;
    int firstArticleX = 0;
    int firstArticleY = 0;
  };

  struct Result {
    bool success = false;
    QString stage;
    QString message;
    QStringList commands;
    QString uiDumpPreview;
  };

  struct CollectionCriteria {
    int maxArticles = 5;
    int minRead = 10000;
    int minLike = 100;
    int minOldLike = 20;
    int minComment = 0;
    int perArticleWaitMs = 30000;
    int maxAttempts = 1;
    bool officialAccountOnly = true;
    bool rejectAds = true;
    bool rejectMiniPrograms = true;
    bool rejectVideoAccounts = true;
  };

  struct CandidateMetrics {
    QString url;
    int read = 0;
    int like = 0;
    int oldLike = 0;
    int comment = 0;
  };

  struct CollectionDecision {
    bool accepted = false;
    QString reason;
  };

  struct CollectionSummary {
    int attempted = 0;
    int opened = 0;
    int captured = 0;
    int accepted = 0;
    int rejectedByThreshold = 0;
    int rejectedAsDuplicate = 0;
    int failed = 0;
    QStringList failureReasons;
  };

  static QStringList adbLaunchWeChatArguments();
  static QStringList adbTapArguments(int x, int y);
  static QStringList adbInputTextArguments(const QString& text);
  static QStringList adbKeyEventArguments(int keyCode);
  static QStringList adbDumpUiArguments();
  static QStringList adbCatUiDumpArguments();
  static QStringList adbWmSizeArguments();
  static QStringList adbWindowFocusArguments();
  static QString escapeInputText(const QString& text);
  static bool hasChineseInputRisk(const QString& text);
  static AdaptivePoints adaptivePoints(int screenWidth, int screenHeight);
  static QStringList networkSearchTextHints();
  static QStringList articlesTabTextHints();
  static Result dryRunPlan(const QStringList& keywords, const Options& options);
  static bool findNodeCenterByText(const QString& uiXml, const QStringList& textHints, int* x, int* y);
  static bool findArticleEntryCenter(const QString& uiXml, int* x, int* y);
  static bool findArticlesTabCenter(const QString& uiXml, int* x, int* y);
  static bool findOfficialAccountArticleResultCenter(const QString& uiXml, int* x, int* y);
  static bool uiDumpLooksLikeArticlePage(const QString& uiXml);
  static bool uiDumpLooksLikeMetricsVisible(const QString& uiXml);
  static bool windowFocusLooksLikeArticleContainer(const QString& windowDump);
  static bool windowFocusLooksLikeRejectedContent(const QString& windowDump);
  static CollectionDecision evaluateCandidate(const CandidateMetrics& metrics, const CollectionCriteria& criteria,
                                              const QSet<QString>& seenUrls = {});
  static CollectionSummary summarizeCollection(const QVector<CandidateMetrics>& metrics, const CollectionCriteria& criteria);
  static CollectionSummary runCollection(const QStringList& keywords, const Options& options,
                                         const CollectionCriteria& criteria);
  static Result run(const QStringList& keywords, const Options& options);

 private:
  static bool runAdb(const QStringList& arguments, QString* output, QString* errorMessage, int timeoutMs = 10000);
};

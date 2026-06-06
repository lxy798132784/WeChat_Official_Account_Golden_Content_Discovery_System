#pragma once

#include <QWidget>

#include "KeywordDiscoveryWidget.h"
#include "UiText.h"

class QLabel;
class QPushButton;
class QSpinBox;
class QTextEdit;

/**
 * @brief One-click Quick Start panel for phone + keyword ingestion.
 *
 * Keeps normal users on one screen: connect a phone, enter keywords, press one
 * button, then watch diagnostics/search/queue/metric progress. Advanced proxy,
 * replay, and queue pages remain available but are not required for the happy path.
 */
class QuickStartWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit QuickStartWidget(QWidget* parent = nullptr);

  void setLanguage(UiLanguage language);
  QString keywordsText() const;
  int maxCandidatesPerKeyword() const;
  int intervalSeconds() const;
  KeywordHotCriteria hotCriteria() const;

  void setRunning(bool running);
  void setPhoneStatus(const QString& status, const QString& detail = QString());
  void setSearchStatus(const QString& status, const QString& detail = QString());
  void setQueueStatus(const QString& status, const QString& detail = QString());
  void setMetricStatus(const QString& status, const QString& detail = QString());
  void setSummary(int candidates, int enqueued, int opened, int failed, int articles);

 signals:
  void startOneClickRequested(const QString& keywords, int maxCandidatesPerKeyword,
                              int intervalSeconds, const KeywordHotCriteria& criteria);
  void stopRequested();
  void openArticlesRequested();
  void openReportsRequested();

 private:
  QString stepText(const QString& key, const QString& status, const QString& detail) const;

  UiLanguage language_ = UiLanguage::English;
  QLabel* titleLabel_ = nullptr;
  QLabel* introLabel_ = nullptr;
  QLabel* keywordsLabel_ = nullptr;
  QTextEdit* keywordsEdit_ = nullptr;
  QLabel* optionsLabel_ = nullptr;
  QLabel* maxCandidatesLabel_ = nullptr;
  QLabel* intervalLabel_ = nullptr;
  QLabel* minimumReadLabel_ = nullptr;
  QLabel* minimumLikeLabel_ = nullptr;
  QLabel* minimumCommentLabel_ = nullptr;
  QLabel* minimumHotScoreLabel_ = nullptr;
  QSpinBox* maxCandidatesSpinBox_ = nullptr;
  QSpinBox* intervalSpinBox_ = nullptr;
  QSpinBox* minimumReadSpinBox_ = nullptr;
  QSpinBox* minimumLikeSpinBox_ = nullptr;
  QSpinBox* minimumCommentSpinBox_ = nullptr;
  QSpinBox* minimumHotScoreSpinBox_ = nullptr;
  QPushButton* startButton_ = nullptr;
  QPushButton* stopButton_ = nullptr;
  QPushButton* openArticlesButton_ = nullptr;
  QPushButton* openReportsButton_ = nullptr;
  QLabel* phoneStatusLabel_ = nullptr;
  QLabel* searchStatusLabel_ = nullptr;
  QLabel* queueStatusLabel_ = nullptr;
  QLabel* metricStatusLabel_ = nullptr;
  QLabel* summaryLabel_ = nullptr;
  int lastCandidates_ = 0;
  int lastEnqueued_ = 0;
  int lastOpened_ = 0;
  int lastFailed_ = 0;
  int lastArticles_ = 0;
};

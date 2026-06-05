#pragma once

#include <QWidget>

#include "KeywordDiscoveryController.h"
#include "UiText.h"

class QLabel;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTextEdit;

struct KeywordHotCriteria {
  int minimumReadCount = 0;
  int minimumLikeCount = 0;
  int minimumCommentCount = 0;
  int minimumHotScore = 0;
};

class KeywordDiscoveryWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit KeywordDiscoveryWidget(QWidget* parent = nullptr);

  void setLanguage(UiLanguage language);
  void setResults(const QVector<KeywordDiscoveryResult>& results);
  void setSearching(bool searching);
  QString keywordsText() const;
  int minimumReadCount() const;
  int minimumLikeCount() const;
  int minimumCommentCount() const;
  int minimumHotScore() const;
  KeywordHotCriteria hotCriteria() const;
  int maxCandidatesPerKeyword() const;

 signals:
  void generateSearchUrlsRequested(const QString& keywords);
  void autoSearchRequested(const QString& keywords, int maxCandidatesPerKeyword);
  void startKeywordAutoIngestionRequested(const QString& keywords, int maxCandidatesPerKeyword, const KeywordHotCriteria& criteria);
  void importResultsRequested();
  void enqueueHotResultsRequested(const KeywordHotCriteria& criteria);

 private:
  UiLanguage language_ = UiLanguage::English;
  QLabel* introLabel_ = nullptr;
  QLabel* keywordsLabel_ = nullptr;
  QLabel* minimumReadLabel_ = nullptr;
  QLabel* minimumLikeLabel_ = nullptr;
  QLabel* minimumCommentLabel_ = nullptr;
  QLabel* minimumHotScoreLabel_ = nullptr;
  QLabel* maxCandidatesLabel_ = nullptr;
  QLabel* resultsLabel_ = nullptr;
  QTextEdit* keywordsEdit_ = nullptr;
  QSpinBox* minimumReadSpinBox_ = nullptr;
  QSpinBox* minimumLikeSpinBox_ = nullptr;
  QSpinBox* minimumCommentSpinBox_ = nullptr;
  QSpinBox* minimumHotScoreSpinBox_ = nullptr;
  QSpinBox* maxCandidatesSpinBox_ = nullptr;
  QPushButton* generateSearchUrlsButton_ = nullptr;
  QPushButton* autoSearchButton_ = nullptr;
  QPushButton* startKeywordAutoButton_ = nullptr;
  QPushButton* importResultsButton_ = nullptr;
  QPushButton* enqueueHotResultsButton_ = nullptr;
  QTableWidget* resultsTable_ = nullptr;
};

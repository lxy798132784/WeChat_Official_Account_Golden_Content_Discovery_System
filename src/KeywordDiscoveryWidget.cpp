#include "KeywordDiscoveryWidget.h"

#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>

#include "UiText.h"

KeywordDiscoveryWidget::KeywordDiscoveryWidget(QWidget* parent) : QWidget(parent) {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(10, 10, 10, 10);
  layout->setSpacing(8);

  introLabel_ = new QLabel(this);
  introLabel_->setWordWrap(true);
  layout->addWidget(introLabel_);

  keywordsLabel_ = new QLabel(this);
  layout->addWidget(keywordsLabel_);

  keywordsEdit_ = new QTextEdit(this);
  keywordsEdit_->setMinimumHeight(88);
  layout->addWidget(keywordsEdit_);

  auto* criteriaGrid = new QGridLayout();
  criteriaGrid->setHorizontalSpacing(8);
  criteriaGrid->setVerticalSpacing(6);
  minimumReadLabel_ = new QLabel(this);
  minimumReadSpinBox_ = new QSpinBox(this);
  minimumReadSpinBox_->setRange(0, 100000000);
  minimumReadSpinBox_->setSingleStep(1000);
  minimumReadSpinBox_->setValue(10000);
  minimumLikeLabel_ = new QLabel(this);
  minimumLikeSpinBox_ = new QSpinBox(this);
  minimumLikeSpinBox_->setRange(0, 100000000);
  minimumLikeSpinBox_->setSingleStep(100);
  minimumLikeSpinBox_->setValue(0);
  minimumCommentLabel_ = new QLabel(this);
  minimumCommentSpinBox_ = new QSpinBox(this);
  minimumCommentSpinBox_->setRange(0, 100000000);
  minimumCommentSpinBox_->setSingleStep(10);
  minimumCommentSpinBox_->setValue(0);
  minimumHotScoreLabel_ = new QLabel(this);
  minimumHotScoreSpinBox_ = new QSpinBox(this);
  minimumHotScoreSpinBox_->setRange(0, 100000000);
  minimumHotScoreSpinBox_->setSingleStep(1000);
  minimumHotScoreSpinBox_->setValue(0);
  maxCandidatesLabel_ = new QLabel(this);
  maxCandidatesSpinBox_ = new QSpinBox(this);
  maxCandidatesSpinBox_->setRange(1, 50);
  maxCandidatesSpinBox_->setValue(10);
  criteriaGrid->addWidget(minimumReadLabel_, 0, 0);
  criteriaGrid->addWidget(minimumReadSpinBox_, 0, 1);
  criteriaGrid->addWidget(minimumLikeLabel_, 0, 2);
  criteriaGrid->addWidget(minimumLikeSpinBox_, 0, 3);
  criteriaGrid->addWidget(minimumCommentLabel_, 1, 0);
  criteriaGrid->addWidget(minimumCommentSpinBox_, 1, 1);
  criteriaGrid->addWidget(minimumHotScoreLabel_, 1, 2);
  criteriaGrid->addWidget(minimumHotScoreSpinBox_, 1, 3);
  criteriaGrid->addWidget(maxCandidatesLabel_, 2, 0);
  criteriaGrid->addWidget(maxCandidatesSpinBox_, 2, 1);
  layout->addLayout(criteriaGrid);

  auto* controls = new QHBoxLayout();
  generateSearchUrlsButton_ = new QPushButton(this);
  autoSearchButton_ = new QPushButton(this);
  startKeywordAutoButton_ = new QPushButton(this);
  importResultsButton_ = new QPushButton(this);
  enqueueHotResultsButton_ = new QPushButton(this);
  controls->addWidget(startKeywordAutoButton_);
  controls->addWidget(autoSearchButton_);
  controls->addWidget(generateSearchUrlsButton_);
  controls->addWidget(importResultsButton_);
  controls->addWidget(enqueueHotResultsButton_);
  controls->addStretch();
  layout->addLayout(controls);

  resultsLabel_ = new QLabel(this);
  layout->addWidget(resultsLabel_);

  resultsTable_ = new QTableWidget(this);
  resultsTable_->setColumnCount(7);
  resultsTable_->horizontalHeader()->setStretchLastSection(true);
  resultsTable_->verticalHeader()->setVisible(false);
  resultsTable_->setAlternatingRowColors(true);
  resultsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
  layout->addWidget(resultsTable_, 1);

  connect(generateSearchUrlsButton_, &QPushButton::clicked, this, [this]() { emit generateSearchUrlsRequested(keywordsText()); });
  connect(autoSearchButton_, &QPushButton::clicked, this, [this]() { emit autoSearchRequested(keywordsText(), maxCandidatesPerKeyword()); });
  connect(startKeywordAutoButton_, &QPushButton::clicked, this, [this]() { emit startKeywordAutoIngestionRequested(keywordsText(), maxCandidatesPerKeyword(), hotCriteria()); });
  connect(importResultsButton_, &QPushButton::clicked, this, &KeywordDiscoveryWidget::importResultsRequested);
  connect(enqueueHotResultsButton_, &QPushButton::clicked, this, [this]() { emit enqueueHotResultsRequested(hotCriteria()); });

  setLanguage(language_);
}

QString KeywordDiscoveryWidget::keywordsText() const {
  return keywordsEdit_->toPlainText();
}

int KeywordDiscoveryWidget::minimumReadCount() const {
  return minimumReadSpinBox_->value();
}

int KeywordDiscoveryWidget::minimumLikeCount() const {
  return minimumLikeSpinBox_->value();
}

int KeywordDiscoveryWidget::minimumCommentCount() const {
  return minimumCommentSpinBox_->value();
}

int KeywordDiscoveryWidget::minimumHotScore() const {
  return minimumHotScoreSpinBox_->value();
}

KeywordHotCriteria KeywordDiscoveryWidget::hotCriteria() const {
  KeywordHotCriteria criteria;
  criteria.minimumReadCount = minimumReadCount();
  criteria.minimumLikeCount = minimumLikeCount();
  criteria.minimumCommentCount = minimumCommentCount();
  criteria.minimumHotScore = minimumHotScore();
  return criteria;
}

int KeywordDiscoveryWidget::maxCandidatesPerKeyword() const {
  return maxCandidatesSpinBox_->value();
}

void KeywordDiscoveryWidget::setSearching(bool searching) {
  autoSearchButton_->setEnabled(!searching);
  startKeywordAutoButton_->setEnabled(!searching);
  generateSearchUrlsButton_->setEnabled(!searching);
  importResultsButton_->setEnabled(!searching);
  enqueueHotResultsButton_->setEnabled(!searching);
}

void KeywordDiscoveryWidget::setLanguage(UiLanguage language) {
  language_ = language;
  introLabel_->setText(UiText::text(QStringLiteral("discover.intro"), language_));
  keywordsLabel_->setText(UiText::text(QStringLiteral("discover.keywords"), language_));
  minimumReadLabel_->setText(UiText::text(QStringLiteral("discover.min_read"), language_));
  minimumLikeLabel_->setText(UiText::text(QStringLiteral("discover.min_like"), language_));
  minimumCommentLabel_->setText(UiText::text(QStringLiteral("discover.min_comment"), language_));
  minimumHotScoreLabel_->setText(UiText::text(QStringLiteral("discover.min_hot_score"), language_));
  maxCandidatesLabel_->setText(UiText::text(QStringLiteral("discover.max_candidates"), language_));
  resultsLabel_->setText(UiText::text(QStringLiteral("discover.results"), language_));
  startKeywordAutoButton_->setText(UiText::text(QStringLiteral("discover.start_keyword_auto"), language_));
  autoSearchButton_->setText(UiText::text(QStringLiteral("discover.auto_search"), language_));
  generateSearchUrlsButton_->setText(UiText::text(QStringLiteral("discover.generate_search_urls"), language_));
  importResultsButton_->setText(UiText::text(QStringLiteral("discover.import_results"), language_));
  enqueueHotResultsButton_->setText(UiText::text(QStringLiteral("discover.enqueue_hot"), language_));
  keywordsEdit_->setPlaceholderText(UiText::text(QStringLiteral("discover.keywords_placeholder"), language_));
  keywordsEdit_->setToolTip(UiText::text(QStringLiteral("tip.discover.keywords"), language_));
  startKeywordAutoButton_->setToolTip(UiText::text(QStringLiteral("tip.discover.start_keyword_auto"), language_));
  autoSearchButton_->setToolTip(UiText::text(QStringLiteral("tip.discover.auto_search"), language_));
  generateSearchUrlsButton_->setToolTip(UiText::text(QStringLiteral("tip.discover.generate"), language_));
  importResultsButton_->setToolTip(UiText::text(QStringLiteral("tip.discover.import"), language_));
  enqueueHotResultsButton_->setToolTip(UiText::text(QStringLiteral("tip.discover.enqueue"), language_));
  resultsTable_->setHorizontalHeaderLabels({UiText::text(QStringLiteral("discover.col.keyword"), language_),
                                            UiText::text(QStringLiteral("discover.col.title"), language_),
                                            UiText::text(QStringLiteral("discover.col.account"), language_),
                                            UiText::text(QStringLiteral("discover.col.read"), language_),
                                            UiText::text(QStringLiteral("discover.col.like"), language_),
                                            UiText::text(QStringLiteral("discover.col.score"), language_),
                                            UiText::text(QStringLiteral("discover.col.url"), language_)});
}

void KeywordDiscoveryWidget::setResults(const QVector<KeywordDiscoveryResult>& results) {
  resultsTable_->setRowCount(results.size());
  for (int row = 0; row < results.size(); ++row) {
    const KeywordDiscoveryResult& result = results.at(row);
    resultsTable_->setItem(row, 0, new QTableWidgetItem(result.keyword));
    resultsTable_->setItem(row, 1, new QTableWidgetItem(result.title));
    resultsTable_->setItem(row, 2, new QTableWidgetItem(result.accountName));
    resultsTable_->setItem(row, 3, new QTableWidgetItem(QString::number(result.readNum)));
    resultsTable_->setItem(row, 4, new QTableWidgetItem(QString::number(result.likeNum)));
    resultsTable_->setItem(row, 5, new QTableWidgetItem(QString::number(result.hotScore, 'f', 0)));
    resultsTable_->setItem(row, 6, new QTableWidgetItem(result.url));
  }
  resultsTable_->resizeColumnsToContents();
}

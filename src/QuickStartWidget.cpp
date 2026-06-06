#include "QuickStartWidget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

#include "UiText.h"

QuickStartWidget::QuickStartWidget(QWidget* parent) : QWidget(parent) {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(12, 12, 12, 12);
  layout->setSpacing(10);

  titleLabel_ = new QLabel(this);
  titleLabel_->setObjectName(QStringLiteral("manualTitle"));
  layout->addWidget(titleLabel_);

  introLabel_ = new QLabel(this);
  introLabel_->setWordWrap(true);
  layout->addWidget(introLabel_);

  keywordsLabel_ = new QLabel(this);
  layout->addWidget(keywordsLabel_);
  keywordsEdit_ = new QTextEdit(this);
  keywordsEdit_->setMinimumHeight(90);
  layout->addWidget(keywordsEdit_);

  optionsLabel_ = new QLabel(this);
  layout->addWidget(optionsLabel_);
  auto* grid = new QGridLayout();
  grid->setHorizontalSpacing(8);
  grid->setVerticalSpacing(6);

  maxCandidatesSpinBox_ = new QSpinBox(this);
  maxCandidatesSpinBox_->setRange(1, 50);
  maxCandidatesSpinBox_->setValue(10);
  intervalSpinBox_ = new QSpinBox(this);
  intervalSpinBox_->setRange(5, 3600);
  intervalSpinBox_->setValue(15);
  minimumReadSpinBox_ = new QSpinBox(this);
  minimumReadSpinBox_->setRange(0, 100000000);
  minimumReadSpinBox_->setSingleStep(1000);
  minimumReadSpinBox_->setValue(0);
  minimumLikeSpinBox_ = new QSpinBox(this);
  minimumLikeSpinBox_->setRange(0, 100000000);
  minimumLikeSpinBox_->setSingleStep(100);
  minimumCommentSpinBox_ = new QSpinBox(this);
  minimumCommentSpinBox_->setRange(0, 100000000);
  minimumCommentSpinBox_->setSingleStep(10);
  minimumHotScoreSpinBox_ = new QSpinBox(this);
  minimumHotScoreSpinBox_->setRange(0, 100000000);
  minimumHotScoreSpinBox_->setSingleStep(1000);

  maxCandidatesLabel_ = new QLabel(this);
  intervalLabel_ = new QLabel(this);
  minimumReadLabel_ = new QLabel(this);
  minimumLikeLabel_ = new QLabel(this);
  minimumCommentLabel_ = new QLabel(this);
  minimumHotScoreLabel_ = new QLabel(this);
  grid->addWidget(maxCandidatesLabel_, 0, 0);
  grid->addWidget(maxCandidatesSpinBox_, 0, 1);
  grid->addWidget(intervalLabel_, 0, 2);
  grid->addWidget(intervalSpinBox_, 0, 3);
  grid->addWidget(minimumReadLabel_, 1, 0);
  grid->addWidget(minimumReadSpinBox_, 1, 1);
  grid->addWidget(minimumLikeLabel_, 1, 2);
  grid->addWidget(minimumLikeSpinBox_, 1, 3);
  grid->addWidget(minimumCommentLabel_, 2, 0);
  grid->addWidget(minimumCommentSpinBox_, 2, 1);
  grid->addWidget(minimumHotScoreLabel_, 2, 2);
  grid->addWidget(minimumHotScoreSpinBox_, 2, 3);
  minimumReadSpinBox_->setToolTip(UiText::text(QStringLiteral("tip.discover.enqueue"), language_));
  layout->addLayout(grid);

  auto* buttons = new QHBoxLayout();
  startButton_ = new QPushButton(this);
  stopButton_ = new QPushButton(this);
  openArticlesButton_ = new QPushButton(this);
  openReportsButton_ = new QPushButton(this);
  buttons->addWidget(startButton_);
  buttons->addWidget(stopButton_);
  buttons->addWidget(openArticlesButton_);
  buttons->addWidget(openReportsButton_);
  buttons->addStretch();
  layout->addLayout(buttons);

  phoneStatusLabel_ = new QLabel(this);
  searchStatusLabel_ = new QLabel(this);
  queueStatusLabel_ = new QLabel(this);
  metricStatusLabel_ = new QLabel(this);
  summaryLabel_ = new QLabel(this);
  for (QLabel* label : {phoneStatusLabel_, searchStatusLabel_, queueStatusLabel_, metricStatusLabel_, summaryLabel_}) {
    label->setWordWrap(true);
    layout->addWidget(label);
  }
  layout->addStretch();

  connect(startButton_, &QPushButton::clicked, this, [this]() {
    emit startOneClickRequested(keywordsText(), maxCandidatesPerKeyword(), intervalSeconds(), hotCriteria());
  });
  connect(stopButton_, &QPushButton::clicked, this, &QuickStartWidget::stopRequested);
  connect(openArticlesButton_, &QPushButton::clicked, this, &QuickStartWidget::openArticlesRequested);
  connect(openReportsButton_, &QPushButton::clicked, this, &QuickStartWidget::openReportsRequested);

  setLanguage(language_);
  setRunning(false);
  setPhoneStatus(QStringLiteral("pending"));
  setSearchStatus(QStringLiteral("pending"));
  setQueueStatus(QStringLiteral("pending"));
  setMetricStatus(QStringLiteral("pending"));
  setSummary(0, 0, 0, 0, 0);
}

QString QuickStartWidget::keywordsText() const { return keywordsEdit_->toPlainText(); }
int QuickStartWidget::maxCandidatesPerKeyword() const { return maxCandidatesSpinBox_->value(); }
int QuickStartWidget::intervalSeconds() const { return intervalSpinBox_->value(); }

KeywordHotCriteria QuickStartWidget::hotCriteria() const {
  KeywordHotCriteria criteria;
  criteria.minimumReadCount = minimumReadSpinBox_->value();
  criteria.minimumLikeCount = minimumLikeSpinBox_->value();
  criteria.minimumCommentCount = minimumCommentSpinBox_->value();
  criteria.minimumHotScore = minimumHotScoreSpinBox_->value();
  return criteria;
}

void QuickStartWidget::setRunning(bool running) {
  startButton_->setEnabled(!running);
  stopButton_->setEnabled(running);
}

QString QuickStartWidget::stepText(const QString& key, const QString& status, const QString& detail) const {
  QString icon = QStringLiteral("○");
  if (status == QStringLiteral("pass")) icon = QStringLiteral("✅");
  if (status == QStringLiteral("running")) icon = QStringLiteral("▶");
  if (status == QStringLiteral("warn")) icon = QStringLiteral("🟡");
  if (status == QStringLiteral("fail")) icon = QStringLiteral("❌");
  const QString base = QStringLiteral("%1 %2").arg(icon, UiText::text(key, language_));
  return detail.trimmed().isEmpty() ? base : QStringLiteral("%1 — %2").arg(base, detail.trimmed());
}

void QuickStartWidget::setPhoneStatus(const QString& status, const QString& detail) {
  phoneStatusLabel_->setText(stepText(QStringLiteral("quick.step.phone"), status, detail));
}
void QuickStartWidget::setSearchStatus(const QString& status, const QString& detail) {
  searchStatusLabel_->setText(stepText(QStringLiteral("quick.step.search"), status, detail));
}
void QuickStartWidget::setQueueStatus(const QString& status, const QString& detail) {
  queueStatusLabel_->setText(stepText(QStringLiteral("quick.step.queue"), status, detail));
}
void QuickStartWidget::setMetricStatus(const QString& status, const QString& detail) {
  metricStatusLabel_->setText(stepText(QStringLiteral("quick.step.metrics"), status, detail));
}

void QuickStartWidget::setSummary(int candidates, int enqueued, int opened, int failed, int articles) {
  lastCandidates_ = candidates;
  lastEnqueued_ = enqueued;
  lastOpened_ = opened;
  lastFailed_ = failed;
  lastArticles_ = articles;
  summaryLabel_->setText(UiText::text(QStringLiteral("quick.summary"), language_)
                             .arg(candidates)
                             .arg(enqueued)
                             .arg(opened)
                             .arg(failed)
                             .arg(articles));
}

void QuickStartWidget::setLanguage(UiLanguage language) {
  language_ = language;
  titleLabel_->setText(UiText::text(QStringLiteral("quick.title"), language_));
  introLabel_->setText(UiText::text(QStringLiteral("quick.intro"), language_));
  keywordsLabel_->setText(UiText::text(QStringLiteral("quick.keywords"), language_));
  keywordsEdit_->setPlaceholderText(UiText::text(QStringLiteral("quick.keywords_placeholder"), language_));
  keywordsEdit_->setToolTip(UiText::text(QStringLiteral("tip.quick.keywords"), language_));
  optionsLabel_->setText(UiText::text(QStringLiteral("quick.options"), language_));
  maxCandidatesLabel_->setText(UiText::text(QStringLiteral("quick.max_candidates"), language_));
  intervalLabel_->setText(UiText::text(QStringLiteral("quick.interval"), language_));
  minimumReadLabel_->setText(UiText::text(QStringLiteral("discover.min_read"), language_));
  minimumLikeLabel_->setText(UiText::text(QStringLiteral("discover.min_like"), language_));
  minimumCommentLabel_->setText(UiText::text(QStringLiteral("discover.min_comment"), language_));
  minimumHotScoreLabel_->setText(UiText::text(QStringLiteral("discover.min_hot_score"), language_));
  startButton_->setText(UiText::text(QStringLiteral("quick.start"), language_));
  stopButton_->setText(UiText::text(QStringLiteral("quick.stop"), language_));
  openArticlesButton_->setText(UiText::text(QStringLiteral("quick.open_articles"), language_));
  openReportsButton_->setText(UiText::text(QStringLiteral("quick.open_reports"), language_));
  startButton_->setToolTip(UiText::text(QStringLiteral("tip.quick.start"), language_));
  setSummary(lastCandidates_, lastEnqueued_, lastOpened_, lastFailed_, lastArticles_);
}

#include "DashboardWidget.h"

#include <QGridLayout>
#include <QLabel>

DashboardWidget::DashboardWidget(QWidget* parent)
    : QWidget(parent),
      accountsTitle_(new QLabel(this)),
      detectionsTitle_(new QLabel(this)),
      topScoreTitle_(new QLabel(this)),
      accounts_(new QLabel(QStringLiteral("0"), this)),
      detections_(new QLabel(QStringLiteral("0"), this)),
      topScore_(new QLabel(QStringLiteral("0.00"), this)) {
  auto* layout = new QGridLayout(this);
  layout->addWidget(accountsTitle_, 0, 0);
  layout->addWidget(accounts_, 1, 0);
  layout->addWidget(detectionsTitle_, 0, 1);
  layout->addWidget(detections_, 1, 1);
  layout->addWidget(topScoreTitle_, 0, 2);
  layout->addWidget(topScore_, 1, 2);
  setLanguage(language_);
}

void DashboardWidget::setMetrics(int accounts, int detections, double topScore) {
  accounts_->setText(QString::number(accounts));
  detections_->setText(QString::number(detections));
  topScore_->setText(QString::number(topScore, 'f', 2));
}

void DashboardWidget::setLanguage(UiLanguage language) {
  language_ = language;
  accountsTitle_->setText(UiText::text("dash.accounts", language_));
  detectionsTitle_->setText(UiText::text("dash.detections", language_));
  topScoreTitle_->setText(UiText::text("dash.top_score", language_));
  accounts_->setToolTip(UiText::text("dash.accounts", language_));
  detections_->setToolTip(UiText::text("dash.detections", language_));
  topScore_->setToolTip(UiText::text("dash.top_score", language_));
}

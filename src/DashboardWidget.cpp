#include "DashboardWidget.h"
#include <QGridLayout>
#include <QLabel>
DashboardWidget::DashboardWidget(QWidget* parent):QWidget(parent),accounts_(new QLabel("0")),detections_(new QLabel("0")),topScore_(new QLabel("0.00")){auto* layout=new QGridLayout(this); layout->addWidget(new QLabel("Scanned Accounts / 已扫描账号"),0,0); layout->addWidget(accounts_,1,0); layout->addWidget(new QLabel("Premium Detections / 高价值发现"),0,1); layout->addWidget(detections_,1,1); layout->addWidget(new QLabel("Top Score / 最高评分"),0,2); layout->addWidget(topScore_,1,2);}
void DashboardWidget::setMetrics(int accounts,int detections,double topScore){accounts_->setText(QString::number(accounts)); detections_->setText(QString::number(detections)); topScore_->setText(QString::number(topScore,'f',2));}

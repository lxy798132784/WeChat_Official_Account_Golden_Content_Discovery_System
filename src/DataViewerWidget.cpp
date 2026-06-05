#include "DataViewerWidget.h"
#include <QHeaderView>
#include <QStandardItem>
#include <QTableView>
#include <QVBoxLayout>
DataViewerWidget::DataViewerWidget(QWidget* parent):QWidget(parent),model_(new QStandardItemModel(this)),proxy_(new PremiumContentFilterProxyModel(this)),table_(new QTableView){model_->setHorizontalHeaderLabels({"Title","Account","Category","Read","Like","OldLike","Comment","Freq30d","Score","URL"}); proxy_->setSourceModel(model_); table_->setModel(proxy_); table_->setSortingEnabled(true); table_->horizontalHeader()->setStretchLastSection(true); auto* layout=new QVBoxLayout(this); layout->addWidget(table_);}
void DataViewerWidget::setRecords(const QVector<ContentRecord>& records){model_->removeRows(0,model_->rowCount()); for(const auto& r:records){QList<QStandardItem*> row; row << new QStandardItem(r.title) << new QStandardItem(r.accountName) << new QStandardItem(r.category) << new QStandardItem(QString::number(r.readNum)) << new QStandardItem(QString::number(r.likeNum)) << new QStandardItem(QString::number(r.oldLikeNum)) << new QStandardItem(QString::number(r.commentNum)) << new QStandardItem(QString::number(r.articleCount30d)) << new QStandardItem(QString()) << new QStandardItem(r.url); model_->appendRow(row);}}
PremiumContentFilterProxyModel* DataViewerWidget::proxy(){return proxy_;}

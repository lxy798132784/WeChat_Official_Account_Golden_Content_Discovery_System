#include "DataViewerWidget.h"

#include <QHeaderView>
#include <QItemSelectionModel>
#include <QStandardItem>
#include <QVBoxLayout>

DataViewerWidget::DataViewerWidget(QWidget* parent)
    : QWidget(parent),
      model_(new QStandardItemModel(this)),
      proxy_(new PremiumContentFilterProxyModel(this)),
      table_(new QTableView(this)) {
  model_->setHorizontalHeaderLabels({"Title", "Account", "Category", "Read", "Like",
                                     "OldLike", "Comment", "Freq30d", "Score", "URL"});
  proxy_->setSourceModel(model_);
  table_->setModel(proxy_);
  table_->setSortingEnabled(true);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::SingleSelection);
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

  auto* layout = new QVBoxLayout(this);
  layout->addWidget(table_);
  connect(table_->selectionModel(), &QItemSelectionModel::selectionChanged, this,
          &DataViewerWidget::selectionChanged);
}

void DataViewerWidget::setRecords(const QVector<ContentRecord>& records) {
  records_ = records;
  model_->removeRows(0, model_->rowCount());
  for (const auto& record : records) {
    QList<QStandardItem*> row;
    row << new QStandardItem(record.title) << new QStandardItem(record.accountName)
        << new QStandardItem(record.category) << new QStandardItem(QString::number(record.readNum))
        << new QStandardItem(QString::number(record.likeNum))
        << new QStandardItem(QString::number(record.oldLikeNum))
        << new QStandardItem(QString::number(record.commentNum))
        << new QStandardItem(QString::number(record.articleCount30d)) << new QStandardItem(QString())
        << new QStandardItem(record.url);
    model_->appendRow(row);
  }
}

PremiumContentFilterProxyModel* DataViewerWidget::proxy() {
  return proxy_;
}

bool DataViewerWidget::hasSelection() const {
  return table_->selectionModel() != nullptr && table_->selectionModel()->hasSelection();
}

ContentRecord DataViewerWidget::selectedRecord() const {
  if (!hasSelection()) {
    return ContentRecord();
  }
  const QModelIndex proxyIndex = table_->selectionModel()->selectedRows().first();
  const QModelIndex sourceIndex = proxy_->mapToSource(proxyIndex);
  if (!sourceIndex.isValid() || sourceIndex.row() < 0 || sourceIndex.row() >= records_.size()) {
    return ContentRecord();
  }
  return records_.at(sourceIndex.row());
}

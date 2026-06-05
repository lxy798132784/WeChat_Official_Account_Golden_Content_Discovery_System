#include "SeedManagerWidget.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItem>
#include <QTableView>
#include <QVBoxLayout>

SeedManagerWidget::SeedManagerWidget(QWidget* parent)
    : QWidget(parent),
      gzhIdEdit_(new QLineEdit(this)),
      nameEdit_(new QLineEdit(this)),
      categoryCombo_(new QComboBox(this)),
      table_(new QTableView(this)),
      model_(new QStandardItemModel(this)) {
  auto* root = new QVBoxLayout(this);
  auto* form = new QFormLayout();
  categoryCombo_->addItems({"Technology", "Finance", "Media", "Lifestyle", "Other"});
  gzhIdEdit_->setPlaceholderText("gh_xxx / 公众号原始ID");
  nameEdit_->setPlaceholderText("Account name / 账号名称");
  form->addRow("GZH ID / 公众号ID", gzhIdEdit_);
  form->addRow("Name / 名称", nameEdit_);
  form->addRow("Category / 分类", categoryCombo_);
  root->addLayout(form);

  auto* addButton = new QPushButton("Add or Update Seed / 添加或更新种子", this);
  auto* removeButton = new QPushButton("Remove Selected / 删除选中", this);
  auto* exportButton = new QPushButton("Export Seeds CSV / 导出种子", this);
  root->addWidget(addButton);
  root->addWidget(removeButton);
  root->addWidget(exportButton);

  model_->setHorizontalHeaderLabels({"GZH ID", "Name", "Category", "30d", "Score"});
  table_->setModel(model_);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::SingleSelection);
  table_->horizontalHeader()->setStretchLastSection(true);
  root->addWidget(table_);

  connect(addButton, &QPushButton::clicked, this, &SeedManagerWidget::emitAddSeed);
  connect(removeButton, &QPushButton::clicked, this, &SeedManagerWidget::emitRemoveSeed);
  connect(exportButton, &QPushButton::clicked, this, &SeedManagerWidget::exportSeedsRequested);
}

void SeedManagerWidget::setSeeds(const QVector<SeedRecord>& seeds) {
  model_->removeRows(0, model_->rowCount());
  for (const auto& seed : seeds) {
    QList<QStandardItem*> row;
    row << new QStandardItem(seed.gzhId) << new QStandardItem(seed.name) << new QStandardItem(seed.category)
        << new QStandardItem(QString::number(seed.articleCount30d))
        << new QStandardItem(QString::number(seed.accountScore, 'f', 2));
    model_->appendRow(row);
  }
}

QString SeedManagerWidget::selectedGzhId() const {
  if (table_->selectionModel() == nullptr || !table_->selectionModel()->hasSelection()) {
    return QString();
  }
  const QModelIndex index = table_->selectionModel()->selectedRows().first();
  return model_->item(index.row(), 0)->text();
}

void SeedManagerWidget::emitAddSeed() {
  emit addSeedRequested(gzhIdEdit_->text(), nameEdit_->text(), categoryCombo_->currentText());
}

void SeedManagerWidget::emitRemoveSeed() {
  emit removeSeedRequested(selectedGzhId());
}

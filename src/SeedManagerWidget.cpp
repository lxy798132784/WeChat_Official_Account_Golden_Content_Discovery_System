#include "SeedManagerWidget.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItem>
#include <QTableView>
#include <QVBoxLayout>

SeedManagerWidget::SeedManagerWidget(QWidget* parent)
    : QWidget(parent),
      gzhIdLabel_(new QLabel(this)),
      nameLabel_(new QLabel(this)),
      categoryLabel_(new QLabel(this)),
      gzhIdEdit_(new QLineEdit(this)),
      nameEdit_(new QLineEdit(this)),
      categoryCombo_(new QComboBox(this)),
      addButton_(new QPushButton(this)),
      removeButton_(new QPushButton(this)),
      exportButton_(new QPushButton(this)),
      table_(new QTableView(this)),
      model_(new QStandardItemModel(this)) {
  auto* root = new QVBoxLayout(this);
  auto* form = new QFormLayout();
  form->addRow(gzhIdLabel_, gzhIdEdit_);
  form->addRow(nameLabel_, nameEdit_);
  form->addRow(categoryLabel_, categoryCombo_);
  root->addLayout(form);

  root->addWidget(addButton_);
  root->addWidget(removeButton_);
  root->addWidget(exportButton_);

  applyHeaders();
  table_->setModel(model_);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::SingleSelection);
  table_->horizontalHeader()->setStretchLastSection(true);
  root->addWidget(table_);

  connect(addButton_, &QPushButton::clicked, this, &SeedManagerWidget::emitAddSeed);
  connect(removeButton_, &QPushButton::clicked, this, &SeedManagerWidget::emitRemoveSeed);
  connect(exportButton_, &QPushButton::clicked, this, &SeedManagerWidget::exportSeedsRequested);
  setLanguage(language_);
}

void SeedManagerWidget::applyHeaders() {
  model_->setHorizontalHeaderLabels({UiText::text("seed.gzh", language_),
                                     UiText::text("seed.name", language_),
                                     UiText::text("seed.category", language_),
                                     UiText::text("seed.count30d", language_),
                                     UiText::text("seed.score", language_)});
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
  applyHeaders();
}

QString SeedManagerWidget::selectedGzhId() const {
  if (table_->selectionModel() == nullptr || !table_->selectionModel()->hasSelection()) {
    return QString();
  }
  const QModelIndex index = table_->selectionModel()->selectedRows().first();
  return model_->item(index.row(), 0)->text();
}

void SeedManagerWidget::rebuildCategories() {
  const QString selected = categoryCombo_->currentData().toString();
  categoryCombo_->blockSignals(true);
  categoryCombo_->clear();
  categoryCombo_->addItem(UiText::text("filter.technology", language_), QStringLiteral("Technology"));
  categoryCombo_->addItem(UiText::text("filter.finance", language_), QStringLiteral("Finance"));
  categoryCombo_->addItem(QStringLiteral("Media"), QStringLiteral("Media"));
  categoryCombo_->addItem(UiText::text("filter.lifestyle", language_), QStringLiteral("Lifestyle"));
  categoryCombo_->addItem(language_ == UiLanguage::Chinese ? QStringLiteral("其他") : QStringLiteral("Other"), QStringLiteral("Other"));
  const int index = categoryCombo_->findData(selected.isEmpty() ? QStringLiteral("Technology") : selected);
  categoryCombo_->setCurrentIndex(index >= 0 ? index : 0);
  categoryCombo_->blockSignals(false);
}

void SeedManagerWidget::setLanguage(UiLanguage language) {
  language_ = language;
  gzhIdLabel_->setText(UiText::text("seed.gzh", language_));
  nameLabel_->setText(UiText::text("seed.name", language_));
  categoryLabel_->setText(UiText::text("seed.category", language_));
  gzhIdEdit_->setPlaceholderText(UiText::text("seed.placeholder.gzh", language_));
  nameEdit_->setPlaceholderText(UiText::text("seed.placeholder.name", language_));
  addButton_->setText(UiText::text("seed.add", language_));
  removeButton_->setText(UiText::text("seed.remove", language_));
  exportButton_->setText(UiText::text("seed.export", language_));
  gzhIdEdit_->setToolTip(UiText::text("tip.seed.gzh", language_));
  nameEdit_->setToolTip(UiText::text("tip.seed.name", language_));
  categoryCombo_->setToolTip(UiText::text("tip.seed.category", language_));
  addButton_->setToolTip(UiText::text("tip.seed.add", language_));
  removeButton_->setToolTip(UiText::text("tip.seed.remove", language_));
  exportButton_->setToolTip(UiText::text("tip.seed.export", language_));
  table_->setToolTip(UiText::text("tip.seed.table", language_));
  rebuildCategories();
  applyHeaders();
}

void SeedManagerWidget::emitAddSeed() {
  emit addSeedRequested(gzhIdEdit_->text(), nameEdit_->text(), categoryCombo_->currentData().toString());
}

void SeedManagerWidget::emitRemoveSeed() {
  emit removeSeedRequested(selectedGzhId());
}

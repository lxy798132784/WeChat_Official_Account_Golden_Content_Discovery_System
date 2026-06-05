#pragma once

#include <QStandardItemModel>
#include <QWidget>

#include "content_record.h"

class QLineEdit;
class QComboBox;
class QTableView;

/**
 * @brief 种子池管理组件 / Seed pool manager widget
 */
class SeedManagerWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit SeedManagerWidget(QWidget* parent = nullptr);
  void setSeeds(const QVector<SeedRecord>& seeds);
  QString selectedGzhId() const;

 signals:
  void addSeedRequested(const QString& gzhId, const QString& name, const QString& category);
  void removeSeedRequested(const QString& gzhId);
  void exportSeedsRequested();

 private slots:
  void emitAddSeed();
  void emitRemoveSeed();

 private:
  QLineEdit* gzhIdEdit_ = nullptr;
  QLineEdit* nameEdit_ = nullptr;
  QComboBox* categoryCombo_ = nullptr;
  QTableView* table_ = nullptr;
  QStandardItemModel* model_ = nullptr;
};

#pragma once

#include <QStandardItemModel>
#include <QWidget>

#include "UiText.h"
#include "content_record.h"

class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
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
  void setLanguage(UiLanguage language);

 signals:
  void addSeedRequested(const QString& gzhId, const QString& name, const QString& category);
  void removeSeedRequested(const QString& gzhId);
  void exportSeedsRequested();

 private slots:
  void emitAddSeed();
  void emitRemoveSeed();

 private:
  void applyHeaders();
  void rebuildCategories();

  UiLanguage language_ = UiLanguage::English;
  QLabel* gzhIdLabel_ = nullptr;
  QLabel* nameLabel_ = nullptr;
  QLabel* categoryLabel_ = nullptr;
  QLineEdit* gzhIdEdit_ = nullptr;
  QLineEdit* nameEdit_ = nullptr;
  QComboBox* categoryCombo_ = nullptr;
  QPushButton* addButton_ = nullptr;
  QPushButton* removeButton_ = nullptr;
  QPushButton* exportButton_ = nullptr;
  QTableView* table_ = nullptr;
  QStandardItemModel* model_ = nullptr;
};

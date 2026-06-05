#pragma once

#include <QStandardItemModel>
#include <QWidget>

#include "PremiumContentFilterProxyModel.h"
#include "content_record.h"

class QTableView;

/**
 * @brief 数据查看组件 / Data viewer widget
 *
 * @details 将内容记录装入标准模型并交给高价值评分代理模型展示。
 * Loads content records into a standard model and displays them through the scoring proxy.
 */
class DataViewerWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit DataViewerWidget(QWidget* parent = nullptr);
  void setRecords(const QVector<ContentRecord>& records);
  PremiumContentFilterProxyModel* proxy();

 private:
  QStandardItemModel* model_ = nullptr;
  PremiumContentFilterProxyModel* proxy_ = nullptr;
  QTableView* table_ = nullptr;
};

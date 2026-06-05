#pragma once

#include <QSortFilterProxyModel>

/**
 * @brief 高价值内容评分代理模型 / Premium content scoring proxy model
 *
 * @details 在内存中计算互动率、评论密度和发文频率分，避免 UI 排序筛选频繁读盘。
 * Computes engagement, comment density, and frequency scores in memory for fast UI filtering.
 */
class PremiumContentFilterProxyModel final : public QSortFilterProxyModel {
  Q_OBJECT
 public:
  enum SourceColumns {
    kTitle = 0,
    kAccount,
    kCategory,
    kPublishDate,
    kRead,
    kLike,
    kOldLike,
    kComment,
    kFrequency,
    kScore,
    kUrl,
    kColumnCount
  };
  explicit PremiumContentFilterProxyModel(QObject* parent = nullptr);
  void setWeights(double engagementWeight, double commentWeight, double frequencyWeight);
  void setMinimums(int minimumReadThreshold, double minimumScore);
  double scoreForSourceRow(int sourceRow) const;

 protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
  QVariant data(const QModelIndex& index, int role) const override;

 private:
  double engagementWeight_ = 0.55;
  double commentWeight_ = 0.30;
  double frequencyWeight_ = 0.15;
  int minimumReadThreshold_ = 1000;
  double minimumScore_ = 1.0;
};

#include "PremiumContentFilterProxyModel.h"

#include <QBrush>
#include <QColor>

PremiumContentFilterProxyModel::PremiumContentFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {}

void PremiumContentFilterProxyModel::setWeights(double engagementWeight, double commentWeight, double frequencyWeight) {
  engagementWeight_ = engagementWeight;
  commentWeight_ = commentWeight;
  frequencyWeight_ = frequencyWeight;
  invalidateFilter();
}

void PremiumContentFilterProxyModel::setMinimums(int minimumReadThreshold, double minimumScore) {
  minimumReadThreshold_ = minimumReadThreshold;
  minimumScore_ = minimumScore;
  invalidateFilter();
}

double PremiumContentFilterProxyModel::scoreForSourceRow(int sourceRow) const {
  const auto model = sourceModel();
  if (model == nullptr) return 0.0;
  const int read = model->index(sourceRow, kRead).data().toInt();
  if (read <= 0) return 0.0;
  const int like = model->index(sourceRow, kLike).data().toInt();
  const int oldLike = model->index(sourceRow, kOldLike).data().toInt();
  const int comment = model->index(sourceRow, kComment).data().toInt();
  const int frequency = model->index(sourceRow, kFrequency).data().toInt();
  const double engagementRate = static_cast<double>(like + oldLike) / static_cast<double>(read);
  const double commentDensity = static_cast<double>(comment) / static_cast<double>(read) * 10000.0;
  const double frequencyScore = qMin(100.0, static_cast<double>(frequency) / 30.0 * 100.0);
  return engagementWeight_ * engagementRate * 100.0 + commentWeight_ * commentDensity + frequencyWeight_ * frequencyScore;
}

bool PremiumContentFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
  const auto model = sourceModel();
  if (model == nullptr) return false;
  const int read = model->index(sourceRow, kRead, sourceParent).data().toInt();
  return read >= minimumReadThreshold_ && scoreForSourceRow(sourceRow) >= minimumScore_;
}

bool PremiumContentFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
  if (left.column() == kScore || right.column() == kScore) {
    return scoreForSourceRow(left.row()) < scoreForSourceRow(right.row());
  }
  return QSortFilterProxyModel::lessThan(left, right);
}

QVariant PremiumContentFilterProxyModel::data(const QModelIndex& index, int role) const {
  if (role == Qt::BackgroundRole && index.isValid()) {
    const double score = scoreForSourceRow(mapToSource(index).row());
    if (score >= 85.0) return QBrush(QColor(63, 185, 80, 32));
  }
  if (role == Qt::DisplayRole && index.column() == kScore) {
    return QString::number(scoreForSourceRow(mapToSource(index).row()), 'f', 2);
  }
  return QSortFilterProxyModel::data(index, role);
}

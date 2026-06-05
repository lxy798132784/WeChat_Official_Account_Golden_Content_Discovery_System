#pragma once

#include <QWidget>

#include "AutoIngestionController.h"
#include "UiText.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTextEdit;

class AutoIngestionWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit AutoIngestionWidget(QWidget* parent = nullptr);

  void setLanguage(UiLanguage language);
  void setQueue(const QVector<AutoIngestionTask>& tasks);
  void setRunning(bool running);
  bool automationEnabled() const;
  int intervalSeconds() const;
  int maxAttempts() const;
  QString urlsText() const;

 signals:
  void addUrlsRequested(const QString& text);
  void startRequested();
  void stopRequested();
  void runNextRequested();
  void clearCompletedRequested();
  void clearAllRequested();
  void saveQueueRequested();
  void loadQueueRequested();

 private:
  UiLanguage language_ = UiLanguage::English;
  QLabel* enableLabel_ = nullptr;
  QLabel* intervalLabel_ = nullptr;
  QLabel* attemptsLabel_ = nullptr;
  QLabel* urlsLabel_ = nullptr;
  QLabel* queueLabel_ = nullptr;
  QCheckBox* enableCheckBox_ = nullptr;
  QSpinBox* intervalSpinBox_ = nullptr;
  QSpinBox* attemptsSpinBox_ = nullptr;
  QTextEdit* urlsEdit_ = nullptr;
  QPushButton* addUrlsButton_ = nullptr;
  QPushButton* startButton_ = nullptr;
  QPushButton* stopButton_ = nullptr;
  QPushButton* runNextButton_ = nullptr;
  QPushButton* clearCompletedButton_ = nullptr;
  QPushButton* clearAllButton_ = nullptr;
  QPushButton* saveQueueButton_ = nullptr;
  QPushButton* loadQueueButton_ = nullptr;
  QTableWidget* queueTable_ = nullptr;
};

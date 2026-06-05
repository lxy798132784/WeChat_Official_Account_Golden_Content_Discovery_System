#pragma once

#include <QMutex>
#include <QQueue>
#include <QString>
#include <QThread>

/** ADB automation engine. Disabled by default in production settings. */
class AdbAutomationEngine final : public QThread {
  Q_OBJECT

 public:
  explicit AdbAutomationEngine(QObject* parent = nullptr);
  void enqueueSeed(const QString& seed);
  void requestStop();

 signals:
  void automationLog(const QString& message);

 protected:
  void run() override;

 private:
  QMutex mutex_;
  QQueue<QString> seeds_;
  bool stopRequested_ = false;
};

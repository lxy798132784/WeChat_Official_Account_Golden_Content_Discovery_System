#pragma once
#include <QMutex>
#include <QQueue>
#include <QThread>
#include <QString>
/** @brief ADB 自动化引擎 / ADB automation engine */
class AdbAutomationEngine final : public QThread { Q_OBJECT
 public: explicit AdbAutomationEngine(QObject* parent=nullptr); void enqueueSeed(const QString& seed); void requestStop();
 signals: void automationLog(const QString& message);
 protected: void run() override;
 private: QMutex mutex_; QQueue<QString> seeds_; bool stopRequested_=false; };

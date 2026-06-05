#pragma once

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QString>
#include <QTimer>
#include <QVector>

struct AutoIngestionTask {
  QString url;
  QString accountName;
  QString category;
  QString status = QStringLiteral("pending");
  int attempts = 0;
  QDateTime lastAttempt;
  QString lastError;
};

/**
 * Controlled auto-ingestion scheduler.
 *
 * It does not scrape WeChat by itself and does not store credentials. It only opens
 * operator-provided article URLs on a configured Android test device through ADB;
 * a local proxy adapter then captures allowed metric endpoints and sends compact
 * JSON to the existing localhost bridge.
 */
class AutoIngestionController final : public QObject {
  Q_OBJECT
 public:
  explicit AutoIngestionController(QObject* parent = nullptr);

  void setIntervalSeconds(int seconds);
  void setMaxAttempts(int attempts);
  void setEnabled(bool enabled);
  bool isRunning() const;
  QVector<AutoIngestionTask> tasks() const;
  int pendingCount() const;

  bool enqueueUrl(const QString& url, const QString& accountName = QString(),
                  const QString& category = QString(), QString* errorMessage = nullptr);
  int enqueueUrlsFromText(const QString& text, QString* errorMessage = nullptr);
  void clearCompleted();
  void clearAll();

  bool saveQueue(const QString& path, QString* errorMessage = nullptr) const;
  bool loadQueue(const QString& path, QString* errorMessage = nullptr);

  static bool isSupportedArticleUrl(const QString& url);
  static QStringList adbOpenUrlArguments(const QString& url);
  static bool hasConnectedAdbDevice(QString* errorMessage = nullptr);

 public slots:
  void start();
  void stop();
  void runNextNow();

 signals:
  void logMessage(const QString& message);
  void queueChanged();

 private slots:
  void onTick();

 private:
  int nextRunnableIndex() const;
  void openTask(int index);
  bool openUrlWithAdb(const QString& url, QString* errorMessage) const;

  QVector<AutoIngestionTask> tasks_;
  QTimer timer_;
  int intervalSeconds_ = 30;
  int maxAttempts_ = 3;
  bool enabled_ = false;
};

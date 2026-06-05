#include "AutoIngestionController.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTextStream>
#include <QUrl>

namespace {
constexpr int kMinIntervalSeconds = 5;
constexpr int kMaxIntervalSeconds = 3600;

QString normalizedUrl(const QString& raw) {
  const QString trimmed = raw.trimmed();
  if (trimmed.isEmpty()) {
    return QString();
  }
  QUrl url(trimmed);
  if (!url.isValid() || url.scheme().isEmpty()) {
    return QString();
  }
  return url.toString(QUrl::FullyEncoded);
}

QJsonObject taskToJson(const AutoIngestionTask& task) {
  QJsonObject object;
  object.insert(QStringLiteral("url"), task.url);
  object.insert(QStringLiteral("account_name"), task.accountName);
  object.insert(QStringLiteral("category"), task.category);
  object.insert(QStringLiteral("status"), task.status);
  object.insert(QStringLiteral("attempts"), task.attempts);
  object.insert(QStringLiteral("last_attempt"), task.lastAttempt.isValid() ? task.lastAttempt.toString(Qt::ISODate) : QString());
  object.insert(QStringLiteral("last_error"), task.lastError);
  return object;
}

AutoIngestionTask taskFromJson(const QJsonObject& object) {
  AutoIngestionTask task;
  task.url = object.value(QStringLiteral("url")).toString();
  task.accountName = object.value(QStringLiteral("account_name")).toString();
  task.category = object.value(QStringLiteral("category")).toString();
  task.status = object.value(QStringLiteral("status")).toString(QStringLiteral("pending"));
  task.attempts = object.value(QStringLiteral("attempts")).toInt(0);
  task.lastAttempt = QDateTime::fromString(object.value(QStringLiteral("last_attempt")).toString(), Qt::ISODate);
  task.lastError = object.value(QStringLiteral("last_error")).toString();
  return task;
}
}  // namespace

AutoIngestionController::AutoIngestionController(QObject* parent) : QObject(parent) {
  timer_.setSingleShot(false);
  connect(&timer_, &QTimer::timeout, this, &AutoIngestionController::onTick);
}

void AutoIngestionController::setIntervalSeconds(int seconds) {
  intervalSeconds_ = qBound(kMinIntervalSeconds, seconds, kMaxIntervalSeconds);
  if (timer_.isActive()) {
    timer_.start(intervalSeconds_ * 1000);
  }
}

void AutoIngestionController::setMaxAttempts(int attempts) {
  maxAttempts_ = qBound(1, attempts, 20);
}

void AutoIngestionController::setEnabled(bool enabled) {
  enabled_ = enabled;
}

bool AutoIngestionController::isRunning() const {
  return timer_.isActive();
}

QVector<AutoIngestionTask> AutoIngestionController::tasks() const {
  return tasks_;
}

int AutoIngestionController::pendingCount() const {
  int count = 0;
  for (const AutoIngestionTask& task : tasks_) {
    if (task.status == QStringLiteral("pending") || task.status == QStringLiteral("failed")) {
      ++count;
    }
  }
  return count;
}

bool AutoIngestionController::isSupportedArticleUrl(const QString& urlText) {
  const QUrl url(urlText);
  if (!url.isValid() || url.scheme().isEmpty() || url.host().isEmpty()) {
    return false;
  }
  const QString host = url.host().toLower();
  if (host == QStringLiteral("mp.weixin.qq.com")) {
    return true;
  }
  if (host.endsWith(QStringLiteral("weixin.qq.com"))) {
    return true;
  }
  return urlText.contains(QStringLiteral("__biz=")) || urlText.contains(QStringLiteral("mid="));
}

bool AutoIngestionController::enqueueUrl(const QString& url, const QString& accountName,
                                         const QString& category, QString* errorMessage) {
  const QString normalized = normalizedUrl(url);
  if (normalized.isEmpty() || !isSupportedArticleUrl(normalized)) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("只接受有效的微信文章链接");
    }
    return false;
  }
  for (const AutoIngestionTask& task : tasks_) {
    if (task.url == normalized) {
      if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("链接已在队列中");
      }
      return false;
    }
  }
  AutoIngestionTask task;
  task.url = normalized;
  task.accountName = accountName.trimmed();
  task.category = category.trimmed();
  tasks_.push_back(task);
  emit queueChanged();
  return true;
}

int AutoIngestionController::enqueueUrlsFromText(const QString& text, QString* errorMessage) {
  int added = 0;
  QStringList rejected;
  const QStringList lines = text.split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
  for (const QString& line : lines) {
    QString localError;
    if (enqueueUrl(line.trimmed(), QString(), QString(), &localError)) {
      ++added;
    } else {
      rejected.push_back(QStringLiteral("%1 (%2)").arg(line.trimmed(), localError));
    }
  }
  if (errorMessage != nullptr) {
    *errorMessage = rejected.join(QStringLiteral("; "));
  }
  return added;
}

void AutoIngestionController::clearCompleted() {
  QVector<AutoIngestionTask> kept;
  for (const AutoIngestionTask& task : tasks_) {
    if (task.status != QStringLiteral("opened") && task.status != QStringLiteral("done")) {
      kept.push_back(task);
    }
  }
  tasks_ = kept;
  emit queueChanged();
}

void AutoIngestionController::clearAll() {
  tasks_.clear();
  emit queueChanged();
}

bool AutoIngestionController::saveQueue(const QString& path, QString* errorMessage) const {
  QJsonArray array;
  for (const AutoIngestionTask& task : tasks_) {
    array.push_back(taskToJson(task));
  }
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    if (errorMessage != nullptr) {
      *errorMessage = file.errorString();
    }
    return false;
  }
  file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
  return true;
}

bool AutoIngestionController::loadQueue(const QString& path, QString* errorMessage) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    if (errorMessage != nullptr) {
      *errorMessage = file.errorString();
    }
    return false;
  }
  const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
  if (!document.isArray()) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("队列文件必须是数组格式");
    }
    return false;
  }
  QVector<AutoIngestionTask> loaded;
  for (const QJsonValue& value : document.array()) {
    if (!value.isObject()) {
      continue;
    }
    const AutoIngestionTask task = taskFromJson(value.toObject());
    if (isSupportedArticleUrl(task.url)) {
      loaded.push_back(task);
    }
  }
  tasks_ = loaded;
  emit queueChanged();
  return true;
}

QStringList AutoIngestionController::adbOpenUrlArguments(const QString& url) {
  return {QStringLiteral("shell"), QStringLiteral("am"), QStringLiteral("start"),
          QStringLiteral("-a"), QStringLiteral("android.intent.action.VIEW"),
          QStringLiteral("-d"), url};
}

bool AutoIngestionController::hasConnectedAdbDevice(QString* errorMessage) {
  QProcess process;
  process.start(QStringLiteral("adb"), {QStringLiteral("devices")});
  if (!process.waitForStarted(3000)) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("手机调试工具未启动，请先安装。");
    }
    return false;
  }
  if (!process.waitForFinished(10000)) {
    process.kill();
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("手机设备检测超时。");
    }
    return false;
  }
  const QString output = QString::fromUtf8(process.readAllStandardOutput()) +
                         QString::fromUtf8(process.readAllStandardError());
  if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
    if (errorMessage != nullptr) {
      *errorMessage = output.trimmed().isEmpty() ? QStringLiteral("手机设备检测失败") : output.trimmed();
    }
    return false;
  }
  const QStringList lines = output.split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
  for (const QString& line : lines) {
    if (line.contains(QStringLiteral("\tdevice"))) {
      return true;
    }
  }
  if (errorMessage != nullptr) {
    *errorMessage = QStringLiteral("未发现已授权安卓设备。请连接手机并允许 USB 调试。");
  }
  return false;
}

void AutoIngestionController::start() {
  if (!enabled_) {
    emit logMessage(QStringLiteral("自动采集需要先明确打开手机调试自动化开关"));
    return;
  }
  if (timer_.isActive()) {
    return;
  }
  timer_.start(intervalSeconds_ * 1000);
  emit logMessage(QStringLiteral("自动采集调度已启动"));
  onTick();
}

void AutoIngestionController::stop() {
  if (timer_.isActive()) {
    timer_.stop();
  }
  emit logMessage(QStringLiteral("自动采集调度已停止"));
}

void AutoIngestionController::runNextNow() {
  onTick();
}

void AutoIngestionController::onTick() {
  if (!enabled_) {
    stop();
    return;
  }
  const int index = nextRunnableIndex();
  if (index < 0) {
    emit logMessage(QStringLiteral("自动采集队列没有可执行任务"));
    return;
  }
  openTask(index);
}

int AutoIngestionController::nextRunnableIndex() const {
  for (int index = 0; index < tasks_.size(); ++index) {
    const AutoIngestionTask& task = tasks_.at(index);
    if ((task.status == QStringLiteral("pending") || task.status == QStringLiteral("failed")) &&
        task.attempts < maxAttempts_) {
      return index;
    }
  }
  return -1;
}

void AutoIngestionController::openTask(int index) {
  if (index < 0 || index >= tasks_.size()) {
    return;
  }
  AutoIngestionTask& task = tasks_[index];
  task.status = QStringLiteral("opening");
  task.attempts += 1;
  task.lastAttempt = QDateTime::currentDateTimeUtc();
  task.lastError.clear();
  emit queueChanged();

  QString error;
  if (openUrlWithAdb(task.url, &error)) {
    task.status = QStringLiteral("opened");
    emit logMessage(QStringLiteral("已通过手机调试打开文章：%1").arg(task.url));
  } else {
    task.status = QStringLiteral("failed");
    task.lastError = error;
    emit logMessage(QStringLiteral("手机调试打开失败：%1").arg(error));
  }
  emit queueChanged();
}

bool AutoIngestionController::openUrlWithAdb(const QString& url, QString* errorMessage) const {
  QProcess process;
  process.start(QStringLiteral("adb"), adbOpenUrlArguments(url));
  if (!process.waitForStarted(3000)) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("adb did not start. Install adb and connect a test phone.");
    }
    return false;
  }
  if (!process.waitForFinished(10000)) {
    process.kill();
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("adb timed out while opening the article URL");
    }
    return false;
  }
  const QString output = QString::fromUtf8(process.readAllStandardOutput()) +
                         QString::fromUtf8(process.readAllStandardError());
  if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
    if (errorMessage != nullptr) {
      *errorMessage = output.trimmed().isEmpty() ? QStringLiteral("adb returned a non-zero exit code") : output.trimmed();
    }
    return false;
  }
  return true;
}

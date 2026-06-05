#include "AutoIngestionWidget.h"

#include <QCheckBox>
#include <QDateTime>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>

#include "UiText.h"

namespace {
QString localizedTaskStatus(const QString& status, UiLanguage language) {
  if (language != UiLanguage::Chinese) return status;
  if (status == QStringLiteral("pending")) return QStringLiteral("待处理");
  if (status == QStringLiteral("opening")) return QStringLiteral("打开中");
  if (status == QStringLiteral("opened")) return QStringLiteral("已打开");
  if (status == QStringLiteral("done")) return QStringLiteral("已完成");
  if (status == QStringLiteral("failed")) return QStringLiteral("失败");
  return status;
}

QString localizedTaskError(const QString& error, UiLanguage language) {
  if (language != UiLanguage::Chinese) return error;
  QString text = error;
  text.replace(QStringLiteral("Only valid WeChat article URLs are accepted"), QStringLiteral("只接受有效的微信文章链接"));
  text.replace(QStringLiteral("URL already exists in the queue"), QStringLiteral("链接已在队列中"));
  text.replace(QStringLiteral("Queue file must be a JSON array"), QStringLiteral("队列文件必须是数组格式"));
  text.replace(QStringLiteral("adb did not start. Install adb first."), QStringLiteral("手机调试工具未启动，请先安装。"));
  text.replace(QStringLiteral("adb devices timed out."), QStringLiteral("手机设备检测超时。"));
  text.replace(QStringLiteral("adb devices failed"), QStringLiteral("手机设备检测失败"));
  text.replace(QStringLiteral("No authorized Android device found. Connect the phone and allow USB debugging."), QStringLiteral("未发现已授权安卓设备。请连接手机并允许 USB 调试。"));
  text.replace(QStringLiteral("adb start failed"), QStringLiteral("手机调试命令启动失败"));
  text.replace(QStringLiteral("adb command timed out"), QStringLiteral("手机调试命令超时"));
  return text;
}
}  // namespace

AutoIngestionWidget::AutoIngestionWidget(QWidget* parent)
    : QWidget(parent),
      enableLabel_(new QLabel(this)),
      intervalLabel_(new QLabel(this)),
      attemptsLabel_(new QLabel(this)),
      urlsLabel_(new QLabel(this)),
      queueLabel_(new QLabel(this)),
      enableCheckBox_(new QCheckBox(this)),
      intervalSpinBox_(new QSpinBox(this)),
      attemptsSpinBox_(new QSpinBox(this)),
      urlsEdit_(new QTextEdit(this)),
      addUrlsButton_(new QPushButton(this)),
      startButton_(new QPushButton(this)),
      stopButton_(new QPushButton(this)),
      runNextButton_(new QPushButton(this)),
      clearCompletedButton_(new QPushButton(this)),
      clearAllButton_(new QPushButton(this)),
      saveQueueButton_(new QPushButton(this)),
      loadQueueButton_(new QPushButton(this)),
      queueTable_(new QTableWidget(this)) {
  auto* rootLayout = new QVBoxLayout(this);
  auto* formLayout = new QFormLayout();
  intervalSpinBox_->setRange(5, 3600);
  intervalSpinBox_->setValue(30);
  attemptsSpinBox_->setRange(1, 20);
  attemptsSpinBox_->setValue(3);
  urlsEdit_->setMinimumHeight(110);
  urlsEdit_->setAcceptRichText(false);

  formLayout->addRow(enableLabel_, enableCheckBox_);
  formLayout->addRow(intervalLabel_, intervalSpinBox_);
  formLayout->addRow(attemptsLabel_, attemptsSpinBox_);
  rootLayout->addLayout(formLayout);
  rootLayout->addWidget(urlsLabel_);
  rootLayout->addWidget(urlsEdit_);

  auto* actionRow1 = new QHBoxLayout();
  actionRow1->addWidget(addUrlsButton_);
  actionRow1->addWidget(startButton_);
  actionRow1->addWidget(stopButton_);
  actionRow1->addWidget(runNextButton_);
  rootLayout->addLayout(actionRow1);

  auto* actionRow2 = new QHBoxLayout();
  actionRow2->addWidget(clearCompletedButton_);
  actionRow2->addWidget(clearAllButton_);
  actionRow2->addWidget(saveQueueButton_);
  actionRow2->addWidget(loadQueueButton_);
  rootLayout->addLayout(actionRow2);

  rootLayout->addWidget(queueLabel_);
  queueTable_->setColumnCount(6);
  queueTable_->horizontalHeader()->setStretchLastSection(true);
  queueTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
  queueTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  queueTable_->setAlternatingRowColors(true);
  queueTable_->setMinimumHeight(180);
  rootLayout->addWidget(queueTable_);

  connect(addUrlsButton_, &QPushButton::clicked, this, [this]() { emit addUrlsRequested(urlsText()); });
  connect(startButton_, &QPushButton::clicked, this, &AutoIngestionWidget::startRequested);
  connect(stopButton_, &QPushButton::clicked, this, &AutoIngestionWidget::stopRequested);
  connect(runNextButton_, &QPushButton::clicked, this, &AutoIngestionWidget::runNextRequested);
  connect(clearCompletedButton_, &QPushButton::clicked, this, &AutoIngestionWidget::clearCompletedRequested);
  connect(clearAllButton_, &QPushButton::clicked, this, &AutoIngestionWidget::clearAllRequested);
  connect(saveQueueButton_, &QPushButton::clicked, this, &AutoIngestionWidget::saveQueueRequested);
  connect(loadQueueButton_, &QPushButton::clicked, this, &AutoIngestionWidget::loadQueueRequested);

  setLanguage(language_);
  setRunning(false);
}

void AutoIngestionWidget::setLanguage(UiLanguage language) {
  language_ = language;
  enableLabel_->setText(UiText::text(QStringLiteral("auto.enable"), language_));
  intervalLabel_->setText(UiText::text(QStringLiteral("auto.interval"), language_));
  attemptsLabel_->setText(UiText::text(QStringLiteral("auto.attempts"), language_));
  urlsLabel_->setText(UiText::text(QStringLiteral("auto.urls"), language_));
  queueLabel_->setText(UiText::text(QStringLiteral("auto.queue"), language_));
  enableCheckBox_->setText(UiText::text(QStringLiteral("auto.enable_checkbox"), language_));
  addUrlsButton_->setText(UiText::text(QStringLiteral("auto.add_urls"), language_));
  startButton_->setText(UiText::text(QStringLiteral("auto.start"), language_));
  stopButton_->setText(UiText::text(QStringLiteral("auto.stop"), language_));
  runNextButton_->setText(UiText::text(QStringLiteral("auto.run_next"), language_));
  clearCompletedButton_->setText(UiText::text(QStringLiteral("auto.clear_completed"), language_));
  clearAllButton_->setText(UiText::text(QStringLiteral("auto.clear_all"), language_));
  saveQueueButton_->setText(UiText::text(QStringLiteral("auto.save_queue"), language_));
  loadQueueButton_->setText(UiText::text(QStringLiteral("auto.load_queue"), language_));
  urlsEdit_->setPlaceholderText(UiText::text(QStringLiteral("auto.urls_placeholder"), language_));
  queueTable_->setHorizontalHeaderLabels({UiText::text(QStringLiteral("auto.col.status"), language_),
                                          UiText::text(QStringLiteral("auto.col.attempts"), language_),
                                          UiText::text(QStringLiteral("auto.col.account"), language_),
                                          UiText::text(QStringLiteral("auto.col.category"), language_),
                                          UiText::text(QStringLiteral("auto.col.last"), language_),
                                          UiText::text(QStringLiteral("auto.col.url"), language_)});
  enableCheckBox_->setToolTip(UiText::text(QStringLiteral("tip.auto.enable"), language_));
  intervalSpinBox_->setToolTip(UiText::text(QStringLiteral("tip.auto.interval"), language_));
  attemptsSpinBox_->setToolTip(UiText::text(QStringLiteral("tip.auto.attempts"), language_));
  urlsEdit_->setToolTip(UiText::text(QStringLiteral("tip.auto.urls"), language_));
  startButton_->setToolTip(UiText::text(QStringLiteral("tip.auto.start"), language_));
  runNextButton_->setToolTip(UiText::text(QStringLiteral("tip.auto.run_next"), language_));
}

void AutoIngestionWidget::setQueue(const QVector<AutoIngestionTask>& tasks) {
  queueTable_->setRowCount(tasks.size());
  for (int row = 0; row < tasks.size(); ++row) {
    const AutoIngestionTask& task = tasks.at(row);
    queueTable_->setItem(row, 0, new QTableWidgetItem(localizedTaskStatus(task.status, language_)));
    queueTable_->setItem(row, 1, new QTableWidgetItem(QString::number(task.attempts)));
    queueTable_->setItem(row, 2, new QTableWidgetItem(task.accountName));
    queueTable_->setItem(row, 3, new QTableWidgetItem(task.category));
    queueTable_->setItem(row, 4, new QTableWidgetItem(task.lastAttempt.isValid() ? task.lastAttempt.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")) : QStringLiteral("-")));
    queueTable_->setItem(row, 5, new QTableWidgetItem(task.lastError.isEmpty() ? task.url : QStringLiteral("%1 | %2").arg(task.url, localizedTaskError(task.lastError, language_))));
  }
  queueTable_->resizeColumnsToContents();
}

void AutoIngestionWidget::setRunning(bool running) {
  startButton_->setEnabled(!running);
  stopButton_->setEnabled(running);
}

bool AutoIngestionWidget::automationEnabled() const {
  return enableCheckBox_->isChecked();
}

int AutoIngestionWidget::intervalSeconds() const {
  return intervalSpinBox_->value();
}

int AutoIngestionWidget::maxAttempts() const {
  return attemptsSpinBox_->value();
}

QString AutoIngestionWidget::urlsText() const {
  return urlsEdit_->toPlainText();
}

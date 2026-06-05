#include "PhoneDiagnosticsWidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {
QString statusIcon(const QString& status) {
  if (status == QStringLiteral("pass")) return QStringLiteral("✅");
  if (status == QStringLiteral("warn")) return QStringLiteral("🟡");
  if (status == QStringLiteral("fail")) return QStringLiteral("❌");
  return QStringLiteral("⚪");
}
}  // namespace

PhoneDiagnosticsWidget::PhoneDiagnosticsWidget(QWidget* parent)
    : QWidget(parent),
      introLabel_(new QLabel(this)),
      overallLabel_(new QLabel(this)),
      serialLabel_(new QLabel(this)),
      proxyPortLabel_(new QLabel(this)),
      testUrlLabel_(new QLabel(this)),
      serialComboBox_(new QComboBox(this)),
      proxyPortSpinBox_(new QSpinBox(this)),
      openLinkCheckBox_(new QCheckBox(this)),
      testUrlEdit_(new QPlainTextEdit(this)),
      runButton_(new QPushButton(this)),
      restartAdbButton_(new QPushButton(this)),
      openLinkButton_(new QPushButton(this)),
      copyReportButton_(new QPushButton(this)),
      exportJsonButton_(new QPushButton(this)),
      table_(new QTableWidget(this)) {
  auto* root = new QVBoxLayout(this);
  introLabel_->setWordWrap(true);
  overallLabel_->setWordWrap(true);
  root->addWidget(introLabel_);
  root->addWidget(overallLabel_);

  auto* grid = new QGridLayout();
  proxyPortSpinBox_->setRange(0, 65535);
  proxyPortSpinBox_->setValue(0);
  testUrlEdit_->setMaximumHeight(52);
  testUrlEdit_->setPlainText(QStringLiteral("https://mp.weixin.qq.com/s/test"));
  grid->addWidget(serialLabel_, 0, 0);
  grid->addWidget(serialComboBox_, 0, 1);
  grid->addWidget(proxyPortLabel_, 0, 2);
  grid->addWidget(proxyPortSpinBox_, 0, 3);
  grid->addWidget(openLinkCheckBox_, 1, 0, 1, 2);
  grid->addWidget(testUrlLabel_, 1, 2);
  grid->addWidget(testUrlEdit_, 1, 3);
  root->addLayout(grid);

  auto* actions = new QGridLayout();
  actions->addWidget(runButton_, 0, 0);
  actions->addWidget(restartAdbButton_, 0, 1);
  actions->addWidget(openLinkButton_, 0, 2);
  actions->addWidget(copyReportButton_, 0, 3);
  actions->addWidget(exportJsonButton_, 0, 4);
  root->addLayout(actions);

  table_->setColumnCount(5);
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->verticalHeader()->setVisible(false);
  table_->setAlternatingRowColors(true);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  root->addWidget(table_, 1);

  connect(runButton_, &QPushButton::clicked, this, &PhoneDiagnosticsWidget::runDiagnosticsRequested);
  connect(restartAdbButton_, &QPushButton::clicked, this, &PhoneDiagnosticsWidget::restartAdbRequested);
  connect(openLinkButton_, &QPushButton::clicked, this, &PhoneDiagnosticsWidget::testOpenLinkRequested);
  connect(copyReportButton_, &QPushButton::clicked, this, &PhoneDiagnosticsWidget::copyReportRequested);
  connect(exportJsonButton_, &QPushButton::clicked, this, &PhoneDiagnosticsWidget::exportJsonRequested);

  setLanguage(language_);
}

QString PhoneDiagnosticsWidget::selectedSerial() const {
  return serialComboBox_->currentData().toString();
}

quint16 PhoneDiagnosticsWidget::proxyPort() const {
  return static_cast<quint16>(proxyPortSpinBox_->value());
}

bool PhoneDiagnosticsWidget::includeOpenLinkTest() const {
  return openLinkCheckBox_->isChecked();
}

QString PhoneDiagnosticsWidget::testUrl() const {
  return testUrlEdit_->toPlainText().trimmed();
}

void PhoneDiagnosticsWidget::setLanguage(UiLanguage language) {
  language_ = language;
  introLabel_->setText(UiText::text(QStringLiteral("phone.intro"), language_));
  serialLabel_->setText(UiText::text(QStringLiteral("phone.serial"), language_));
  proxyPortLabel_->setText(UiText::text(QStringLiteral("phone.proxy_port"), language_));
  testUrlLabel_->setText(UiText::text(QStringLiteral("phone.test_url"), language_));
  openLinkCheckBox_->setText(UiText::text(QStringLiteral("phone.include_open_test"), language_));
  runButton_->setText(UiText::text(QStringLiteral("phone.run"), language_));
  restartAdbButton_->setText(UiText::text(QStringLiteral("phone.restart_adb"), language_));
  openLinkButton_->setText(UiText::text(QStringLiteral("phone.open_link"), language_));
  copyReportButton_->setText(UiText::text(QStringLiteral("phone.copy_report"), language_));
  exportJsonButton_->setText(UiText::text(QStringLiteral("phone.export_json"), language_));
  table_->setHorizontalHeaderLabels({UiText::text(QStringLiteral("phone.col.status"), language_),
                                     UiText::text(QStringLiteral("phone.col.item"), language_),
                                     UiText::text(QStringLiteral("phone.col.details"), language_),
                                     UiText::text(QStringLiteral("phone.col.fix"), language_),
                                     UiText::text(QStringLiteral("phone.col.raw"), language_)});
  runButton_->setToolTip(UiText::text(QStringLiteral("tip.phone.run"), language_));
  restartAdbButton_->setToolTip(UiText::text(QStringLiteral("tip.phone.restart_adb"), language_));
  openLinkButton_->setToolTip(UiText::text(QStringLiteral("tip.phone.open_link"), language_));
  copyReportButton_->setToolTip(UiText::text(QStringLiteral("tip.phone.copy_report"), language_));
  exportJsonButton_->setToolTip(UiText::text(QStringLiteral("tip.phone.export_json"), language_));
  setReport(currentReport_);
}

void PhoneDiagnosticsWidget::setReport(const PhoneDiagnosticReport& report) {
  currentReport_ = report;
  QString overallText;
  if (language_ == UiLanguage::Chinese) {
    overallText = QStringLiteral("总体状态：%1；目标设备：%2；平台：%3")
                      .arg(report.overallStatus, report.targetSerial.isEmpty() ? QStringLiteral("未选择") : report.targetSerial, report.platform);
  } else {
    overallText = QStringLiteral("Overall: %1; Target: %2; Platform: %3")
                      .arg(report.overallStatus, report.targetSerial.isEmpty() ? QStringLiteral("not selected") : report.targetSerial, report.platform);
  }
  overallLabel_->setText(overallText);

  const QString previous = selectedSerial();
  serialComboBox_->clear();
  serialComboBox_->addItem(language_ == UiLanguage::Chinese ? QStringLiteral("自动选择授权设备") : QStringLiteral("Auto-select authorized device"), QString());
  int selectedIndex = 0;
  for (const PhoneDeviceInfo& device : report.devices) {
    const QString label = PhoneDiagnosticsController::deviceSummary(device);
    serialComboBox_->addItem(label, device.serial);
    if ((!previous.isEmpty() && previous == device.serial) || (previous.isEmpty() && report.targetSerial == device.serial)) {
      selectedIndex = serialComboBox_->count() - 1;
    }
  }
  serialComboBox_->setCurrentIndex(selectedIndex);

  table_->setRowCount(report.items.size());
  for (int row = 0; row < report.items.size(); ++row) {
    const PhoneDiagnosticItem& item = report.items.at(row);
    table_->setItem(row, 0, new QTableWidgetItem(statusIcon(item.status) + QStringLiteral(" ") + item.status));
    table_->setItem(row, 1, new QTableWidgetItem(item.label));
    table_->setItem(row, 2, new QTableWidgetItem(item.details));
    table_->setItem(row, 3, new QTableWidgetItem(item.fixHint));
    table_->setItem(row, 4, new QTableWidgetItem(item.rawOutput.left(800)));
  }
  table_->resizeColumnsToContents();
}

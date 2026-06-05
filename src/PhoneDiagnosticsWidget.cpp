#include "PhoneDiagnosticsWidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QHash>
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

QString localizedStatus(const QString& status, UiLanguage language) {
  if (language != UiLanguage::Chinese) {
    return status;
  }
  if (status == QStringLiteral("pass")) return QStringLiteral("通过");
  if (status == QStringLiteral("warn")) return QStringLiteral("提醒");
  if (status == QStringLiteral("fail")) return QStringLiteral("失败");
  if (status == QStringLiteral("ready")) return QStringLiteral("就绪");
  if (status == QStringLiteral("warning")) return QStringLiteral("有提醒");
  if (status == QStringLiteral("blocked")) return QStringLiteral("阻塞");
  return status;
}

QString localizedDeviceState(const QString& state, UiLanguage language) {
  if (language != UiLanguage::Chinese) return state;
  if (state == QStringLiteral("device")) return QStringLiteral("已授权");
  if (state == QStringLiteral("unauthorized")) return QStringLiteral("未授权");
  if (state == QStringLiteral("offline")) return QStringLiteral("离线");
  return state;
}

QString localizedItemLabel(const PhoneDiagnosticItem& item, UiLanguage language) {
  if (language != UiLanguage::Chinese) return item.label;
  static const QHash<QString, QString> labels = {
      {QStringLiteral("adb_tool"), QStringLiteral("安卓调试工具")},
      {QStringLiteral("adb_server"), QStringLiteral("安卓调试服务")},
      {QStringLiteral("device_detected"), QStringLiteral("手机检测")},
      {QStringLiteral("usb_authorization"), QStringLiteral("USB 调试授权")},
      {QStringLiteral("target_device"), QStringLiteral("目标设备选择")},
      {QStringLiteral("shell_control"), QStringLiteral("手机命令控制")},
      {QStringLiteral("open_link"), QStringLiteral("打开文章链接")},
      {QStringLiteral("local_bridge"), QStringLiteral("本地桥端口")},
      {QStringLiteral("proxy_port"), QStringLiteral("本地代理端口")},
      {QStringLiteral("platform_guidance"), QStringLiteral("驱动和系统指引")},
  };
  return labels.value(item.id, item.label);
}

QString localizedPlatformHint(UiLanguage language) {
  if (language != UiLanguage::Chinese) return QString();
#if defined(Q_OS_WIN)
  return QStringLiteral("Windows：安装安卓平台工具，然后安装手机厂商 USB 驱动或通用 USB 驱动。打开设备管理器，确认手机显示为安卓调试接口，而不是未知设备。");
#elif defined(Q_OS_LINUX)
  return QStringLiteral("Linux：安装安卓调试工具。如果设备显示权限不足，添加厂商设备规则，重新加载规则，重新插拔手机，然后重启调试服务。");
#elif defined(Q_OS_MACOS)
  return QStringLiteral("macOS：安装安卓平台工具。通常不需要厂商 USB 驱动；请确认数据线支持数据传输，并在手机上信任这台电脑。");
#else
  return QStringLiteral("安装安卓平台工具，使用支持数据传输的数据线，启用开发者选项和 USB 调试，并在手机上授权这台电脑。");
#endif
}

QString localizedDetails(const PhoneDiagnosticItem& item, UiLanguage language) {
  if (language != UiLanguage::Chinese) return item.details;
  if (item.id == QStringLiteral("adb_tool")) {
    return item.status == QStringLiteral("pass") ? QStringLiteral("安卓调试工具已安装并且可以执行。") : QStringLiteral("安卓调试工具缺失或无法运行。");
  }
  if (item.id == QStringLiteral("adb_server")) {
    return item.status == QStringLiteral("pass") ? QStringLiteral("安卓调试服务正在运行。") : QStringLiteral("安卓调试服务启动失败。");
  }
  if (item.id == QStringLiteral("device_detected")) {
    if (item.status == QStringLiteral("fail")) return QStringLiteral("当前看不到任何安卓设备。");
    QString details = item.details;
    return QStringLiteral("已检测到设备：%1").arg(details.mid(QStringLiteral("Detected device(s): ").size()).replace(QStringLiteral("device"), QStringLiteral("已授权")));
  }
  if (item.id == QStringLiteral("usb_authorization")) {
    if (item.details.contains(QStringLiteral("not authorized"))) return QStringLiteral("手机已连接，但还没有授权 USB 调试。");
    if (item.details.contains(QStringLiteral("offline"))) return QStringLiteral("从调试连接视角看，手机当前处于离线状态。");
    return QStringLiteral("至少有一台手机已经完成授权。");
  }
  if (item.id == QStringLiteral("target_device")) return QStringLiteral("检测到多台已授权设备。自动采集建议明确选择一个序列号。");
  if (item.id == QStringLiteral("shell_control")) {
    if (item.status == QStringLiteral("pass")) {
      QString details = item.details;
      return details.replace(QStringLiteral("ADB shell works."), QStringLiteral("手机命令控制可用。"));
    }
    return QStringLiteral("手机命令执行失败。");
  }
  if (item.id == QStringLiteral("open_link")) {
    if (item.details.contains(QStringLiteral("skipped"))) return QStringLiteral("为了避免改变手机当前界面，本次跳过打开链接测试。");
    return item.status == QStringLiteral("pass") ? QStringLiteral("手机已接受文章链接打开命令。") : QStringLiteral("手机没有接受文章链接打开命令。");
  }
  if (item.id == QStringLiteral("local_bridge")) {
    QString details = item.details;
    return item.status == QStringLiteral("pass") ? details.replace(QStringLiteral("Local bridge port"), QStringLiteral("本地桥端口")).replace(QStringLiteral("is reachable."), QStringLiteral("可以访问。"))
                                                  : details.replace(QStringLiteral("Local bridge port"), QStringLiteral("本地桥端口")).replace(QStringLiteral("is not reachable right now."), QStringLiteral("当前不可访问。"));
  }
  if (item.id == QStringLiteral("proxy_port")) {
    if (item.details.contains(QStringLiteral("not configured"))) return QStringLiteral("未配置代理端口。");
    QString details = item.details;
    return item.status == QStringLiteral("pass") ? details.replace(QStringLiteral("Local proxy port"), QStringLiteral("本地代理端口")).replace(QStringLiteral("is reachable."), QStringLiteral("可以访问。"))
                                                  : details.replace(QStringLiteral("Local proxy port"), QStringLiteral("本地代理端口")).replace(QStringLiteral("is not reachable on this computer."), QStringLiteral("在这台电脑上不可访问。"));
  }
  if (item.id == QStringLiteral("platform_guidance")) {
    QString details = item.details;
    return details.replace(QStringLiteral("Platform:"), QStringLiteral("平台："));
  }
  return item.details;
}

QString localizedFixHint(const PhoneDiagnosticItem& item, UiLanguage language) {
  if (language != UiLanguage::Chinese) return item.fixHint;
  if (item.fixHint == QStringLiteral("No action needed.")) return QStringLiteral("无需处理。");
  if (item.id == QStringLiteral("adb_tool") || item.id == QStringLiteral("platform_guidance")) return localizedPlatformHint(language);
  if (item.id == QStringLiteral("adb_server")) return QStringLiteral("尝试重启安卓调试服务，重新插拔手机，然后重新诊断。");
  if (item.id == QStringLiteral("device_detected")) return localizedPlatformHint(language);
  if (item.id == QStringLiteral("usb_authorization")) {
    if (item.details.contains(QStringLiteral("offline"))) return QStringLiteral("重新插拔 USB，重启安卓调试服务；如果仍离线，重启手机。");
    return QStringLiteral("解锁手机，启用开发者选项 -> USB 调试，重新插拔 USB，然后在授权弹窗中点击允许。");
  }
  if (item.id == QStringLiteral("target_device")) return QStringLiteral("在诊断界面选择一个目标设备，避免调试工具出现多设备冲突。");
  if (item.id == QStringLiteral("shell_control")) return QStringLiteral("如果命令控制失败，请重新授权 USB 调试或重启安卓调试服务。");
  if (item.id == QStringLiteral("open_link")) {
    if (item.details.contains(QStringLiteral("skipped"))) return QStringLiteral("需要验证完整打开链路时，再点击“测试打开链接”。");
    return QStringLiteral("确认手机已解锁，并且浏览器或微信可以处理该文章链接。");
  }
  if (item.id == QStringLiteral("local_bridge")) return QStringLiteral("启动应用内本地桥，或在“微信接入”页运行本地桥冒烟测试。");
  if (item.id == QStringLiteral("proxy_port")) {
    if (item.details.contains(QStringLiteral("not configured"))) return QStringLiteral("如果需要检测代理进程是否运行，请填写本地代理适配器端口。");
    return QStringLiteral("启动 Reqable、mitmproxy、Fiddler、Charles、whistle 或 Proxyman，并确认监听端口。 ");
  }
  return item.fixHint;
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
                      .arg(localizedStatus(report.overallStatus, language_), report.targetSerial.isEmpty() ? QStringLiteral("未选择") : report.targetSerial, report.platform);
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
    table_->setItem(row, 0, new QTableWidgetItem(statusIcon(item.status) + QStringLiteral(" ") + localizedStatus(item.status, language_)));
    table_->setItem(row, 1, new QTableWidgetItem(localizedItemLabel(item, language_)));
    table_->setItem(row, 2, new QTableWidgetItem(localizedDetails(item, language_)));
    table_->setItem(row, 3, new QTableWidgetItem(localizedFixHint(item, language_)));
    table_->setItem(row, 4, new QTableWidgetItem(item.rawOutput.left(800)));
  }
  table_->resizeColumnsToContents();
}

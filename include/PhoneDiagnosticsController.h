#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

/**
 * @brief A single phone/ADB diagnostic result.
 * @brief 单项手机/ADB 诊断结果。
 *
 * English: Each item is intentionally user-facing: it contains a stable id for
 * automation, bilingual-ready label/details/hints from the caller, and raw command
 * output for support. This avoids the common “ADB failed” black box.
 * 中文：每个诊断项都面向用户展示：稳定 id 便于自动化，label/details/hints 可翻译，
 * rawOutput 保留原始命令输出，避免用户只看到“ADB失败”却不知道原因。
 */
struct PhoneDiagnosticItem {
  QString id;
  QString label;
  QString status = QStringLiteral("unknown");  // pass / warn / fail / unknown
  QString details;
  QString fixHint;
  QString rawOutput;
};

/**
 * @brief A detected Android device from `adb devices -l`.
 * @brief 从 `adb devices -l` 解析出的安卓设备。
 */
struct PhoneDeviceInfo {
  QString serial;
  QString state;
  QString model;
  QString product;
  QString transportId;
  QString rawLine;
};

/**
 * @brief Full diagnostic report for phone auto-ingestion readiness.
 * @brief 手机自动采集就绪状态的完整诊断报告。
 */
struct PhoneDiagnosticReport {
  QString overallStatus = QStringLiteral("unknown");  // ready / warning / blocked / unknown
  QString targetSerial;
  QString platform;
  QVector<PhoneDeviceInfo> devices;
  QVector<PhoneDiagnosticItem> items;
};

/**
 * @brief PhoneDiagnosticsController runs P0/P1/P2 checks for ADB, phone, proxy, and bridge readiness.
 * @brief 手机诊断控制器：执行 P0/P1/P2 的 ADB、手机、代理、本地桥就绪检测。
 *
 * English design intent:
 * - P0: make sure ADB exists, server starts, a usable authorized phone is selected,
 *   shell commands work, and article URLs can be opened.
 * - P1: check local proxy port, phone-to-computer reachability hint, and localhost
 *   bridge smoke readiness.
 * - P2: generate platform-specific driver/udev guidance and exportable JSON report.
 *
 * 中文设计意图：
 * - P0：确认 ADB 存在、服务可启动、有授权手机、shell 可控、能打开文章链接。
 * - P1：检查本地代理端口、手机访问电脑链路提示、本地桥冒烟能力。
 * - P2：生成按平台区分的驱动/udev 修复建议，并可导出 JSON 诊断报告。
 */
class PhoneDiagnosticsController final : public QObject {
  Q_OBJECT
 public:
  explicit PhoneDiagnosticsController(QObject* parent = nullptr);

  static QString currentPlatformName();
  static QVector<PhoneDeviceInfo> parseAdbDevices(const QString& output);
  static QString deviceSummary(const PhoneDeviceInfo& device);
  static QJsonObject reportToJson(const PhoneDiagnosticReport& report);
  static QString reportToText(const PhoneDiagnosticReport& report, bool chinese = false);
  static bool isCoreReady(const PhoneDiagnosticReport& report, QString* reason = nullptr);
  static bool isScreenUnlocked(const QString& serial = QString(), QString* rawOutput = nullptr);

  PhoneDiagnosticReport runDiagnostics(const QString& preferredSerial = QString(),
                                       quint16 bridgePort = 9000,
                                       quint16 proxyPort = 0,
                                       bool includeLinkOpenTest = false,
                                       const QString& testUrl = QStringLiteral("https://mp.weixin.qq.com/s/test"));

 signals:
  void diagnosticLog(const QString& message);

 private:
  static PhoneDiagnosticItem makeItem(const QString& id, const QString& label, const QString& status,
                                      const QString& details, const QString& fixHint,
                                      const QString& rawOutput = QString());
  static QString selectedSerial(const QVector<PhoneDeviceInfo>& devices, const QString& preferredSerial);
  static QString adbExecutable();
  static QStringList serialArgs(const QString& serial);
  static QString runProcess(const QString& program, const QStringList& args, int timeoutMs,
                            int* exitCode = nullptr, QString* stdErr = nullptr);
  static bool canConnectToLocalPort(quint16 port, int timeoutMs, QString* errorMessage = nullptr);
  static QString platformDriverHint();
};

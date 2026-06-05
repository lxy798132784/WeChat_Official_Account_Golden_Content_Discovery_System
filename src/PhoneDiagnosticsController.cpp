#include "PhoneDiagnosticsController.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QSysInfo>

namespace {
QString statusLabel(const QString& status) {
  if (status == QStringLiteral("pass")) return QStringLiteral("PASS");
  if (status == QStringLiteral("warn")) return QStringLiteral("WARN");
  if (status == QStringLiteral("fail")) return QStringLiteral("FAIL");
  return QStringLiteral("UNKNOWN");
}

QString valueAfterPrefix(const QStringList& tokens, const QString& prefix) {
  for (const QString& token : tokens) {
    if (token.startsWith(prefix)) {
      return token.mid(prefix.size()).trimmed();
    }
  }
  return QString();
}
}  // namespace

PhoneDiagnosticsController::PhoneDiagnosticsController(QObject* parent) : QObject(parent) {}

QString PhoneDiagnosticsController::currentPlatformName() {
  return QSysInfo::prettyProductName().isEmpty() ? QSysInfo::productType() : QSysInfo::prettyProductName();
}

QVector<PhoneDeviceInfo> PhoneDiagnosticsController::parseAdbDevices(const QString& output) {
  QVector<PhoneDeviceInfo> devices;
  const QStringList lines = output.split(QRegularExpression(QStringLiteral("[\r\n]+")), Qt::SkipEmptyParts);
  for (const QString& raw : lines) {
    const QString line = raw.trimmed();
    if (line.isEmpty() || line.startsWith(QStringLiteral("List of devices")) || line.startsWith(QStringLiteral("* daemon"))) {
      continue;
    }
    const QStringList tokens = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (tokens.size() < 2) {
      continue;
    }
    PhoneDeviceInfo device;
    device.serial = tokens.at(0);
    device.state = tokens.at(1);
    device.rawLine = line;
    device.model = valueAfterPrefix(tokens, QStringLiteral("model:"));
    device.product = valueAfterPrefix(tokens, QStringLiteral("product:"));
    device.transportId = valueAfterPrefix(tokens, QStringLiteral("transport_id:"));
    devices.push_back(device);
  }
  return devices;
}

QString PhoneDiagnosticsController::deviceSummary(const PhoneDeviceInfo& device) {
  QStringList parts;
  parts << device.serial << device.state;
  if (!device.model.isEmpty()) parts << QStringLiteral("model=%1").arg(device.model);
  if (!device.product.isEmpty()) parts << QStringLiteral("product=%1").arg(device.product);
  if (!device.transportId.isEmpty()) parts << QStringLiteral("transport=%1").arg(device.transportId);
  return parts.join(QStringLiteral(" | "));
}

PhoneDiagnosticItem PhoneDiagnosticsController::makeItem(const QString& id, const QString& label, const QString& status,
                                                         const QString& details, const QString& fixHint,
                                                         const QString& rawOutput) {
  PhoneDiagnosticItem item;
  item.id = id;
  item.label = label;
  item.status = status;
  item.details = details;
  item.fixHint = fixHint;
  item.rawOutput = rawOutput;
  return item;
}

QString PhoneDiagnosticsController::adbExecutable() {
  return QStringLiteral("adb");
}

QStringList PhoneDiagnosticsController::serialArgs(const QString& serial) {
  if (serial.trimmed().isEmpty()) {
    return {};
  }
  return {QStringLiteral("-s"), serial.trimmed()};
}

QString PhoneDiagnosticsController::runProcess(const QString& program, const QStringList& args, int timeoutMs,
                                               int* exitCode, QString* stdErr) {
  QProcess process;
  process.start(program, args);
  if (!process.waitForStarted(timeoutMs)) {
    if (exitCode != nullptr) *exitCode = -1;
    if (stdErr != nullptr) *stdErr = process.errorString();
    return QString();
  }
  if (!process.waitForFinished(timeoutMs)) {
    process.kill();
    process.waitForFinished(1000);
    if (exitCode != nullptr) *exitCode = -2;
    if (stdErr != nullptr) *stdErr = QStringLiteral("timeout");
    return QString::fromLocal8Bit(process.readAllStandardOutput());
  }
  if (exitCode != nullptr) *exitCode = process.exitCode();
  if (stdErr != nullptr) *stdErr = QString::fromLocal8Bit(process.readAllStandardError());
  return QString::fromLocal8Bit(process.readAllStandardOutput());
}

bool PhoneDiagnosticsController::canConnectToLocalPort(quint16 port, int timeoutMs, QString* errorMessage) {
  QTcpSocket socket;
  socket.connectToHost(QStringLiteral("127.0.0.1"), port);
  if (socket.waitForConnected(timeoutMs)) {
    socket.disconnectFromHost();
    return true;
  }
  if (errorMessage != nullptr) {
    *errorMessage = socket.errorString();
  }
  return false;
}

QString PhoneDiagnosticsController::selectedSerial(const QVector<PhoneDeviceInfo>& devices, const QString& preferredSerial) {
  const QString preferred = preferredSerial.trimmed();
  if (!preferred.isEmpty()) {
    for (const PhoneDeviceInfo& device : devices) {
      if (device.serial == preferred) return preferred;
    }
  }
  for (const PhoneDeviceInfo& device : devices) {
    if (device.state == QStringLiteral("device")) return device.serial;
  }
  return QString();
}

QString PhoneDiagnosticsController::platformDriverHint() {
#if defined(Q_OS_WIN)
  return QStringLiteral("Windows: install Android Platform Tools, then install the phone vendor USB driver or Google USB Driver. Open Device Manager and confirm the phone appears as Android ADB Interface instead of Unknown Device.");
#elif defined(Q_OS_LINUX)
  return QStringLiteral("Linux: install android-tools-adb. If the device shows no permissions, add udev rules for the vendor id, reload udev, replug the phone, then run adb kill-server && adb start-server.");
#elif defined(Q_OS_MACOS)
  return QStringLiteral("macOS: install Android Platform Tools. A vendor USB driver is usually not required; check that the cable supports data transfer and the phone trusts this computer.");
#else
  return QStringLiteral("Install Android Platform Tools, use a data-capable USB cable, enable Developer options and USB debugging, then authorize this computer on the phone.");
#endif
}

PhoneDiagnosticReport PhoneDiagnosticsController::runDiagnostics(const QString& preferredSerial,
                                                                 quint16 bridgePort,
                                                                 quint16 proxyPort,
                                                                 bool includeLinkOpenTest,
                                                                 const QString& testUrl) {
  PhoneDiagnosticReport report;
  report.platform = currentPlatformName();

  int code = 0;
  QString err;
  const QString version = runProcess(adbExecutable(), {QStringLiteral("version")}, 5000, &code, &err);
  if (code == 0 && version.contains(QStringLiteral("Android Debug Bridge"), Qt::CaseInsensitive)) {
    report.items.push_back(makeItem(QStringLiteral("adb_tool"), QStringLiteral("ADB tool"), QStringLiteral("pass"),
                                    QStringLiteral("ADB is installed and executable."), QStringLiteral("No action needed."), version));
  } else {
    report.items.push_back(makeItem(QStringLiteral("adb_tool"), QStringLiteral("ADB tool"), QStringLiteral("fail"),
                                    QStringLiteral("ADB is missing or cannot run."), platformDriverHint(), err + version));
    report.overallStatus = QStringLiteral("blocked");
    return report;
  }

  const QString server = runProcess(adbExecutable(), {QStringLiteral("start-server")}, 8000, &code, &err);
  report.items.push_back(makeItem(QStringLiteral("adb_server"), QStringLiteral("ADB server"), code == 0 ? QStringLiteral("pass") : QStringLiteral("fail"),
                                  code == 0 ? QStringLiteral("ADB server is running.") : QStringLiteral("ADB server failed to start."),
                                  QStringLiteral("Try adb kill-server, replug the phone, then run diagnostics again."), server + err));

  const QString devicesOutput = runProcess(adbExecutable(), {QStringLiteral("devices"), QStringLiteral("-l")}, 8000, &code, &err);
  report.devices = parseAdbDevices(devicesOutput);
  QStringList summaries;
  for (const PhoneDeviceInfo& device : report.devices) summaries << deviceSummary(device);

  if (report.devices.isEmpty()) {
    report.items.push_back(makeItem(QStringLiteral("device_detected"), QStringLiteral("Phone detection"), QStringLiteral("fail"),
                                    QStringLiteral("No Android device is visible to ADB."), platformDriverHint(), devicesOutput + err));
    report.overallStatus = QStringLiteral("blocked");
  } else {
    report.items.push_back(makeItem(QStringLiteral("device_detected"), QStringLiteral("Phone detection"), QStringLiteral("pass"),
                                    QStringLiteral("Detected device(s): %1").arg(summaries.join(QStringLiteral("; "))),
                                    QStringLiteral("If this is not the target phone, select the correct serial in the diagnostics UI."), devicesOutput));
  }

  int authorizedCount = 0;
  int unauthorizedCount = 0;
  int offlineCount = 0;
  for (const PhoneDeviceInfo& device : report.devices) {
    if (device.state == QStringLiteral("device")) ++authorizedCount;
    if (device.state == QStringLiteral("unauthorized")) ++unauthorizedCount;
    if (device.state == QStringLiteral("offline")) ++offlineCount;
  }
  if (unauthorizedCount > 0) {
    report.items.push_back(makeItem(QStringLiteral("usb_authorization"), QStringLiteral("USB authorization"), QStringLiteral("fail"),
                                    QStringLiteral("A phone is connected but has not authorized USB debugging."),
                                    QStringLiteral("Unlock the phone, enable Developer options -> USB debugging, replug USB, then tap Allow on the authorization dialog."), devicesOutput));
  } else if (offlineCount > 0) {
    report.items.push_back(makeItem(QStringLiteral("usb_authorization"), QStringLiteral("USB authorization"), QStringLiteral("fail"),
                                    QStringLiteral("A phone is offline from ADB's point of view."),
                                    QStringLiteral("Replug USB, restart ADB server, or reboot the phone if it stays offline."), devicesOutput));
  } else if (authorizedCount > 0) {
    report.items.push_back(makeItem(QStringLiteral("usb_authorization"), QStringLiteral("USB authorization"), QStringLiteral("pass"),
                                    QStringLiteral("At least one phone is authorized."), QStringLiteral("No action needed."), devicesOutput));
  }

  if (authorizedCount > 1 && preferredSerial.trimmed().isEmpty()) {
    report.items.push_back(makeItem(QStringLiteral("target_device"), QStringLiteral("Target device selection"), QStringLiteral("warn"),
                                    QStringLiteral("Multiple authorized devices are connected. Auto-ingestion should use an explicit serial."),
                                    QStringLiteral("Select one target device in the diagnostics UI to avoid ADB 'more than one device' failures."), devicesOutput));
  }

  report.targetSerial = selectedSerial(report.devices, preferredSerial);
  if (!report.targetSerial.isEmpty()) {
    const QStringList baseArgs = serialArgs(report.targetSerial);
    const QString model = runProcess(adbExecutable(), baseArgs + QStringList{QStringLiteral("shell"), QStringLiteral("getprop"), QStringLiteral("ro.product.model")}, 8000, &code, &err).trimmed();
    const QString android = runProcess(adbExecutable(), baseArgs + QStringList{QStringLiteral("shell"), QStringLiteral("getprop"), QStringLiteral("ro.build.version.release")}, 8000, &code, &err).trimmed();
    const QString echo = runProcess(adbExecutable(), baseArgs + QStringList{QStringLiteral("shell"), QStringLiteral("echo"), QStringLiteral("ok")}, 8000, &code, &err).trimmed();
    const bool shellOk = code == 0 && echo.contains(QStringLiteral("ok"));
    report.items.push_back(makeItem(QStringLiteral("shell_control"), QStringLiteral("Shell control"), shellOk ? QStringLiteral("pass") : QStringLiteral("fail"),
                                    shellOk ? QStringLiteral("ADB shell works. Model=%1 Android=%2").arg(model, android)
                                            : QStringLiteral("ADB shell command failed."),
                                    QStringLiteral("If shell fails, re-authorize USB debugging or restart ADB server."), echo + err));

    if (includeLinkOpenTest) {
      const QString openOutput = runProcess(adbExecutable(), baseArgs + QStringList{QStringLiteral("shell"), QStringLiteral("am"), QStringLiteral("start"),
                                                                                   QStringLiteral("-a"), QStringLiteral("android.intent.action.VIEW"),
                                                                                   QStringLiteral("-d"), testUrl}, 10000, &code, &err);
      const bool openOk = code == 0 && (openOutput.contains(QStringLiteral("Starting"), Qt::CaseInsensitive) || err.isEmpty());
      report.items.push_back(makeItem(QStringLiteral("open_link"), QStringLiteral("Open article link"), openOk ? QStringLiteral("pass") : QStringLiteral("fail"),
                                      openOk ? QStringLiteral("The phone accepted an article URL open command.") : QStringLiteral("The phone did not accept the URL open command."),
                                      QStringLiteral("Check that a browser or WeChat can handle the URL and that the phone is unlocked."), openOutput + err));
    } else {
      report.items.push_back(makeItem(QStringLiteral("open_link"), QStringLiteral("Open article link"), QStringLiteral("warn"),
                                      QStringLiteral("Open-link test was skipped to avoid changing the phone screen."),
                                      QStringLiteral("Click Test Open Link when you want to verify the full ADB article-opening path.")));
    }
  }

  QString portError;
  const bool bridgeOk = canConnectToLocalPort(bridgePort, 800, &portError);
  report.items.push_back(makeItem(QStringLiteral("local_bridge"), QStringLiteral("Local bridge port"), bridgeOk ? QStringLiteral("pass") : QStringLiteral("warn"),
                                  bridgeOk ? QStringLiteral("Local bridge port %1 is reachable.").arg(bridgePort)
                                           : QStringLiteral("Local bridge port %1 is not reachable right now.").arg(bridgePort),
                                  QStringLiteral("Start the app bridge or run the built-in bridge smoke test from WeChat Integration."), portError));

  if (proxyPort > 0) {
    const bool proxyOk = canConnectToLocalPort(proxyPort, 800, &portError);
    report.items.push_back(makeItem(QStringLiteral("proxy_port"), QStringLiteral("Local proxy port"), proxyOk ? QStringLiteral("pass") : QStringLiteral("warn"),
                                    proxyOk ? QStringLiteral("Local proxy port %1 is reachable.").arg(proxyPort)
                                            : QStringLiteral("Local proxy port %1 is not reachable on this computer.").arg(proxyPort),
                                    QStringLiteral("Start Reqable/mitmproxy/Fiddler/Charles/whistle/Proxyman and confirm its listening port."), portError));
  } else {
    report.items.push_back(makeItem(QStringLiteral("proxy_port"), QStringLiteral("Local proxy port"), QStringLiteral("warn"),
                                    QStringLiteral("Proxy port was not configured."),
                                    QStringLiteral("Set the local proxy adapter port if you want the app to test whether the proxy process is running.")));
  }

  report.items.push_back(makeItem(QStringLiteral("platform_guidance"), QStringLiteral("Driver and OS guidance"), QStringLiteral("pass"),
                                  QStringLiteral("Platform: %1").arg(report.platform), platformDriverHint()));

  QString reason;
  report.overallStatus = isCoreReady(report, &reason) ? QStringLiteral("ready") : QStringLiteral("blocked");
  if (report.overallStatus == QStringLiteral("ready")) {
    for (const PhoneDiagnosticItem& item : report.items) {
      if (item.status == QStringLiteral("warn")) {
        report.overallStatus = QStringLiteral("warning");
        break;
      }
    }
  }
  return report;
}

bool PhoneDiagnosticsController::isCoreReady(const PhoneDiagnosticReport& report, QString* reason) {
  const QStringList required = {QStringLiteral("adb_tool"), QStringLiteral("adb_server"), QStringLiteral("device_detected"),
                                QStringLiteral("usb_authorization"), QStringLiteral("shell_control")};
  for (const QString& id : required) {
    bool found = false;
    for (const PhoneDiagnosticItem& item : report.items) {
      if (item.id == id) {
        found = true;
        if (item.status != QStringLiteral("pass")) {
          if (reason != nullptr) *reason = item.details;
          return false;
        }
      }
    }
    if (!found) {
      if (reason != nullptr) *reason = QStringLiteral("Missing diagnostic item: %1").arg(id);
      return false;
    }
  }
  if (report.targetSerial.isEmpty()) {
    if (reason != nullptr) *reason = QStringLiteral("No target phone serial selected.");
    return false;
  }
  return true;
}

QJsonObject PhoneDiagnosticsController::reportToJson(const PhoneDiagnosticReport& report) {
  QJsonObject root;
  root.insert(QStringLiteral("generated_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
  root.insert(QStringLiteral("overall_status"), report.overallStatus);
  root.insert(QStringLiteral("target_serial"), report.targetSerial);
  root.insert(QStringLiteral("platform"), report.platform);
  QJsonArray devices;
  for (const PhoneDeviceInfo& device : report.devices) {
    QJsonObject object;
    object.insert(QStringLiteral("serial"), device.serial);
    object.insert(QStringLiteral("state"), device.state);
    object.insert(QStringLiteral("model"), device.model);
    object.insert(QStringLiteral("product"), device.product);
    object.insert(QStringLiteral("transport_id"), device.transportId);
    object.insert(QStringLiteral("raw_line"), device.rawLine);
    devices.push_back(object);
  }
  root.insert(QStringLiteral("devices"), devices);
  QJsonArray items;
  for (const PhoneDiagnosticItem& item : report.items) {
    QJsonObject object;
    object.insert(QStringLiteral("id"), item.id);
    object.insert(QStringLiteral("label"), item.label);
    object.insert(QStringLiteral("status"), item.status);
    object.insert(QStringLiteral("details"), item.details);
    object.insert(QStringLiteral("fix_hint"), item.fixHint);
    object.insert(QStringLiteral("raw_output"), item.rawOutput);
    items.push_back(object);
  }
  root.insert(QStringLiteral("items"), items);
  return root;
}

QString PhoneDiagnosticsController::reportToText(const PhoneDiagnosticReport& report) {
  QStringList lines;
  lines << QStringLiteral("Phone Diagnostics Report")
        << QStringLiteral("Overall: %1").arg(report.overallStatus)
        << QStringLiteral("Platform: %1").arg(report.platform)
        << QStringLiteral("Target serial: %1").arg(report.targetSerial.isEmpty() ? QStringLiteral("<none>") : report.targetSerial)
        << QStringLiteral("");
  for (const PhoneDiagnosticItem& item : report.items) {
    lines << QStringLiteral("[%1] %2").arg(statusLabel(item.status), item.label)
          << QStringLiteral("  Details: %1").arg(item.details)
          << QStringLiteral("  Fix: %1").arg(item.fixHint);
    if (!item.rawOutput.trimmed().isEmpty()) {
      lines << QStringLiteral("  Raw: %1").arg(item.rawOutput.trimmed().left(600));
    }
  }
  return lines.join(QStringLiteral("\n"));
}

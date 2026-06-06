#include "WeChatSearchAutomationController.h"

#include <QProcess>
#include <QRegularExpression>
#include <QThread>
#include <QXmlStreamReader>

QStringList WeChatSearchAutomationController::adbLaunchWeChatArguments() {
  return {QStringLiteral("shell"), QStringLiteral("monkey"), QStringLiteral("-p"),
          QStringLiteral("com.tencent.mm"), QStringLiteral("1")};
}

QStringList WeChatSearchAutomationController::adbTapArguments(int x, int y) {
  return {QStringLiteral("shell"), QStringLiteral("input"), QStringLiteral("tap"), QString::number(x), QString::number(y)};
}

QStringList WeChatSearchAutomationController::adbInputTextArguments(const QString& text) {
  return {QStringLiteral("shell"), QStringLiteral("input"), QStringLiteral("text"), escapeInputText(text)};
}

QStringList WeChatSearchAutomationController::adbKeyEventArguments(int keyCode) {
  return {QStringLiteral("shell"), QStringLiteral("input"), QStringLiteral("keyevent"), QString::number(keyCode)};
}

QStringList WeChatSearchAutomationController::adbDumpUiArguments() {
  return {QStringLiteral("shell"), QStringLiteral("uiautomator"), QStringLiteral("dump"),
          QStringLiteral("/sdcard/window.xml")};
}

QStringList WeChatSearchAutomationController::adbCatUiDumpArguments() {
  return {QStringLiteral("shell"), QStringLiteral("cat"), QStringLiteral("/sdcard/window.xml")};
}

QString WeChatSearchAutomationController::escapeInputText(const QString& text) {
  QString escaped = text.trimmed();
  escaped.replace(QStringLiteral("%"), QStringLiteral("%25"));
  escaped.replace(QStringLiteral(" "), QStringLiteral("%s"));
  escaped.replace(QStringLiteral("&"), QStringLiteral("\\&"));
  escaped.replace(QStringLiteral("|"), QStringLiteral("\\|"));
  escaped.replace(QStringLiteral("<"), QStringLiteral("\\<"));
  escaped.replace(QStringLiteral(">"), QStringLiteral("\\>"));
  escaped.replace(QStringLiteral(";"), QStringLiteral("\\;"));
  escaped.replace(QStringLiteral("("), QStringLiteral("\\("));
  escaped.replace(QStringLiteral(")"), QStringLiteral("\\)"));
  return escaped;
}

bool WeChatSearchAutomationController::hasChineseInputRisk(const QString& text) {
  for (const QChar ch : text) {
    if (ch.unicode() >= 0x4e00 && ch.unicode() <= 0x9fff) {
      return true;
    }
  }
  return false;
}

WeChatSearchAutomationController::Result WeChatSearchAutomationController::dryRunPlan(const QStringList& keywords,
                                                                                      const Options& options) {
  Result result;
  result.stage = QStringLiteral("plan");
  if (!options.enabled) {
    result.message = QStringLiteral("advanced_wechat_search_disabled");
    return result;
  }
  if (keywords.isEmpty()) {
    result.message = QStringLiteral("no_keywords");
    return result;
  }
  if (!options.autoLocateSearch && (options.searchTapX <= 0 || options.searchTapY <= 0)) {
    result.message = QStringLiteral("missing_search_tap_coordinates");
    return result;
  }
  result.success = true;
  result.commands << adbLaunchWeChatArguments().join(QStringLiteral(" "));
  result.commands << adbDumpUiArguments().join(QStringLiteral(" "));
  result.commands << (options.autoLocateSearch ? QStringLiteral("auto locate WeChat search entry")
                                                : adbTapArguments(options.searchTapX, options.searchTapY).join(QStringLiteral(" ")));
  for (const QString& keyword : keywords) {
    result.commands << adbInputTextArguments(keyword).join(QStringLiteral(" "));
    result.commands << adbKeyEventArguments(66).join(QStringLiteral(" "));
    if (options.resultTapX > 0 && options.resultTapY > 0) {
      result.commands << adbTapArguments(options.resultTapX, options.resultTapY).join(QStringLiteral(" "));
    }
  }
  result.message = hasChineseInputRisk(keywords.join(QString()))
                       ? QStringLiteral("plan_ready_chinese_input_may_need_phone_ime")
                       : QStringLiteral("plan_ready");
  return result;
}


bool WeChatSearchAutomationController::findNodeCenterByText(const QString& uiXml, const QStringList& textHints, int* x, int* y) {
  QXmlStreamReader reader(uiXml);
  QRegularExpression boundsRe(QStringLiteral(R"(\[(\d+),(\d+)\]\[(\d+),(\d+)\])"));
  while (!reader.atEnd()) {
    reader.readNext();
    if (!reader.isStartElement() || reader.name() != QStringLiteral("node")) {
      continue;
    }
    const auto attrs = reader.attributes();
    const QString text = attrs.value(QStringLiteral("text")).toString();
    const QString desc = attrs.value(QStringLiteral("content-desc")).toString();
    bool matched = false;
    for (const QString& hint : textHints) {
      if ((!hint.isEmpty()) && (text.contains(hint, Qt::CaseInsensitive) || desc.contains(hint, Qt::CaseInsensitive))) {
        matched = true;
        break;
      }
    }
    if (!matched) {
      continue;
    }
    const QString bounds = attrs.value(QStringLiteral("bounds")).toString();
    const auto match = boundsRe.match(bounds);
    if (!match.hasMatch()) {
      continue;
    }
    if (x != nullptr) *x = (match.captured(1).toInt() + match.captured(3).toInt()) / 2;
    if (y != nullptr) *y = (match.captured(2).toInt() + match.captured(4).toInt()) / 2;
    return true;
  }
  return false;
}

bool WeChatSearchAutomationController::runAdb(const QStringList& arguments, QString* output, QString* errorMessage, int timeoutMs) {
  QProcess process;
  process.start(QStringLiteral("adb"), arguments);
  if (!process.waitForStarted(3000)) {
    if (errorMessage != nullptr) *errorMessage = QStringLiteral("adb_start_failed");
    return false;
  }
  if (!process.waitForFinished(timeoutMs)) {
    process.kill();
    if (errorMessage != nullptr) *errorMessage = QStringLiteral("adb_timeout");
    return false;
  }
  const QString combined = QString::fromUtf8(process.readAllStandardOutput()) + QString::fromUtf8(process.readAllStandardError());
  if (output != nullptr) *output = combined;
  if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
    if (errorMessage != nullptr) *errorMessage = combined.trimmed().isEmpty() ? QStringLiteral("adb_failed") : combined.trimmed();
    return false;
  }
  return true;
}

WeChatSearchAutomationController::Result WeChatSearchAutomationController::run(const QStringList& keywords,
                                                                               const Options& options) {
  Result result = dryRunPlan(keywords, options);
  if (!result.success) {
    return result;
  }
  result.stage = QStringLiteral("launch");
  QString output;
  QString error;
  if (!runAdb(adbLaunchWeChatArguments(), &output, &error)) {
    result.success = false;
    result.message = error;
    return result;
  }
  QThread::msleep(static_cast<unsigned long>(qBound(200, options.waitMs, 10000)));
  result.stage = QStringLiteral("dump_ui");
  runAdb(adbDumpUiArguments(), &output, &error, 8000);
  QString uiXml;
  if (runAdb(adbCatUiDumpArguments(), &uiXml, &error, 8000)) {
    result.uiDumpPreview = uiXml.left(2000);
  }
  int searchX = options.searchTapX;
  int searchY = options.searchTapY;
  if (options.autoLocateSearch) {
    findNodeCenterByText(uiXml, {QStringLiteral("搜索"), QStringLiteral("Search")}, &searchX, &searchY);
    if (searchX <= 0 || searchY <= 0) {
      searchX = 925;
      searchY = 198;
    }
  }
  result.stage = QStringLiteral("tap_search");
  if (!runAdb(adbTapArguments(searchX, searchY), &output, &error)) {
    result.success = false;
    result.message = error;
    return result;
  }
  QThread::msleep(static_cast<unsigned long>(qBound(200, options.waitMs, 10000)));
  for (const QString& keyword : keywords) {
    result.stage = QStringLiteral("input_keyword");
    if (!runAdb(adbInputTextArguments(keyword), &output, &error)) {
      result.success = false;
      result.message = error;
      return result;
    }
    QThread::msleep(static_cast<unsigned long>(qBound(200, options.waitMs, 10000)));
    result.stage = QStringLiteral("dump_results");
    runAdb(adbDumpUiArguments(), &output, &error, 8000);
    uiXml.clear();
    runAdb(adbCatUiDumpArguments(), &uiXml, &error, 8000);
    int resultX = options.resultTapX;
    int resultY = options.resultTapY;
    if (options.tapNetworkResults) {
      findNodeCenterByText(uiXml, {QStringLiteral("搜索网络结果"), QStringLiteral("搜一搜"), QStringLiteral("文章")}, &resultX, &resultY);
    }
    if (resultX > 0 && resultY > 0) {
      result.stage = QStringLiteral("tap_result");
      runAdb(adbTapArguments(resultX, resultY), &output, &error);
      QThread::msleep(static_cast<unsigned long>(qBound(200, options.waitMs, 10000)));
    } else {
      result.stage = QStringLiteral("search_results_visible");
    }
  }
  result.stage = QStringLiteral("done");
  result.message = QStringLiteral("advanced_wechat_search_completed");
  return result;
}

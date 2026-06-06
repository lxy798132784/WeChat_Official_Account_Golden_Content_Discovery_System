#include "WeChatSearchAutomationController.h"

#include <QProcess>
#include <QRegularExpression>
#include <QThread>
#include <QVector>
#include <algorithm>
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

bool WeChatSearchAutomationController::findArticleEntryCenter(const QString& uiXml, int* x, int* y) {
  struct Candidate {
    int x = 0;
    int y = 0;
    int area = 0;
    int score = 0;
  };
  QVector<Candidate> candidates;
  QXmlStreamReader reader(uiXml);
  QRegularExpression boundsRe(QStringLiteral(R"(\[(\d+),(\d+)\]\[(\d+),(\d+)\])"));
  const QStringList positiveHints = {QStringLiteral("mp.weixin.qq.com"), QStringLiteral("公众号"), QStringLiteral("文章"),
                                     QStringLiteral("阅读"), QStringLiteral("原创"), QStringLiteral("发布于"),
                                     QStringLiteral("搜索网络结果"), QStringLiteral("更多")};
  const QStringList negativeHints = {QStringLiteral("聊天记录"), QStringLiteral("查找账号"), QStringLiteral("朋友圈"),
                                     QStringLiteral("小程序"), QStringLiteral("表情"), QStringLiteral("音乐"),
                                     QStringLiteral("取消"), QStringLiteral("返回")};
  while (!reader.atEnd()) {
    reader.readNext();
    if (!reader.isStartElement() || reader.name() != QStringLiteral("node")) {
      continue;
    }
    const auto attrs = reader.attributes();
    const QString text = attrs.value(QStringLiteral("text")).toString();
    const QString desc = attrs.value(QStringLiteral("content-desc")).toString();
    const QString clazz = attrs.value(QStringLiteral("class")).toString();
    const QString combined = text + QStringLiteral(" ") + desc;
    const QString bounds = attrs.value(QStringLiteral("bounds")).toString();
    const auto match = boundsRe.match(bounds);
    if (!match.hasMatch()) {
      continue;
    }
    const int left = match.captured(1).toInt();
    const int top = match.captured(2).toInt();
    const int right = match.captured(3).toInt();
    const int bottom = match.captured(4).toInt();
    const int width = right - left;
    const int height = bottom - top;
    if (width <= 80 || height <= 30 || top < 250 || top > 2100) {
      continue;
    }
    int score = 0;
    for (const QString& hint : positiveHints) {
      if (combined.contains(hint, Qt::CaseInsensitive)) score += 5;
    }
    for (const QString& hint : negativeHints) {
      if (combined.contains(hint, Qt::CaseInsensitive)) score -= 8;
    }
    if (clazz.contains(QStringLiteral("TextView"))) score += 2;
    if (attrs.value(QStringLiteral("clickable")) == QStringLiteral("true")) score += 4;
    if (width > 400) score += 2;
    if (height > 70) score += 1;
    if (combined.trimmed().size() >= 8) score += 2;
    if (score <= 0) {
      continue;
    }
    candidates.push_back({(left + right) / 2, (top + bottom) / 2, width * height, score});
  }
  if (candidates.isEmpty()) {
    return false;
  }
  std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
    if (a.score != b.score) return a.score > b.score;
    if (a.y != b.y) return a.y < b.y;
    return a.area > b.area;
  });
  if (x != nullptr) *x = candidates.first().x;
  if (y != nullptr) *y = candidates.first().y;
  return true;
}

bool WeChatSearchAutomationController::uiDumpLooksLikeArticlePage(const QString& uiXml) {
  const QStringList articleHints = {QStringLiteral("mp.weixin.qq.com"), QStringLiteral("阅读"), QStringLiteral("赞"),
                                    QStringLiteral("在看"), QStringLiteral("写留言"), QStringLiteral("原创"),
                                    QStringLiteral("微信公众平台"), QStringLiteral("公众号")};
  int hits = 0;
  for (const QString& hint : articleHints) {
    if (uiXml.contains(hint, Qt::CaseInsensitive)) {
      ++hits;
    }
  }
  return hits >= 2;
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
      result.stage = QStringLiteral("tap_network_results");
      runAdb(adbTapArguments(resultX, resultY), &output, &error);
      QThread::msleep(static_cast<unsigned long>(qBound(200, options.waitMs, 10000)));
      runAdb(adbDumpUiArguments(), &output, &error, 8000);
      uiXml.clear();
      runAdb(adbCatUiDumpArguments(), &uiXml, &error, 8000);
      int articleX = 0;
      int articleY = 0;
      if (findArticleEntryCenter(uiXml, &articleX, &articleY)) {
        result.stage = QStringLiteral("tap_article");
        runAdb(adbTapArguments(articleX, articleY), &output, &error);
        QThread::msleep(static_cast<unsigned long>(qBound(600, options.waitMs * 2, 15000)));
        runAdb(adbDumpUiArguments(), &output, &error, 8000);
        uiXml.clear();
        runAdb(adbCatUiDumpArguments(), &uiXml, &error, 8000);
        if (uiDumpLooksLikeArticlePage(uiXml)) {
          result.stage = QStringLiteral("article_visible");
        }
      } else {
        result.stage = QStringLiteral("network_results_visible");
      }
    } else {
      result.stage = QStringLiteral("search_results_visible");
    }
  }
  result.stage = QStringLiteral("done");
  result.message = QStringLiteral("advanced_wechat_search_completed");
  return result;
}

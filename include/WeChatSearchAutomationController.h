#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

/**
 * Advanced, opt-in phone-side WeChat search helper.
 *
 * This class deliberately does not bypass WeChat or collect credentials. It only
 * builds and, when explicitly enabled, runs conservative ADB commands that open
 * WeChat, dump the visible UI tree for operator-side troubleshooting, and input a
 * search keyword through normal Android input events. It is a fallback for cases
 * where desktop search sources are unavailable; the primary path remains desktop
 * candidate search plus ADB article opening.
 */
class WeChatSearchAutomationController final {
 public:
  struct Options {
    bool enabled = false;
    bool autoLocateSearch = true;
    bool tapNetworkResults = true;
    int searchTapX = 0;
    int searchTapY = 0;
    int resultTapX = 0;
    int resultTapY = 0;
    int waitMs = 1200;
  };

  struct Result {
    bool success = false;
    QString stage;
    QString message;
    QStringList commands;
    QString uiDumpPreview;
  };

  static QStringList adbLaunchWeChatArguments();
  static QStringList adbTapArguments(int x, int y);
  static QStringList adbInputTextArguments(const QString& text);
  static QStringList adbKeyEventArguments(int keyCode);
  static QStringList adbDumpUiArguments();
  static QStringList adbCatUiDumpArguments();
  static QString escapeInputText(const QString& text);
  static bool hasChineseInputRisk(const QString& text);
  static Result dryRunPlan(const QStringList& keywords, const Options& options);
  static bool findNodeCenterByText(const QString& uiXml, const QStringList& textHints, int* x, int* y);
  static Result run(const QStringList& keywords, const Options& options);

 private:
  static bool runAdb(const QStringList& arguments, QString* output, QString* errorMessage, int timeoutMs = 10000);
};

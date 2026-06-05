#pragma once

#include <QWidget>

#include "PhoneDiagnosticsController.h"
#include "UiText.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

/**
 * @brief User-facing phone diagnostics panel.
 * @brief 面向用户的手机接入诊断面板。
 *
 * English: This widget intentionally exposes each readiness layer as a visible
 * control/result so non-developer users can understand whether the blocker is ADB,
 * USB authorization, drivers, phone control, proxy, or bridge ingestion.
 * 中文：这个控件把每一层就绪状态都展示出来，让非开发用户也能判断问题在 ADB、
 * USB 授权、驱动、手机控制、代理还是本地桥入库。
 */
class PhoneDiagnosticsWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit PhoneDiagnosticsWidget(QWidget* parent = nullptr);

  void setLanguage(UiLanguage language);
  void setReport(const PhoneDiagnosticReport& report);
  QString selectedSerial() const;
  quint16 proxyPort() const;
  bool includeOpenLinkTest() const;
  QString testUrl() const;

 signals:
  void runDiagnosticsRequested();
  void restartAdbRequested();
  void testOpenLinkRequested();
  void copyReportRequested();
  void exportJsonRequested();

 private:
  UiLanguage language_ = UiLanguage::English;
  PhoneDiagnosticReport currentReport_;

  QLabel* introLabel_ = nullptr;
  QLabel* overallLabel_ = nullptr;
  QLabel* serialLabel_ = nullptr;
  QLabel* proxyPortLabel_ = nullptr;
  QLabel* testUrlLabel_ = nullptr;
  QComboBox* serialComboBox_ = nullptr;
  QSpinBox* proxyPortSpinBox_ = nullptr;
  QCheckBox* openLinkCheckBox_ = nullptr;
  QPlainTextEdit* testUrlEdit_ = nullptr;
  QPushButton* runButton_ = nullptr;
  QPushButton* restartAdbButton_ = nullptr;
  QPushButton* openLinkButton_ = nullptr;
  QPushButton* copyReportButton_ = nullptr;
  QPushButton* exportJsonButton_ = nullptr;
  QTableWidget* table_ = nullptr;
};

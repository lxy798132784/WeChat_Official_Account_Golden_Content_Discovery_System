#include "MainWindow.h"

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QJsonDocument>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QProcess>
#include <QShortcut>
#include <QStatusBar>
#include <QTabWidget>
#include <QUrl>
#include <QVBoxLayout>

#include "ControlPanelWidget.h"
#include "AutoIngestionWidget.h"
#include "DashboardWidget.h"
#include "DataViewerWidget.h"
#include "KeywordDiscoveryWidget.h"
#include "PhoneDiagnosticsWidget.h"
#include "ProductionSuiteWidget.h"
#include "ExportController.h"
#include "BridgePayloadClient.h"
#include "ManualWidget.h"
#include "RuntimeLogWidget.h"
#include "SeedManagerWidget.h"
#include "WeChatConfigWidget.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      dashboard_(new DashboardWidget(this)),
      controls_(new ControlPanelWidget(this)),
      keywordDiscoveryWidget_(new KeywordDiscoveryWidget(this)),
      phoneDiagnosticsWidget_(new PhoneDiagnosticsWidget(this)),
      productionSuiteWidget_(new ProductionSuiteWidget(this)),
      autoIngestionWidget_(new AutoIngestionWidget(this)),
      viewer_(new DataViewerWidget(this)),
      seeds_(new SeedManagerWidget(this)),
      logs_(new RuntimeLogWidget(this)),
      manual_(new ManualWidget(this)),
      wechatConfig_(new WeChatConfigWidget(this)) {
  setWindowTitle("Premium Content Radar");
  resize(1360, 820);

  auto* central = new QWidget(this);
  auto* layout = new QVBoxLayout(central);
  layout->setContentsMargins(12, 12, 12, 12);
  layout->setSpacing(8);
  layout->addWidget(dashboard_);
  layout->addWidget(viewer_);
  setCentralWidget(central);

  dockTabs_ = new QTabWidget(this);
  dockTabs_->addTab(controls_, QString());
  dockTabs_->addTab(keywordDiscoveryWidget_, QString());
  dockTabs_->addTab(phoneDiagnosticsWidget_, QString());
  dockTabs_->addTab(productionSuiteWidget_, QString());
  dockTabs_->addTab(autoIngestionWidget_, QString());
  dockTabs_->addTab(seeds_, QString());
  dockTabs_->addTab(wechatConfig_, QString());
  dockTabs_->addTab(logs_, QString());
  dockTabs_->addTab(manual_, QString());
  dock_ = new QDockWidget(this);
  dock_->setWidget(dockTabs_);
  addDockWidget(Qt::LeftDockWidgetArea, dock_);

  settings_ = AppSettingsController::load();
  language_ = settings_.language == QStringLiteral("zh") ? UiLanguage::Chinese : UiLanguage::English;
  wechatConfig_->setSettings(settings_);
  qputenv("PREMIUM_RADAR_BRIDGE_PORT", QByteArray::number(settings_.bridgePort));
  qputenv("PREMIUM_RADAR_ENABLE_ADB", settings_.adbAutomationEnabled ? "1" : "0");
  if (!reopenDatabase(settings_.databasePath)) {
    appendLogKey(QStringLiteral("database_open_failed"), database_.lastError());
  }
  connect(controls_, &ControlPanelWidget::filtersChanged, this, &MainWindow::refreshData);
  connect(seeds_, &SeedManagerWidget::addSeedRequested, this, &MainWindow::addSeedFromWidget);
  connect(seeds_, &SeedManagerWidget::removeSeedRequested, this, &MainWindow::removeSeedFromWidget);
  connect(seeds_, &SeedManagerWidget::exportSeedsRequested, this, &MainWindow::exportSeedsCsv);
  connect(wechatConfig_, &WeChatConfigWidget::settingsSaveRequested, this, &MainWindow::saveRuntimeSettings);
  connect(wechatConfig_, &WeChatConfigWidget::testBridgeRequested, this, &MainWindow::testLocalBridgePayload);
  connect(wechatConfig_, &WeChatConfigWidget::browseDatabaseRequested, this, &MainWindow::browseDatabasePath);
  connect(wechatConfig_, &WeChatConfigWidget::browsePluginDirectoryRequested, this, &MainWindow::browsePluginDirectory);
  connect(keywordDiscoveryWidget_, &KeywordDiscoveryWidget::generateSearchUrlsRequested, this, &MainWindow::generateKeywordSearchUrls);
  connect(keywordDiscoveryWidget_, &KeywordDiscoveryWidget::autoSearchRequested, this, &MainWindow::autoSearchKeywords);
  connect(keywordDiscoveryWidget_, &KeywordDiscoveryWidget::startKeywordAutoIngestionRequested, this, &MainWindow::startKeywordAutoIngestion);
  connect(&keywordDiscovery_, &KeywordDiscoveryController::searchStarted, this, &MainWindow::onKeywordSearchStarted);
  connect(&keywordDiscovery_, &KeywordDiscoveryController::searchFinished, this, &MainWindow::onKeywordSearchFinished);
  connect(&keywordDiscovery_, &KeywordDiscoveryController::logMessage, this, &MainWindow::appendLog);
  connect(keywordDiscoveryWidget_, &KeywordDiscoveryWidget::importResultsRequested, this, &MainWindow::importKeywordDiscoveryResults);
  connect(keywordDiscoveryWidget_, &KeywordDiscoveryWidget::enqueueHotResultsRequested, this, &MainWindow::enqueueKeywordDiscoveryResults);
  connect(phoneDiagnosticsWidget_, &PhoneDiagnosticsWidget::runDiagnosticsRequested, this, &MainWindow::runPhoneDiagnostics);
  connect(phoneDiagnosticsWidget_, &PhoneDiagnosticsWidget::restartAdbRequested, this, &MainWindow::restartAdbServer);
  connect(phoneDiagnosticsWidget_, &PhoneDiagnosticsWidget::testOpenLinkRequested, this, &MainWindow::testPhoneOpenLink);
  connect(phoneDiagnosticsWidget_, &PhoneDiagnosticsWidget::copyReportRequested, this, &MainWindow::copyPhoneDiagnosticsReport);
  connect(phoneDiagnosticsWidget_, &PhoneDiagnosticsWidget::exportJsonRequested, this, &MainWindow::exportPhoneDiagnosticsJson);
  connect(productionSuiteWidget_, &ProductionSuiteWidget::logMessage, this, &MainWindow::appendLog);
  connect(autoIngestionWidget_, &AutoIngestionWidget::addUrlsRequested, this, &MainWindow::addAutoIngestionUrls);
  connect(autoIngestionWidget_, &AutoIngestionWidget::startRequested, this, &MainWindow::startAutoIngestion);
  connect(autoIngestionWidget_, &AutoIngestionWidget::stopRequested, this, &MainWindow::stopAutoIngestion);
  connect(autoIngestionWidget_, &AutoIngestionWidget::runNextRequested, this, &MainWindow::runNextAutoIngestionTask);
  connect(autoIngestionWidget_, &AutoIngestionWidget::clearCompletedRequested, this, &MainWindow::clearCompletedAutoIngestionTasks);
  connect(autoIngestionWidget_, &AutoIngestionWidget::clearAllRequested, this, &MainWindow::clearAllAutoIngestionTasks);
  connect(autoIngestionWidget_, &AutoIngestionWidget::saveQueueRequested, this, &MainWindow::saveAutoIngestionQueue);
  connect(autoIngestionWidget_, &AutoIngestionWidget::loadQueueRequested, this, &MainWindow::loadAutoIngestionQueue);
  connect(&autoIngestion_, &AutoIngestionController::logMessage, this, &MainWindow::appendLog);
  connect(&autoIngestion_, &AutoIngestionController::queueChanged, this, &MainWindow::refreshAutoIngestionQueue);
  connect(&pluginDrainTimer_, &QTimer::timeout, this, &MainWindow::drainPluginRecords);

  fileMenu_ = menuBar()->addMenu(QString());
  loadSamplesAction_ = fileMenu_->addAction(QString(), this, &MainWindow::loadSampleData);
  exportArticlesCsvAction_ = fileMenu_->addAction(QString(), this, &MainWindow::exportArticlesCsv);
  exportArticlesJsonAction_ = fileMenu_->addAction(QString(), this, &MainWindow::exportArticlesJson);
  exportSeedsCsvAction_ = fileMenu_->addAction(QString(), this, &MainWindow::exportSeedsCsv);
  pluginMenu_ = menuBar()->addMenu(QString());
  loadPluginsAction_ = pluginMenu_->addAction(QString(), this, &MainWindow::loadPlugins);
  actionMenu_ = menuBar()->addMenu(QString());
  previewAction_ = actionMenu_->addAction(QString(), this, &MainWindow::previewSelectedArticle);
  starSeedAction_ = actionMenu_->addAction(QString(), this, &MainWindow::starSelectedSeed);
  bridgeSmokeAction_ = actionMenu_->addAction(QString(), this, &MainWindow::testLocalBridgePayload);
  resetAction_ = actionMenu_->addAction(QString(), this, &MainWindow::resetControls);
  showControlCenterAction_ = actionMenu_->addAction(QString(), this, &MainWindow::showControlCenter);
  helpMenu_ = menuBar()->addMenu(QString());
  languageAction_ = helpMenu_->addAction(QString(), this, &MainWindow::toggleLanguage);
  aboutAction_ = helpMenu_->addAction(QString(), this, &MainWindow::showAboutDialog);

  new QShortcut(QKeySequence(Qt::Key_Space), this, SLOT(previewSelectedArticle()));
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this, SLOT(starSelectedSeed()));
  new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(resetControls()));
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), this, SLOT(showControlCenter()));

  applyTheme();
  applyLanguage();
  if (settings_.autoLoadSamples) {
    loadSampleData();
  } else {
    refreshData();
    refreshSeeds();
  }
  loadPlugins();
  pluginDrainTimer_.start(1000);
}

void MainWindow::applyTheme() {
  qApp->setStyleSheet(
      "QMainWindow,QWidget{background:#0d1117;color:#f0f6fc;font-family:'Inter','Segoe UI','PingFang SC';}"
      "QTableView{background:#161b22;alternate-background-color:#1e1e2e;gridline-color:#30363d;selection-background-color:#1f6feb;}"
      "QHeaderView::section{background:#252538;color:#f0f6fc;padding:6px;border:1px solid #30363d;}"
      "QPushButton,QComboBox,QLineEdit,QSpinBox,QTextEdit{background:#161b22;color:#f0f6fc;border:1px solid #30363d;border-radius:8px;padding:6px;}"
      "QPushButton:hover,QComboBox:hover,QLineEdit:hover{border-color:#58a6ff;}"
      "QPushButton:pressed{background:#252538;}"
      "QSlider::groove:horizontal{height:6px;background:#30363d;border-radius:3px;}"
      "QSlider::handle:horizontal{background:#58a6ff;width:14px;border-radius:7px;}"
      "QDockWidget::title,QTabBar::tab{background:#161b22;padding:8px;}"
      "QTabBar::tab:selected{background:#252538;color:#58a6ff;}"
      "QMenuBar,QMenu{background:#161b22;color:#f0f6fc;}"
      "QStatusBar{background:#0d1117;color:#8b949e;}"
      "QLabel#manualTitle{font-size:20px;font-weight:700;color:#f0f6fc;padding:8px 0;}"
      "QToolTip{background:#252538;color:#f0f6fc;border:1px solid #58a6ff;padding:6px;}");
}

UiLanguage MainWindow::currentLanguage() const {
  return language_;
}

void MainWindow::applyLanguage() {
  setWindowTitle(UiText::text("app.title", language_));
  if (dock_ != nullptr) dock_->setWindowTitle(UiText::text("dock.title", language_));
  if (dockTabs_ != nullptr) {
    dockTabs_->setTabText(0, UiText::text("tab.filters", language_));
    dockTabs_->setTabText(1, UiText::text("tab.discover", language_));
    dockTabs_->setTabText(2, UiText::text("tab.phone", language_));
    dockTabs_->setTabText(3, UiText::text("tab.production", language_));
    dockTabs_->setTabText(4, UiText::text("tab.auto", language_));
    dockTabs_->setTabText(5, UiText::text("tab.seeds", language_));
    dockTabs_->setTabText(6, UiText::text("tab.wechat", language_));
    dockTabs_->setTabText(7, UiText::text("tab.logs", language_));
    dockTabs_->setTabText(8, UiText::text("tab.guide", language_));
  }
  if (fileMenu_ != nullptr) fileMenu_->setTitle(UiText::text("menu.file", language_));
  if (pluginMenu_ != nullptr) pluginMenu_->setTitle(UiText::text("menu.plugins", language_));
  if (actionMenu_ != nullptr) actionMenu_->setTitle(UiText::text("menu.actions", language_));
  if (helpMenu_ != nullptr) helpMenu_->setTitle(UiText::text("menu.help", language_));
  if (loadSamplesAction_ != nullptr) { loadSamplesAction_->setText(UiText::text("action.load_samples", language_)); loadSamplesAction_->setToolTip(UiText::text("tip.load_samples", language_)); }
  if (exportArticlesCsvAction_ != nullptr) { exportArticlesCsvAction_->setText(UiText::text("action.export_articles_csv", language_)); exportArticlesCsvAction_->setToolTip(UiText::text("tip.export_articles_csv", language_)); }
  if (exportArticlesJsonAction_ != nullptr) { exportArticlesJsonAction_->setText(UiText::text("action.export_articles_json", language_)); exportArticlesJsonAction_->setToolTip(UiText::text("tip.export_articles_json", language_)); }
  if (exportSeedsCsvAction_ != nullptr) exportSeedsCsvAction_->setText(UiText::text("action.export_seeds_csv", language_));
  if (loadPluginsAction_ != nullptr) { loadPluginsAction_->setText(UiText::text("action.load_plugins", language_)); loadPluginsAction_->setToolTip(UiText::text("tip.load_plugins", language_)); }
  if (previewAction_ != nullptr) { previewAction_->setText(UiText::text("action.preview", language_)); previewAction_->setToolTip(UiText::text("tip.preview", language_)); }
  if (starSeedAction_ != nullptr) { starSeedAction_->setText(UiText::text("action.star_seed", language_)); starSeedAction_->setToolTip(UiText::text("tip.star_seed", language_)); }
  if (bridgeSmokeAction_ != nullptr) { bridgeSmokeAction_->setText(UiText::text("action.bridge_smoke", language_)); bridgeSmokeAction_->setToolTip(UiText::text("tip.bridge_smoke", language_)); }
  if (resetAction_ != nullptr) { resetAction_->setText(UiText::text("action.reset", language_)); resetAction_->setToolTip(UiText::text("tip.reset", language_)); }
  if (showControlCenterAction_ != nullptr) { showControlCenterAction_->setText(UiText::text("action.show_control_center", language_)); showControlCenterAction_->setToolTip(UiText::text("tip.show_control_center", language_)); }
  if (languageAction_ != nullptr) { languageAction_->setText(UiText::text("action.language", language_)); languageAction_->setToolTip(UiText::text("tip.language", language_)); }
  if (aboutAction_ != nullptr) aboutAction_->setText(UiText::text("action.about", language_));
  dashboard_->setLanguage(language_);
  controls_->setLanguage(language_);
  keywordDiscoveryWidget_->setLanguage(language_);
  phoneDiagnosticsWidget_->setLanguage(language_);
  productionSuiteWidget_->setLanguage(language_);
  autoIngestionWidget_->setLanguage(language_);
  viewer_->setLanguage(language_);
  seeds_->setLanguage(language_);
  logs_->setLanguage(language_);
  manual_->setLanguage(language_);
  wechatConfig_->setLanguage(language_);
}

void MainWindow::toggleLanguage() {
  language_ = language_ == UiLanguage::Chinese ? UiLanguage::English : UiLanguage::Chinese;
  settings_.language = language_ == UiLanguage::Chinese ? QStringLiteral("zh") : QStringLiteral("en");
  QString error;
  AppSettingsController::save(settings_, &error);
  applyLanguage();
  appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("已切换到中文界面") : QStringLiteral("Switched to English UI"));
}

void MainWindow::showControlCenter() {
  if (dock_ == nullptr) {
    return;
  }
  dock_->show();
  dock_->raise();
  dock_->setFloating(false);
  appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("控制中心已打开") : QStringLiteral("Control Center opened"));
}

void MainWindow::appendLog(const QString& message) {
  const QString line = QStringLiteral("[%1] %2").arg(QDateTime::currentDateTime().toString(Qt::ISODate), message);
  logs_->appendLog(line);
  wechatConfig_->appendLog(line);
  statusBar()->showMessage(message);
}

QString MainWindow::trLog(const QString& key, const QString& value) const {
  const bool zh = language_ == UiLanguage::Chinese;
  if (key == QStringLiteral("database_open_failed")) return zh ? QStringLiteral("数据库打开失败：%1").arg(value) : QStringLiteral("Database open failed: %1").arg(value);
  if (key == QStringLiteral("sample_loaded")) return zh ? QStringLiteral("示例记录已加载") : QStringLiteral("Sample records loaded");
  if (key == QStringLiteral("plugin_start_failed")) return zh ? QStringLiteral("插件启动失败：%1").arg(value) : QStringLiteral("Plugin start failed: %1").arg(value);
  if (key == QStringLiteral("provider_started")) return zh ? QStringLiteral("内容提供者已启动：%1").arg(value) : QStringLiteral("Provider started: %1").arg(value);
  if (key == QStringLiteral("loaded_providers")) return zh ? QStringLiteral("已加载内容提供者数量：%1").arg(value) : QStringLiteral("Loaded providers: %1").arg(value);
  if (key == QStringLiteral("ingested_provider_records")) return zh ? QStringLiteral("已接入内容提供者记录：%1").arg(value) : QStringLiteral("Ingested provider records: %1").arg(value);
  if (key == QStringLiteral("settings_save_failed")) return zh ? QStringLiteral("配置保存失败：%1").arg(value) : QStringLiteral("Settings save failed: %1").arg(value);
  if (key == QStringLiteral("database_reopen_failed")) return zh ? QStringLiteral("数据库重新打开失败：%1").arg(value) : QStringLiteral("Database reopen failed: %1").arg(value);
  if (key == QStringLiteral("runtime_settings_saved")) return zh ? QStringLiteral("运行配置已保存：%1").arg(value) : QStringLiteral("Runtime settings saved: %1").arg(value);
  if (key == QStringLiteral("restart_reload_providers")) return zh ? QStringLiteral("更改本地桥端口或 ADB 设置后，请重启或重新加载 Provider。") : QStringLiteral("Restart or reload providers after changing the bridge port or ADB setting.");
  if (key == QStringLiteral("bridge_smoke_failed")) return zh ? QStringLiteral("本地桥冒烟载荷发送失败：%1").arg(value) : QStringLiteral("Bridge smoke payload failed: %1").arg(value);
  if (key == QStringLiteral("bridge_smoke_sent")) return zh ? QStringLiteral("本地桥冒烟载荷已发送：%1").arg(value) : QStringLiteral("Bridge smoke payload sent: %1").arg(value);
  if (key == QStringLiteral("controls_reset")) return zh ? QStringLiteral("控件已重置") : QStringLiteral("Controls reset");
  if (key == QStringLiteral("add_seed_failed")) return zh ? QStringLiteral("添加种子失败：%1").arg(value) : QStringLiteral("Add seed failed: %1").arg(value);
  if (key == QStringLiteral("seed_saved")) return zh ? QStringLiteral("种子已保存：%1").arg(value) : QStringLiteral("Seed saved: %1").arg(value);
  if (key == QStringLiteral("no_seed_selected")) return zh ? QStringLiteral("未选中种子") : QStringLiteral("No seed selected");
  if (key == QStringLiteral("seed_removed")) return zh ? QStringLiteral("种子已删除：%1").arg(value) : QStringLiteral("Seed removed: %1").arg(value);
  if (key == QStringLiteral("export_articles_csv_failed")) return zh ? QStringLiteral("导出文章 CSV 失败：%1").arg(value) : QStringLiteral("Export articles CSV failed: %1").arg(value);
  if (key == QStringLiteral("articles_csv_exported")) return zh ? QStringLiteral("文章 CSV 已导出：%1").arg(value) : QStringLiteral("Articles CSV exported: %1").arg(value);
  if (key == QStringLiteral("export_articles_json_failed")) return zh ? QStringLiteral("导出文章 JSON 失败：%1").arg(value) : QStringLiteral("Export articles JSON failed: %1").arg(value);
  if (key == QStringLiteral("articles_json_exported")) return zh ? QStringLiteral("文章 JSON 已导出：%1").arg(value) : QStringLiteral("Articles JSON exported: %1").arg(value);
  if (key == QStringLiteral("export_seeds_failed")) return zh ? QStringLiteral("导出种子失败：%1").arg(value) : QStringLiteral("Export seeds failed: %1").arg(value);
  if (key == QStringLiteral("seeds_csv_exported")) return zh ? QStringLiteral("种子 CSV 已导出：%1").arg(value) : QStringLiteral("Seeds CSV exported: %1").arg(value);
  return value.isEmpty() ? key : QStringLiteral("%1: %2").arg(key, value);
}

void MainWindow::appendLogKey(const QString& key, const QString& value) {
  appendLog(trLog(key, value));
}

QString MainWindow::pluginDirectory() const {
  return settings_.pluginDirectory.isEmpty() ? AppSettingsController::defaultPluginDirectory()
                                             : settings_.pluginDirectory;
}

void MainWindow::loadSampleData() {
  database_.addSeed("gh_seed_001", "Deep Insight Lab", "Technology");
  database_.addSeed("gh_seed_002", "Radar Notes", "Media");
  ContentRecord first;
  first.title = "AI Agents Are Changing Content Discovery";
  first.accountName = "Deep Insight Lab";
  first.gzhId = "gh_seed_001";
  first.category = "Technology";
  first.url = "https://example.local/article/1";
  first.readNum = 120000;
  first.likeNum = 5200;
  first.oldLikeNum = 1800;
  first.commentNum = 430;
  first.articleCount30d = 18;
  first.publishTime = QDateTime::currentDateTimeUtc().addDays(-1);
  database_.enqueueArticle(first);
  ContentRecord second;
  second.title = "High-density Comment Signals in WeChat Articles";
  second.accountName = "Radar Notes";
  second.gzhId = "gh_seed_002";
  second.category = "Media";
  second.url = "https://example.local/article/2";
  second.readNum = 76000;
  second.likeNum = 2100;
  second.oldLikeNum = 900;
  second.commentNum = 390;
  second.articleCount30d = 9;
  second.publishTime = QDateTime::currentDateTimeUtc().addDays(-3);
  database_.enqueueArticle(second);
  database_.flush();
  refreshData();
  refreshSeeds();
  appendLogKey(QStringLiteral("sample_loaded"));
}

void MainWindow::loadPlugins() {
  const int count = pluginManager_.loadFromDirectory(pluginDirectory());
  for (IContentProvider* provider : pluginManager_.providers()) {
    QString errorMessage;
    if (!provider->start(&errorMessage)) {
      appendLogKey(QStringLiteral("plugin_start_failed"), errorMessage);
    } else {
      appendLogKey(QStringLiteral("provider_started"), provider->displayName());
    }
  }
  for (const QString& line : pluginManager_.logLines()) {
    appendLog(line);
  }
  appendLogKey(QStringLiteral("loaded_providers"), QString::number(count));
}

void MainWindow::drainPluginRecords() {
  bool changed = false;
  int count = 0;
  for (IContentProvider* provider : pluginManager_.providers()) {
    for (const ContentRecord& record : provider->drainRecords()) {
      if (database_.enqueueArticle(record)) {
        changed = true;
        ++count;
      }
    }
  }
  if (changed) {
    database_.flush();
    refreshData();
    appendLogKey(QStringLiteral("ingested_provider_records"), QString::number(count));
  }
}

void MainWindow::previewSelectedArticle() {
  if (!viewer_->hasSelection()) {
    appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("未选中文章") : QStringLiteral("No article selected"));
    return;
  }
  const ContentRecord record = viewer_->selectedRecord();
  if (!record.url.isEmpty()) {
    QDesktopServices::openUrl(QUrl(record.url));
  }
  QMessageBox::information(this, language_ == UiLanguage::Chinese ? QStringLiteral("文章详情") : QStringLiteral("Article Detail"),
                           language_ == UiLanguage::Chinese
                               ? QString("%1\n%2\n账号：%3\n发布时间：%4\n阅读：%5\n点赞：%6\n评论：%7")
                                     .arg(record.title, record.url, record.accountName,
                                          record.publishTime.isValid() ? record.publishTime.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm")) : QStringLiteral("-"))
                                     .arg(record.readNum)
                                     .arg(record.likeNum + record.oldLikeNum)
                                     .arg(record.commentNum)
                               : QString("%1\n%2\nAccount: %3\nPublish Time: %4\nRead: %5\nLike: %6\nComment: %7")
                                     .arg(record.title, record.url, record.accountName,
                                          record.publishTime.isValid() ? record.publishTime.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm")) : QStringLiteral("-"))
                                     .arg(record.readNum)
                                     .arg(record.likeNum + record.oldLikeNum)
                                     .arg(record.commentNum));
}

void MainWindow::starSelectedSeed() {
  if (!viewer_->hasSelection()) {
    appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("未选中发布者") : QStringLiteral("No publisher selected"));
    return;
  }
  const ContentRecord record = viewer_->selectedRecord();
  addSeedFromWidget(record.gzhId, record.accountName, record.category);
}

bool MainWindow::reopenDatabase(const QString& path) {
  database_ = DatabaseController();
  return database_.open(path);
}

void MainWindow::saveRuntimeSettings(const AppSettings& settings) {
  settings_ = settings;
  qputenv("PREMIUM_RADAR_BRIDGE_PORT", QByteArray::number(settings_.bridgePort));
  qputenv("PREMIUM_RADAR_ENABLE_ADB", settings_.adbAutomationEnabled ? "1" : "0");
  QString error;
  if (!AppSettingsController::save(settings_, &error)) {
    appendLogKey(QStringLiteral("settings_save_failed"), error);
    return;
  }
  if (!reopenDatabase(settings_.databasePath)) {
    appendLogKey(QStringLiteral("database_reopen_failed"), database_.lastError());
    return;
  }
  refreshData();
  refreshSeeds();
  appendLogKey(QStringLiteral("runtime_settings_saved"), AppSettingsController::settingsFilePath());
  appendLogKey(QStringLiteral("restart_reload_providers"));
}

void MainWindow::testLocalBridgePayload() {
  QString error;
  const quint16 port = wechatConfig_->bridgePort();
  bool ok = BridgePayloadClient::sendPayload(QStringLiteral("127.0.0.1"), port,
                                             BridgePayloadClient::sampleMetricsPayload(), &error);
  if (ok) {
    ok = BridgePayloadClient::sendPayload(QStringLiteral("127.0.0.1"), port,
                                          BridgePayloadClient::sampleCommentPayload(), &error);
  }
  if (!ok) {
    appendLogKey(QStringLiteral("bridge_smoke_failed"), QStringLiteral("127.0.0.1:%1: %2").arg(port).arg(error));
    return;
  }
  appendLogKey(QStringLiteral("bridge_smoke_sent"), QStringLiteral("127.0.0.1:%1").arg(port));
}

void MainWindow::runPhoneDiagnostics() {
  lastPhoneReport_ = phoneDiagnostics_.runDiagnostics(
      phoneDiagnosticsWidget_->selectedSerial(), wechatConfig_->bridgePort(),
      phoneDiagnosticsWidget_->proxyPort(), phoneDiagnosticsWidget_->includeOpenLinkTest(),
      phoneDiagnosticsWidget_->testUrl());
  phoneDiagnosticsWidget_->setReport(lastPhoneReport_);
  appendLog(language_ == UiLanguage::Chinese
                ? QStringLiteral("手机诊断完成：%1").arg(lastPhoneReport_.overallStatus)
                : QStringLiteral("Phone diagnostics completed: %1").arg(lastPhoneReport_.overallStatus));
}

void MainWindow::restartAdbServer() {
  QProcess::execute(QStringLiteral("adb"), {QStringLiteral("kill-server")});
  QProcess::execute(QStringLiteral("adb"), {QStringLiteral("start-server")});
  appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("已请求重启 ADB 服务") : QStringLiteral("ADB server restart requested"));
  runPhoneDiagnostics();
}

void MainWindow::testPhoneOpenLink() {
  lastPhoneReport_ = phoneDiagnostics_.runDiagnostics(
      phoneDiagnosticsWidget_->selectedSerial(), wechatConfig_->bridgePort(),
      phoneDiagnosticsWidget_->proxyPort(), true, phoneDiagnosticsWidget_->testUrl());
  phoneDiagnosticsWidget_->setReport(lastPhoneReport_);
  appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("手机打开链接测试完成") : QStringLiteral("Phone open-link test completed"));
}

void MainWindow::copyPhoneDiagnosticsReport() {
  QApplication::clipboard()->setText(PhoneDiagnosticsController::reportToText(lastPhoneReport_));
  appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("手机诊断报告已复制") : QStringLiteral("Phone diagnostics report copied"));
}

void MainWindow::exportPhoneDiagnosticsJson() {
  const QString path = QFileDialog::getSaveFileName(this,
                                                    language_ == UiLanguage::Chinese ? QStringLiteral("导出手机诊断 JSON") : QStringLiteral("Export phone diagnostics JSON"),
                                                    QStringLiteral("phone-diagnostics.json"), QStringLiteral("JSON (*.json)"));
  if (path.isEmpty()) return;
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("诊断 JSON 导出失败：%1").arg(file.errorString()) : QStringLiteral("Diagnostics JSON export failed: %1").arg(file.errorString()));
    return;
  }
  file.write(QJsonDocument(PhoneDiagnosticsController::reportToJson(lastPhoneReport_)).toJson(QJsonDocument::Indented));
  appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("手机诊断 JSON 已导出：%1").arg(path) : QStringLiteral("Phone diagnostics JSON exported: %1").arg(path));
}

void MainWindow::browseDatabasePath() {
  const QString path = QFileDialog::getSaveFileName(this, "SQLite Database", wechatConfig_->databasePath(), "SQLite (*.db *.sqlite);;All files (*)");
  if (path.isEmpty()) {
    return;
  }
  AppSettings settings = wechatConfig_->settings();
  settings.databasePath = path;
  wechatConfig_->setSettings(settings);
}

void MainWindow::browsePluginDirectory() {
  const QString path = QFileDialog::getExistingDirectory(this, "Plugin Directory", wechatConfig_->pluginDirectory());
  if (path.isEmpty()) {
    return;
  }
  AppSettings settings = wechatConfig_->settings();
  settings.pluginDirectory = path;
  wechatConfig_->setSettings(settings);
}

void MainWindow::resetControls() {
  controls_->clearSearch();
  controls_->resetDefaults();
  refreshData();
  appendLogKey(QStringLiteral("controls_reset"));
}

void MainWindow::addSeedFromWidget(const QString& gzhId, const QString& name, const QString& category) {
  if (!database_.addSeed(gzhId, name, category)) {
    appendLogKey(QStringLiteral("add_seed_failed"), database_.lastError());
    return;
  }
  refreshSeeds();
  appendLogKey(QStringLiteral("seed_saved"), name);
}

void MainWindow::removeSeedFromWidget(const QString& gzhId) {
  if (gzhId.isEmpty()) {
    appendLogKey(QStringLiteral("no_seed_selected"));
    return;
  }
  database_.removeSeed(gzhId);
  refreshSeeds();
  appendLogKey(QStringLiteral("seed_removed"), gzhId);
}

void MainWindow::exportArticlesCsv() {
  const QString path = QFileDialog::getSaveFileName(this, "Export CSV", "articles.csv", "CSV (*.csv)");
  if (path.isEmpty()) return;
  QString error;
  if (!ExportController::exportArticlesCsv(database_.listArticles(), path, &error)) {
    appendLogKey(QStringLiteral("export_articles_csv_failed"), error);
    return;
  }
  appendLogKey(QStringLiteral("articles_csv_exported"), path);
}

void MainWindow::exportArticlesJson() {
  const QString path = QFileDialog::getSaveFileName(this, "Export JSON", "articles.json", "JSON (*.json)");
  if (path.isEmpty()) return;
  QString error;
  if (!ExportController::exportArticlesJson(database_.listArticles(), path, &error)) {
    appendLogKey(QStringLiteral("export_articles_json_failed"), error);
    return;
  }
  appendLogKey(QStringLiteral("articles_json_exported"), path);
}

void MainWindow::exportSeedsCsv() {
  const QString path = QFileDialog::getSaveFileName(this, "Export Seeds", "seeds.csv", "CSV (*.csv)");
  if (path.isEmpty()) return;
  QString error;
  if (!ExportController::exportSeedsCsv(database_.listSeeds(), path, &error)) {
    appendLogKey(QStringLiteral("export_seeds_failed"), error);
    return;
  }
  appendLogKey(QStringLiteral("seeds_csv_exported"), path);
}

void MainWindow::refreshAutoIngestionQueue() {
  autoIngestionWidget_->setQueue(autoIngestion_.tasks());
  autoIngestionWidget_->setRunning(autoIngestion_.isRunning());
}

void MainWindow::generateKeywordSearchUrls(const QString& keywords) {
  const QStringList parsed = KeywordDiscoveryController::parseKeywords(keywords);
  if (parsed.isEmpty()) {
    appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("请先输入至少一个关键词") : QStringLiteral("Enter at least one keyword first"));
    return;
  }
  QStringList urls;
  for (const QString& keyword : parsed) {
    urls.push_back(KeywordDiscoveryController::searchUrlForKeyword(keyword));
  }
  const QString message = urls.join(QStringLiteral("\n"));
  QMessageBox::information(this,
                           language_ == UiLanguage::Chinese ? QStringLiteral("关键词搜索入口") : QStringLiteral("Keyword search URLs"),
                           message);
  appendLog(language_ == UiLanguage::Chinese
                ? QStringLiteral("已生成 %1 个关键词搜索入口，优先用外部适配器抓取这些搜索结果。原始搜索结果不要入库，只导入精简 JSON。").arg(urls.size())
                : QStringLiteral("Generated %1 keyword search URL(s). Use an external adapter to collect search results, then import sanitized JSON only.").arg(urls.size()));
}

void MainWindow::autoSearchKeywords(const QString& keywords, int maxCandidatesPerKeyword) {
  startAutoAfterKeywordSearch_ = false;
  pendingKeywordCriteria_ = KeywordHotCriteria{};
  keywordDiscovery_.setMaxResultsPerKeyword(maxCandidatesPerKeyword);
  keywordDiscovery_.searchKeywords(keywords);
}

void MainWindow::startKeywordAutoIngestion(const QString& keywords, int maxCandidatesPerKeyword, const KeywordHotCriteria& criteria) {
  lastPhoneReport_ = phoneDiagnostics_.runDiagnostics(phoneDiagnosticsWidget_->selectedSerial(), wechatConfig_->bridgePort(),
                                                     phoneDiagnosticsWidget_->proxyPort(), false, phoneDiagnosticsWidget_->testUrl());
  QString preflightReason;
  if (!PhoneDiagnosticsController::isCoreReady(lastPhoneReport_, &preflightReason)) {
    phoneDiagnosticsWidget_->setReport(lastPhoneReport_);
    if (dockTabs_ != nullptr) dockTabs_->setCurrentWidget(phoneDiagnosticsWidget_);
    appendLog(language_ == UiLanguage::Chinese
                  ? QStringLiteral("关键词自动采集启动失败：手机接入诊断未通过：%1").arg(preflightReason)
                  : QStringLiteral("Keyword auto-ingestion start failed: phone diagnostics preflight failed: %1").arg(preflightReason));
    return;
  }
  startAutoAfterKeywordSearch_ = true;
  pendingKeywordCriteria_ = criteria;
  autoIngestion_.setEnabled(true);
  autoIngestion_.setIntervalSeconds(autoIngestionWidget_->intervalSeconds());
  autoIngestion_.setMaxAttempts(autoIngestionWidget_->maxAttempts());
  keywordDiscovery_.setMaxResultsPerKeyword(maxCandidatesPerKeyword);
  keywordDiscovery_.searchKeywords(keywords);
}

void MainWindow::onKeywordSearchStarted() {
  keywordDiscoveryWidget_->setSearching(true);
  appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("关键词自动搜索开始") : QStringLiteral("Keyword auto-search started"));
}

void MainWindow::onKeywordSearchFinished(const QVector<KeywordDiscoveryResult>& results) {
  keywordResults_ = results;
  keywordDiscoveryWidget_->setSearching(false);
  keywordDiscoveryWidget_->setResults(keywordResults_);
  appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("关键词自动搜索完成，候选文章 %1 条").arg(results.size()) : QStringLiteral("Keyword auto-search finished, %1 candidate article(s)").arg(results.size()));
  if (!startAutoAfterKeywordSearch_) {
    return;
  }
  startAutoAfterKeywordSearch_ = false;
  const QString queueText = KeywordDiscoveryController::resultsToQueueText(
      keywordResults_, pendingKeywordCriteria_.minimumReadCount, pendingKeywordCriteria_.minimumLikeCount,
      pendingKeywordCriteria_.minimumCommentCount, pendingKeywordCriteria_.minimumHotScore);
  QString error;
  const int added = autoIngestion_.enqueueUrlsFromText(queueText, &error);
  refreshAutoIngestionQueue();
  appendLog(language_ == UiLanguage::Chinese
                ? QStringLiteral("关键词候选已自动加入采集队列：%1 条%2").arg(added).arg(error.isEmpty() ? QString() : QStringLiteral("；跳过：%1").arg(error))
                : QStringLiteral("Keyword candidates automatically enqueued: %1%2").arg(added).arg(error.isEmpty() ? QString() : QStringLiteral("; skipped: %1").arg(error)));
  if (added > 0) {
    autoIngestion_.start();
    refreshAutoIngestionQueue();
  }
}

void MainWindow::importKeywordDiscoveryResults() {
  const QString path = QFileDialog::getOpenFileName(this,
                                                    language_ == UiLanguage::Chinese ? QStringLiteral("导入关键词发现结果") : QStringLiteral("Import keyword discovery results"),
                                                    QString(), QStringLiteral("JSON (*.json)"));
  if (path.isEmpty()) return;
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("关键词结果读取失败：%1").arg(file.errorString()) : QStringLiteral("Keyword result read failed: %1").arg(file.errorString()));
    return;
  }
  QString error;
  keywordResults_ = KeywordDiscoveryController::parseResultsJson(file.readAll(), &error);
  if (keywordResults_.isEmpty()) {
    appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("关键词结果导入失败：%1").arg(error) : QStringLiteral("Keyword result import failed: %1").arg(error));
    return;
  }
  keywordDiscoveryWidget_->setResults(keywordResults_);
  appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("已导入关键词发现结果：%1 条").arg(keywordResults_.size()) : QStringLiteral("Imported keyword discovery results: %1").arg(keywordResults_.size()));
}

void MainWindow::enqueueKeywordDiscoveryResults(const KeywordHotCriteria& criteria) {
  if (keywordResults_.isEmpty()) {
    appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("没有可加入队列的关键词发现结果") : QStringLiteral("No keyword discovery results to enqueue"));
    return;
  }
  const QString queueText = KeywordDiscoveryController::resultsToQueueText(
      keywordResults_, criteria.minimumReadCount, criteria.minimumLikeCount,
      criteria.minimumCommentCount, criteria.minimumHotScore);
  QString error;
  const int added = autoIngestion_.enqueueUrlsFromText(queueText, &error);
  refreshAutoIngestionQueue();
  appendLog(language_ == UiLanguage::Chinese
                ? QStringLiteral("关键词爆款结果已加入自动采集队列：%1 条%2").arg(added).arg(error.isEmpty() ? QString() : QStringLiteral("；跳过：%1").arg(error))
                : QStringLiteral("Keyword hot results enqueued: %1%2").arg(added).arg(error.isEmpty() ? QString() : QStringLiteral("; skipped: %1").arg(error)));
}

void MainWindow::addAutoIngestionUrls(const QString& text) {
  QString error;
  const int added = autoIngestion_.enqueueUrlsFromText(text, &error);
  appendLog(language_ == UiLanguage::Chinese
                ? QStringLiteral("自动采集队列新增 %1 条 URL%2").arg(added).arg(error.isEmpty() ? QString() : QStringLiteral("；跳过：%1").arg(error))
                : QStringLiteral("Added %1 URL(s) to auto-ingestion queue%2").arg(added).arg(error.isEmpty() ? QString() : QStringLiteral("; skipped: %1").arg(error)));
  refreshAutoIngestionQueue();
}

void MainWindow::startAutoIngestion() {
  autoIngestion_.setEnabled(autoIngestionWidget_->automationEnabled());
  autoIngestion_.setIntervalSeconds(autoIngestionWidget_->intervalSeconds());
  autoIngestion_.setMaxAttempts(autoIngestionWidget_->maxAttempts());
  autoIngestion_.start();
  refreshAutoIngestionQueue();
}

void MainWindow::stopAutoIngestion() {
  autoIngestion_.stop();
  refreshAutoIngestionQueue();
}

void MainWindow::runNextAutoIngestionTask() {
  autoIngestion_.setEnabled(autoIngestionWidget_->automationEnabled());
  autoIngestion_.setIntervalSeconds(autoIngestionWidget_->intervalSeconds());
  autoIngestion_.setMaxAttempts(autoIngestionWidget_->maxAttempts());
  autoIngestion_.runNextNow();
  refreshAutoIngestionQueue();
}

void MainWindow::clearCompletedAutoIngestionTasks() {
  autoIngestion_.clearCompleted();
  refreshAutoIngestionQueue();
}

void MainWindow::clearAllAutoIngestionTasks() {
  autoIngestion_.clearAll();
  refreshAutoIngestionQueue();
}

void MainWindow::saveAutoIngestionQueue() {
  const QString path = QFileDialog::getSaveFileName(this, "Save Auto Ingestion Queue", "auto-ingestion-queue.json", "JSON (*.json)");
  if (path.isEmpty()) return;
  QString error;
  if (!autoIngestion_.saveQueue(path, &error)) {
    appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("自动采集队列保存失败：%1").arg(error) : QStringLiteral("Auto-ingestion queue save failed: %1").arg(error));
    return;
  }
  appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("自动采集队列已保存：%1").arg(path) : QStringLiteral("Auto-ingestion queue saved: %1").arg(path));
}

void MainWindow::loadAutoIngestionQueue() {
  const QString path = QFileDialog::getOpenFileName(this, "Load Auto Ingestion Queue", QString(), "JSON (*.json)");
  if (path.isEmpty()) return;
  QString error;
  if (!autoIngestion_.loadQueue(path, &error)) {
    appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("自动采集队列加载失败：%1").arg(error) : QStringLiteral("Auto-ingestion queue load failed: %1").arg(error));
    return;
  }
  appendLog(language_ == UiLanguage::Chinese ? QStringLiteral("自动采集队列已加载：%1").arg(path) : QStringLiteral("Auto-ingestion queue loaded: %1").arg(path));
  refreshAutoIngestionQueue();
}

void MainWindow::showAboutDialog() {
  QMessageBox::about(this, UiText::text("app.title", language_),
                     UiText::text("manual.body", language_));
}

void MainWindow::refreshSeeds() {
  seeds_->setSeeds(database_.listSeeds());
}

void MainWindow::refreshData() {
  viewer_->proxy()->setWeights(controls_->engagementWeight(), controls_->commentWeight(),
                               controls_->frequencyWeight());
  viewer_->proxy()->setMinimums(controls_->minimumRead(), 1.0);
  const auto rows = database_.listArticles();
  viewer_->setRecords(rows);
  productionSuiteWidget_->setRecords(rows);
  productionSuiteWidget_->setQueueStats(autoIngestion_.pendingCount(), 0);
  productionSuiteWidget_->setReadiness(lastPhoneReport_.overallStatus == QStringLiteral("ready") || lastPhoneReport_.overallStatus == QStringLiteral("warning"), true, database_.lastError().isEmpty());
  refreshSeeds();

  double topScore = 0.0;
  for (int index = 0; index < rows.size(); ++index) {
    topScore = qMax(topScore, viewer_->proxy()->scoreForSourceRow(index));
  }
  dashboard_->setMetrics(database_.listSeeds().size(), rows.size(), topScore);
}

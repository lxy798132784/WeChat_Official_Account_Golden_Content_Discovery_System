#include "MainWindow.h"

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QShortcut>
#include <QStatusBar>
#include <QTabWidget>
#include <QUrl>
#include <QVBoxLayout>

#include "ControlPanelWidget.h"
#include "DashboardWidget.h"
#include "DataViewerWidget.h"
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
  helpMenu_ = menuBar()->addMenu(QString());
  languageAction_ = helpMenu_->addAction(QString(), this, &MainWindow::toggleLanguage);
  aboutAction_ = helpMenu_->addAction(QString(), this, &MainWindow::showAboutDialog);

  new QShortcut(QKeySequence(Qt::Key_Space), this, SLOT(previewSelectedArticle()));
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this, SLOT(starSelectedSeed()));
  new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(resetControls()));

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
    dockTabs_->setTabText(1, UiText::text("tab.seeds", language_));
    dockTabs_->setTabText(2, UiText::text("tab.wechat", language_));
    dockTabs_->setTabText(3, UiText::text("tab.logs", language_));
    dockTabs_->setTabText(4, UiText::text("tab.guide", language_));
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
  if (languageAction_ != nullptr) { languageAction_->setText(UiText::text("action.language", language_)); languageAction_->setToolTip(UiText::text("tip.language", language_)); }
  if (aboutAction_ != nullptr) aboutAction_->setText(UiText::text("action.about", language_));
  dashboard_->setLanguage(language_);
  controls_->setLanguage(language_);
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
  if (key == QStringLiteral("provider_started")) return zh ? QStringLiteral("Provider 已启动：%1").arg(value) : QStringLiteral("Provider started: %1").arg(value);
  if (key == QStringLiteral("loaded_providers")) return zh ? QStringLiteral("已加载 Provider 数量：%1").arg(value) : QStringLiteral("Loaded providers: %1").arg(value);
  if (key == QStringLiteral("ingested_provider_records")) return zh ? QStringLiteral("已接入 Provider 记录：%1").arg(value) : QStringLiteral("Ingested provider records: %1").arg(value);
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
  refreshSeeds();

  double topScore = 0.0;
  for (int index = 0; index < rows.size(); ++index) {
    topScore = qMax(topScore, viewer_->proxy()->scoreForSourceRow(index));
  }
  dashboard_->setMetrics(database_.listSeeds().size(), rows.size(), topScore);
}

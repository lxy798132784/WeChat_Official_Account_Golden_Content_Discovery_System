#include "MainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QShortcut>
#include <QStatusBar>
#include <QUrl>
#include <QVBoxLayout>

#include "ControlPanelWidget.h"
#include "DashboardWidget.h"
#include "DataViewerWidget.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      dashboard_(new DashboardWidget(this)),
      controls_(new ControlPanelWidget(this)),
      viewer_(new DataViewerWidget(this)) {
  setWindowTitle("Premium Content Radar / 全网黄金内容雷达");
  resize(1280, 760);

  auto* central = new QWidget(this);
  auto* layout = new QVBoxLayout(central);
  layout->setContentsMargins(12, 12, 12, 12);
  layout->setSpacing(8);
  layout->addWidget(dashboard_);
  layout->addWidget(viewer_);
  setCentralWidget(central);

  auto* dock = new QDockWidget("Controls / 控制", this);
  dock->setWidget(controls_);
  addDockWidget(Qt::LeftDockWidgetArea, dock);

  database_.open("wechat_radar.db");
  connect(controls_, &ControlPanelWidget::filtersChanged, this, &MainWindow::refreshData);
  connect(&pluginDrainTimer_, &QTimer::timeout, this, &MainWindow::drainPluginRecords);

  menuBar()->addAction("Load Samples / 加载示例", this, &MainWindow::loadSampleData);
  menuBar()->addAction("Load Plugins / 加载插件", this, &MainWindow::loadPlugins);
  menuBar()->addAction("Preview / 预览", this, &MainWindow::previewSelectedArticle);
  menuBar()->addAction("Star Seed / 标星账号", this, &MainWindow::starSelectedSeed);

  new QShortcut(QKeySequence(Qt::Key_Space), this, SLOT(previewSelectedArticle()));
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this, SLOT(starSelectedSeed()));
  new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(resetControls()));

  applyTheme();
  loadSampleData();
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
      "QDockWidget::title{background:#161b22;padding:8px;}"
      "QMenuBar,QMenu{background:#161b22;color:#f0f6fc;}"
      "QStatusBar{background:#0d1117;color:#8b949e;}");
}

QString MainWindow::pluginDirectory() const {
  return QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("plugins");
}

void MainWindow::loadSampleData() {
  database_.addSeed("gh_seed_001", "Deep Insight Lab", "Technology");
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
  database_.enqueueArticle(second);
  database_.flush();
  refreshData();
  statusBar()->showMessage("Sample records loaded / 示例数据已加载");
}

void MainWindow::loadPlugins() {
  const int count = pluginManager_.loadFromDirectory(pluginDirectory());
  for (IContentProvider* provider : pluginManager_.providers()) {
    QString errorMessage;
    if (!provider->start(&errorMessage)) {
      statusBar()->showMessage(QStringLiteral("Plugin start failed: %1").arg(errorMessage));
    }
  }
  statusBar()->showMessage(QStringLiteral("Loaded providers / 已加载供应商: %1").arg(count));
}

void MainWindow::drainPluginRecords() {
  bool changed = false;
  for (IContentProvider* provider : pluginManager_.providers()) {
    for (const ContentRecord& record : provider->drainRecords()) {
      database_.enqueueArticle(record);
      changed = true;
    }
  }
  if (changed) {
    database_.flush();
    refreshData();
  }
}

void MainWindow::previewSelectedArticle() {
  if (!viewer_->hasSelection()) {
    statusBar()->showMessage("No article selected / 尚未选择文章");
    return;
  }
  const ContentRecord record = viewer_->selectedRecord();
  if (!record.url.isEmpty()) {
    QDesktopServices::openUrl(QUrl(record.url));
  }
  QMessageBox::information(this, "Article Detail / 文章详情",
                           QString("%1\n%2\nRead: %3\nLike: %4\nComment: %5")
                               .arg(record.title, record.url)
                               .arg(record.readNum)
                               .arg(record.likeNum + record.oldLikeNum)
                               .arg(record.commentNum));
}

void MainWindow::starSelectedSeed() {
  if (!viewer_->hasSelection()) {
    statusBar()->showMessage("No publisher selected / 尚未选择账号");
    return;
  }
  const ContentRecord record = viewer_->selectedRecord();
  database_.addSeed(record.gzhId, record.accountName, record.category);
  statusBar()->showMessage(QStringLiteral("Starred seed / 已标星账号: %1").arg(record.accountName));
}

void MainWindow::resetControls() {
  controls_->clearSearch();
  controls_->resetDefaults();
  refreshData();
  statusBar()->showMessage("Controls reset / 控件已重置");
}

void MainWindow::refreshData() {
  viewer_->proxy()->setWeights(controls_->engagementWeight(), controls_->commentWeight(),
                               controls_->frequencyWeight());
  viewer_->proxy()->setMinimums(controls_->minimumRead(), 1.0);
  const auto rows = database_.listArticles();
  viewer_->setRecords(rows);

  double topScore = 0.0;
  for (int index = 0; index < rows.size(); ++index) {
    topScore = qMax(topScore, viewer_->proxy()->scoreForSourceRow(index));
  }
  dashboard_->setMetrics(pluginManager_.providers().size(), rows.size(), topScore);
}

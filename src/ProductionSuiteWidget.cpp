#include "ProductionSuiteWidget.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {
QString localizedSuiteStatus(const QString& status, UiLanguage language) {
  if (language != UiLanguage::Chinese) return status;
  if (status == QStringLiteral("pass")) return QStringLiteral("通过");
  if (status == QStringLiteral("warn")) return QStringLiteral("提醒");
  if (status == QStringLiteral("fail")) return QStringLiteral("失败");
  if (status == QStringLiteral("healthy")) return QStringLiteral("健康");
  if (status == QStringLiteral("warning")) return QStringLiteral("有风险");
  if (status == QStringLiteral("blocked")) return QStringLiteral("阻塞");
  return status;
}

QString localizedProxyStepName(const QString& id, UiLanguage language) {
  if (language != UiLanguage::Chinese) return id;
  if (id == QStringLiteral("proxy_port")) return QStringLiteral("本地代理端口");
  if (id == QStringLiteral("bridge_port")) return QStringLiteral("本地桥端口");
  if (id == QStringLiteral("phone_to_pc")) return QStringLiteral("手机访问电脑");
  if (id == QStringLiteral("metrics_hit")) return QStringLiteral("指标接口命中");
  if (id == QStringLiteral("comments_hit")) return QStringLiteral("评论接口命中");
  return id;
}

QString localizedHealthName(const QString& id, UiLanguage language) {
  if (language != UiLanguage::Chinese) return id;
  if (id == QStringLiteral("phone")) return QStringLiteral("手机接入");
  if (id == QStringLiteral("proxy")) return QStringLiteral("本地代理");
  if (id == QStringLiteral("bridge")) return QStringLiteral("本地桥");
  if (id == QStringLiteral("database")) return QStringLiteral("数据库");
  if (id == QStringLiteral("queue")) return QStringLiteral("任务队列");
  return id;
}

QString localizedSuiteText(QString text, UiLanguage language) {
  if (language != UiLanguage::Chinese) return text;
  text.replace(QStringLiteral("Proxy port is not configured"), QStringLiteral("未配置代理端口"));
  text.replace(QStringLiteral("Set the local proxy listening port. Use 0 only when you intentionally skip proxy checks."), QStringLiteral("设置本地代理监听端口。只有明确跳过代理检测时才使用 0。"));
  text.replace(QStringLiteral("Local bridge port:"), QStringLiteral("本地桥端口："));
  text.replace(QStringLiteral("Bridge port is invalid"), QStringLiteral("本地桥端口无效"));
  text.replace(QStringLiteral("Open WeChat Integration and set a valid local bridge port."), QStringLiteral("打开微信接入页，设置有效的本地桥端口。"));
  text.replace(QStringLiteral("Phone can reach this computer"), QStringLiteral("手机可以访问这台电脑"));
  text.replace(QStringLiteral("Phone reachability is not verified"), QStringLiteral("尚未验证手机能否访问这台电脑"));
  text.replace(QStringLiteral("Configure the phone Wi-Fi proxy and open the local ping/check URL from the phone."), QStringLiteral("配置手机无线网络代理，并从手机打开本地检测地址。"));
  text.replace(QStringLiteral("/mp/getappmsgext has been observed"), QStringLiteral("已观察到文章指标接口"));
  text.replace(QStringLiteral("No metric interface hit yet"), QStringLiteral("尚未命中文章指标接口"));
  text.replace(QStringLiteral("Open a WeChat article on the test phone and check whether the proxy adapter forwards compact JSON."), QStringLiteral("在测试手机打开一篇微信文章，确认代理适配器会转发精简数据。"));
  text.replace(QStringLiteral("/mp/appmsg_comment has been observed"), QStringLiteral("已观察到评论接口"));
  text.replace(QStringLiteral("No comment interface hit yet"), QStringLiteral("尚未命中评论接口"));
  text.replace(QStringLiteral("Open the comment area if comment density is required for scoring."), QStringLiteral("如果评分需要评论密度，请打开文章评论区。"));
  text.replace(QStringLiteral("Phone preflight passed"), QStringLiteral("手机预检通过"));
  text.replace(QStringLiteral("Phone preflight is not ready"), QStringLiteral("手机预检未就绪"));
  text.replace(QStringLiteral("Proxy adapter is reachable"), QStringLiteral("代理适配器可访问"));
  text.replace(QStringLiteral("Proxy adapter is not fully verified"), QStringLiteral("代理适配器尚未完整验证"));
  text.replace(QStringLiteral("Local bridge is reachable"), QStringLiteral("本地桥可访问"));
  text.replace(QStringLiteral("Local bridge is not reachable"), QStringLiteral("本地桥不可访问"));
  text.replace(QStringLiteral("Database is writable"), QStringLiteral("数据库可写"));
  text.replace(QStringLiteral("Database is not writable"), QStringLiteral("数据库不可写"));
  text.replace(QStringLiteral("pending"), QStringLiteral("待处理"));
  text.replace(QStringLiteral("failed"), QStringLiteral("失败"));
  return text;
}
}  // namespace

ProductionSuiteWidget::ProductionSuiteWidget(QWidget* parent)
    : QWidget(parent), tabs_(new QTabWidget(this)) {
  auto* layout = new QVBoxLayout(this);
  layout->addWidget(tabs_);
  buildProxyTab();
  buildReplayTab();
  buildHealthTab();
  buildScoringTab();
  buildWorkspaceTab();
  buildPrivacyTab();
  setLanguage(language_);
}

void ProductionSuiteWidget::buildProxyTab() {
  auto* page = new QWidget(this);
  auto* layout = new QVBoxLayout(page);
  proxyIntro_ = new QLabel(page);
  proxyIntro_->setWordWrap(true);
  proxyPortLabel_ = new QLabel(page);
  bridgePortLabel_ = new QLabel(page);
  proxyPort_ = new QSpinBox(page);
  proxyPort_->setRange(0, 65535);
  proxyPort_->setValue(8080);
  bridgePort_ = new QSpinBox(page);
  bridgePort_->setRange(1, 65535);
  bridgePort_->setValue(9000);
  phoneReachable_ = new QCheckBox(page);
  metricHit_ = new QCheckBox(page);
  commentHit_ = new QCheckBox(page);
  runProxyButton_ = new QPushButton(page);
  proxyTable_ = new QTableWidget(page);
  proxyReport_ = new QPlainTextEdit(page);
  proxyReport_->setReadOnly(true);
  auto* form = new QFormLayout();
  form->addRow(proxyPortLabel_, proxyPort_);
  form->addRow(bridgePortLabel_, bridgePort_);
  layout->addWidget(proxyIntro_);
  layout->addLayout(form);
  layout->addWidget(phoneReachable_);
  layout->addWidget(metricHit_);
  layout->addWidget(commentHit_);
  layout->addWidget(runProxyButton_);
  proxyTable_->setColumnCount(4);
  proxyTable_->horizontalHeader()->setStretchLastSection(true);
  layout->addWidget(proxyTable_);
  layout->addWidget(proxyReport_);
  connect(runProxyButton_, &QPushButton::clicked, this, &ProductionSuiteWidget::runProxyWizard);
  tabs_->addTab(page, QString());
}

void ProductionSuiteWidget::buildReplayTab() {
  auto* page = new QWidget(this);
  auto* layout = new QVBoxLayout(page);
  replayIntro_ = new QLabel(page);
  replayIntro_->setWordWrap(true);
  replayInput_ = new QTextEdit(page);
  replayInput_->setAcceptRichText(false);
  replayInput_->setMinimumHeight(110);
  replayButton_ = new QPushButton(page);
  replayTable_ = new QTableWidget(page);
  replayTable_->setColumnCount(5);
  replayTable_->horizontalHeader()->setStretchLastSection(true);
  layout->addWidget(replayIntro_);
  layout->addWidget(replayInput_);
  layout->addWidget(replayButton_);
  layout->addWidget(replayTable_);
  connect(replayButton_, &QPushButton::clicked, this, &ProductionSuiteWidget::replaySamples);
  tabs_->addTab(page, QString());
}

void ProductionSuiteWidget::buildHealthTab() {
  auto* page = new QWidget(this);
  auto* layout = new QVBoxLayout(page);
  healthIntro_ = new QLabel(page);
  healthIntro_->setWordWrap(true);
  refreshHealthButton_ = new QPushButton(page);
  healthTable_ = new QTableWidget(page);
  healthTable_->setColumnCount(3);
  healthTable_->horizontalHeader()->setStretchLastSection(true);
  layout->addWidget(healthIntro_);
  layout->addWidget(refreshHealthButton_);
  layout->addWidget(healthTable_);
  connect(refreshHealthButton_, &QPushButton::clicked, this, &ProductionSuiteWidget::refreshHealth);
  tabs_->addTab(page, QString());
}

void ProductionSuiteWidget::buildScoringTab() {
  auto* page = new QWidget(this);
  auto* layout = new QVBoxLayout(page);
  scoringIntro_ = new QLabel(page);
  scoringIntro_->setWordWrap(true);
  readWeight_ = new QDoubleSpinBox(page);
  likeWeight_ = new QDoubleSpinBox(page);
  commentWeight_ = new QDoubleSpinBox(page);
  oldLikeWeight_ = new QDoubleSpinBox(page);
  originalWeight_ = new QDoubleSpinBox(page);
  for (auto* spin : {readWeight_, likeWeight_, commentWeight_, oldLikeWeight_, originalWeight_}) {
    spin->setRange(0.0, 10000.0);
    spin->setDecimals(2);
  }
  readWeight_->setValue(1.0);
  likeWeight_->setValue(20.0);
  commentWeight_->setValue(50.0);
  oldLikeWeight_->setValue(10.0);
  originalWeight_->setValue(8.0);
  scorePreviewButton_ = new QPushButton(page);
  scorePreview_ = new QPlainTextEdit(page);
  scorePreview_->setReadOnly(true);
  auto* form = new QFormLayout();
  form->addRow(UiText::text(QStringLiteral("suite.score_read"), language_), readWeight_);
  form->addRow(UiText::text(QStringLiteral("suite.score_like"), language_), likeWeight_);
  form->addRow(UiText::text(QStringLiteral("suite.score_comment"), language_), commentWeight_);
  form->addRow(UiText::text(QStringLiteral("suite.score_old_like"), language_), oldLikeWeight_);
  form->addRow(UiText::text(QStringLiteral("suite.score_original"), language_), originalWeight_);
  layout->addWidget(scoringIntro_);
  layout->addLayout(form);
  layout->addWidget(scorePreviewButton_);
  layout->addWidget(scorePreview_);
  connect(scorePreviewButton_, &QPushButton::clicked, this, &ProductionSuiteWidget::updateScorePreview);
  tabs_->addTab(page, QString());
}

void ProductionSuiteWidget::buildWorkspaceTab() {
  auto* page = new QWidget(this);
  auto* layout = new QVBoxLayout(page);
  workspaceIntro_ = new QLabel(page);
  workspaceIntro_->setWordWrap(true);
  workspaceName_ = new QLineEdit(page);
  reportButton_ = new QPushButton(page);
  snapshotButton_ = new QPushButton(page);
  reportOutput_ = new QPlainTextEdit(page);
  reportOutput_->setReadOnly(true);
  layout->addWidget(workspaceIntro_);
  layout->addWidget(workspaceName_);
  layout->addWidget(reportButton_);
  layout->addWidget(snapshotButton_);
  layout->addWidget(reportOutput_);
  connect(reportButton_, &QPushButton::clicked, this, &ProductionSuiteWidget::generateReport);
  connect(snapshotButton_, &QPushButton::clicked, this, &ProductionSuiteWidget::exportSnapshot);
  tabs_->addTab(page, QString());
}

void ProductionSuiteWidget::buildPrivacyTab() {
  auto* page = new QWidget(this);
  auto* layout = new QVBoxLayout(page);
  privacyIntro_ = new QLabel(page);
  privacyIntro_->setWordWrap(true);
  privacyText_ = new QPlainTextEdit(page);
  privacyText_->setReadOnly(true);
  layout->addWidget(privacyIntro_);
  layout->addWidget(privacyText_);
  tabs_->addTab(page, QString());
}

void ProductionSuiteWidget::setLanguage(UiLanguage language) {
  language_ = language;
  tabs_->setTabText(0, UiText::text(QStringLiteral("suite.tab.proxy"), language_));
  tabs_->setTabText(1, UiText::text(QStringLiteral("suite.tab.replay"), language_));
  tabs_->setTabText(2, UiText::text(QStringLiteral("suite.tab.health"), language_));
  tabs_->setTabText(3, UiText::text(QStringLiteral("suite.tab.scoring"), language_));
  tabs_->setTabText(4, UiText::text(QStringLiteral("suite.tab.workspace"), language_));
  tabs_->setTabText(5, UiText::text(QStringLiteral("suite.tab.privacy"), language_));
  proxyIntro_->setText(UiText::text(QStringLiteral("suite.proxy_intro"), language_));
  proxyPortLabel_->setText(UiText::text(QStringLiteral("suite.proxy_port"), language_));
  bridgePortLabel_->setText(UiText::text(QStringLiteral("suite.bridge_port"), language_));
  phoneReachable_->setText(UiText::text(QStringLiteral("suite.phone_reachable"), language_));
  metricHit_->setText(UiText::text(QStringLiteral("suite.metric_hit"), language_));
  commentHit_->setText(UiText::text(QStringLiteral("suite.comment_hit"), language_));
  runProxyButton_->setText(UiText::text(QStringLiteral("suite.run_proxy"), language_));
  replayIntro_->setText(UiText::text(QStringLiteral("suite.replay_intro"), language_));
  replayInput_->setPlaceholderText(UiText::text(QStringLiteral("suite.replay_placeholder"), language_));
  replayButton_->setText(UiText::text(QStringLiteral("suite.replay_run"), language_));
  healthIntro_->setText(UiText::text(QStringLiteral("suite.health_intro"), language_));
  refreshHealthButton_->setText(UiText::text(QStringLiteral("suite.health_refresh"), language_));
  scoringIntro_->setText(UiText::text(QStringLiteral("suite.scoring_intro"), language_));
  scorePreviewButton_->setText(UiText::text(QStringLiteral("suite.score_preview"), language_));
  workspaceIntro_->setText(UiText::text(QStringLiteral("suite.workspace_intro"), language_));
  workspaceName_->setPlaceholderText(UiText::text(QStringLiteral("suite.workspace_placeholder"), language_));
  reportButton_->setText(UiText::text(QStringLiteral("suite.report_generate"), language_));
  snapshotButton_->setText(UiText::text(QStringLiteral("suite.snapshot_export"), language_));
  privacyIntro_->setText(UiText::text(QStringLiteral("suite.privacy_intro"), language_));
  privacyText_->setPlainText(controller_.privacyBoundaryText(language_ == UiLanguage::Chinese));
  proxyTable_->setHorizontalHeaderLabels({UiText::text(QStringLiteral("suite.col.status"), language_), UiText::text(QStringLiteral("suite.col.item"), language_), UiText::text(QStringLiteral("suite.col.detail"), language_), UiText::text(QStringLiteral("suite.col.fix"), language_)});
  replayTable_->setHorizontalHeaderLabels({UiText::text(QStringLiteral("data.title"), language_), UiText::text(QStringLiteral("data.account"), language_), UiText::text(QStringLiteral("data.read"), language_), UiText::text(QStringLiteral("data.comment"), language_), UiText::text(QStringLiteral("data.url"), language_)});
  healthTable_->setHorizontalHeaderLabels({UiText::text(QStringLiteral("suite.col.status"), language_), UiText::text(QStringLiteral("suite.col.item"), language_), UiText::text(QStringLiteral("suite.col.detail"), language_)});
}

void ProductionSuiteWidget::setRecords(const QVector<ContentRecord>& records) { records_ = records; }
void ProductionSuiteWidget::setQueueStats(int pendingTasks, int failedTasks) { pendingTasks_ = pendingTasks; failedTasks_ = failedTasks; }
void ProductionSuiteWidget::setReadiness(bool phoneReady, bool bridgeReady, bool databaseReady) { phoneReady_ = phoneReady; bridgeReady_ = bridgeReady; databaseReady_ = databaseReady; }

void ProductionSuiteWidget::fillTable(QTableWidget* table, const QStringList& headers, const QList<QStringList>& rows) {
  table->setColumnCount(headers.size());
  table->setHorizontalHeaderLabels(headers);
  table->setRowCount(rows.size());
  for (int row = 0; row < rows.size(); ++row) {
    for (int col = 0; col < rows.at(row).size(); ++col) table->setItem(row, col, new QTableWidgetItem(rows.at(row).at(col)));
  }
  table->resizeColumnsToContents();
}

void ProductionSuiteWidget::runProxyWizard() {
  lastProxySteps_ = controller_.buildProxyWizard(proxyPort_->value(), bridgePort_->value(), phoneReachable_->isChecked(), metricHit_->isChecked(), commentHit_->isChecked());
  QList<QStringList> rows;
  for (const auto& step : lastProxySteps_) rows.push_back({localizedSuiteStatus(step.status, language_), localizedProxyStepName(step.name, language_), localizedSuiteText(step.detail, language_), localizedSuiteText(step.fixHint, language_)});
  fillTable(proxyTable_, {UiText::text(QStringLiteral("suite.col.status"), language_), UiText::text(QStringLiteral("suite.col.item"), language_), UiText::text(QStringLiteral("suite.col.detail"), language_), UiText::text(QStringLiteral("suite.col.fix"), language_)}, rows);
  proxyReport_->setPlainText(controller_.proxyWizardReport(lastProxySteps_, language_ == UiLanguage::Chinese));
}

void ProductionSuiteWidget::replaySamples() {
  QString error;
  const QVector<ContentRecord> parsed = controller_.parseReplaySamples(replayInput_->toPlainText(), &error);
  if (!parsed.isEmpty()) records_ = parsed;
  QList<QStringList> rows;
  for (const ContentRecord& r : parsed) rows.push_back({r.title, r.accountName, QString::number(r.readNum), QString::number(r.commentNum), r.url});
  fillTable(replayTable_, {UiText::text(QStringLiteral("data.title"), language_), UiText::text(QStringLiteral("data.account"), language_), UiText::text(QStringLiteral("data.read"), language_), UiText::text(QStringLiteral("data.comment"), language_), UiText::text(QStringLiteral("data.url"), language_)}, rows);
  if (!error.isEmpty()) emit logMessage(error);
}

void ProductionSuiteWidget::refreshHealth() {
  const bool proxyReady = metricHit_->isChecked() || commentHit_->isChecked();
  lastHealthItems_ = controller_.buildHealthItems(pendingTasks_, failedTasks_, phoneReady_, proxyReady, bridgeReady_, databaseReady_);
  QList<QStringList> rows;
  for (const auto& item : lastHealthItems_) rows.push_back({localizedSuiteStatus(item.status, language_), localizedHealthName(item.name, language_), localizedSuiteText(item.detail, language_)});
  fillTable(healthTable_, {UiText::text(QStringLiteral("suite.col.status"), language_), UiText::text(QStringLiteral("suite.col.item"), language_), UiText::text(QStringLiteral("suite.col.detail"), language_)}, rows);
}

ProductionSuiteController::ScoreProfile ProductionSuiteWidget::scoreProfile() const {
  ProductionSuiteController::ScoreProfile profile;
  profile.readWeight = readWeight_->value();
  profile.likeWeight = likeWeight_->value();
  profile.commentWeight = commentWeight_->value();
  profile.oldLikeWeight = oldLikeWeight_->value();
  profile.originalityWeight = originalWeight_->value();
  return profile;
}

void ProductionSuiteWidget::updateScorePreview() {
  const auto profile = scoreProfile();
  QString text;
  QTextStream stream(&text);
  stream << (language_ == UiLanguage::Chinese
                 ? QStringLiteral("评分 = 阅读*%1 + 点赞*%2 + 评论*%3 + 在看*%4 + 原创*%5\n\n")
                 : QStringLiteral("Score = read*%1 + like*%2 + comment*%3 + old_like*%4 + original*%5\n\n"))
                .arg(profile.readWeight).arg(profile.likeWeight).arg(profile.commentWeight).arg(profile.oldLikeWeight).arg(profile.originalityWeight);
  for (const ContentRecord& r : records_) stream << r.title << QStringLiteral(" => ") << controller_.scoreRecord(r, profile) << '\n';
  stream << '\n' << controller_.accountInsights(records_, language_ == UiLanguage::Chinese).join('\n') << '\n';
  stream << controller_.keywordInsights(records_, language_ == UiLanguage::Chinese).join('\n');
  scorePreview_->setPlainText(text);
}

void ProductionSuiteWidget::generateReport() {
  reportOutput_->setPlainText(controller_.generateMarkdownReport(records_, workspaceName_->text(), scoreProfile(), language_ == UiLanguage::Chinese));
}

void ProductionSuiteWidget::exportSnapshot() {
  refreshHealth();
  const QJsonObject snapshot = controller_.diagnosticSnapshot(lastProxySteps_, lastHealthItems_, scoreProfile());
  reportOutput_->setPlainText(QString::fromUtf8(QJsonDocument(snapshot).toJson(QJsonDocument::Indented)));
}

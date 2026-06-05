#include "UiText.h"

#include <QHash>

QString UiText::text(const QString& key, UiLanguage language) {
  static const QHash<QString, QString> en = {
      {"app.title", "Premium Content Radar"},
      {"dock.title", "Control Center"},
      {"tab.filters", "Filters"}, {"tab.auto", "Auto Ingestion"}, {"tab.seeds", "Seeds"}, {"tab.wechat", "WeChat"}, {"tab.logs", "Logs"}, {"tab.guide", "Guide"},
      {"menu.file", "File"}, {"menu.plugins", "Plugins"}, {"menu.actions", "Actions"}, {"menu.help", "Help"},
      {"action.load_samples", "Load Samples"}, {"action.export_articles_csv", "Export Articles CSV"}, {"action.export_articles_json", "Export Articles JSON"}, {"action.export_seeds_csv", "Export Seeds CSV"},
      {"action.load_plugins", "Load Plugins"}, {"action.preview", "Preview"}, {"action.star_seed", "Star Seed"}, {"action.bridge_smoke", "Send Bridge Smoke Payload"}, {"action.reset", "Reset Controls"},
      {"action.language", "中文"}, {"action.about", "About"}, {"action.manual", "Built-in User Manual"},
      {"tip.language", "Switch the whole software UI between English and Chinese. The current language is stored in local settings."},
      {"tip.load_samples", "Insert safe sample records into the local SQLite database for demo and testing."},
      {"tip.export_articles_csv", "Export the current article library to a CSV file."},
      {"tip.export_articles_json", "Export the current article library to a JSON file."},
      {"tip.load_plugins", "Load content provider plugins from the configured plugin directory."},
      {"tip.preview", "Open and inspect the currently selected article."},
      {"tip.star_seed", "Save the selected article publisher into the seed pool."},
      {"tip.bridge_smoke", "Send sample metric and comment payloads to the localhost bridge to verify ingestion."},
      {"tip.reset", "Reset filters, keyword, weights, and minimum read threshold."},
      {"filter.industry", "Industry"}, {"filter.keyword", "Keyword"}, {"filter.engagement", "Engagement weight"}, {"filter.comment", "Comment weight"}, {"filter.frequency", "Frequency weight"}, {"filter.min_read", "Minimum reads"},
      {"filter.keyword.placeholder", "Keyword"}, {"filter.all", "All"}, {"filter.technology", "Technology"}, {"filter.finance", "Finance"}, {"filter.lifestyle", "Lifestyle"},
      {"tip.filter.industry", "Filter articles by publisher category."}, {"tip.filter.keyword", "Search article title, account name, category, or URL."}, {"tip.filter.engagement", "Adjust how much reads and likes affect the score."}, {"tip.filter.comment", "Adjust how much comment density affects the score."}, {"tip.filter.frequency", "Adjust how much publishing frequency affects the score."}, {"tip.filter.min_read", "Hide articles below this read-count threshold."},
      {"data.title", "Title"}, {"data.account", "Account"}, {"data.category", "Category"}, {"data.publish_time", "Publish Time"}, {"data.read", "Read"}, {"data.like", "Like"}, {"data.old_like", "Old Like"}, {"data.comment", "Comment"}, {"data.freq", "30d Frequency"}, {"data.score", "Score"}, {"data.url", "URL"},
      {"tip.data.table", "Sortable article table. Select one row, then use Preview or Star Seed."},

      {"auto.enable", "Automation gate"}, {"auto.enable_checkbox", "Allow ADB URL opening for this session"}, {"auto.interval", "Open interval (seconds)"}, {"auto.attempts", "Max attempts per URL"}, {"auto.urls", "Article URL list"}, {"auto.queue", "Auto-ingestion queue"},
      {"auto.add_urls", "Add URLs"}, {"auto.start", "Start Auto Ingestion"}, {"auto.stop", "Stop"}, {"auto.run_next", "Run Next Now"}, {"auto.clear_completed", "Clear Opened"}, {"auto.clear_all", "Clear All"}, {"auto.save_queue", "Save Queue"}, {"auto.load_queue", "Load Queue"},
      {"auto.urls_placeholder", "Paste one WeChat article URL per line, for example https://mp.weixin.qq.com/s/xxxx . The scheduler opens these URLs on your connected test phone through adb; metrics still enter through the local proxy bridge."},
      {"auto.col.status", "Status"}, {"auto.col.attempts", "Attempts"}, {"auto.col.account", "Account"}, {"auto.col.category", "Category"}, {"auto.col.last", "Last Attempt"}, {"auto.col.url", "URL / Error"},
      {"tip.auto.enable", "Explicit safety gate. Keep it off unless adb is installed, a dedicated test phone is connected, and local proxy capture is configured."}, {"tip.auto.interval", "Delay between two ADB article-open commands. Use a conservative value to avoid UI churn and network pressure."}, {"tip.auto.attempts", "How many times a failed URL may be retried."}, {"tip.auto.urls", "Only WeChat article URLs are accepted. Cookies, tokens, headers, and raw captures are never required here."}, {"tip.auto.start", "Start opening queued URLs through adb. The proxy adapter must already be running to capture metrics."}, {"tip.auto.run_next", "Open exactly one runnable queued URL now for testing."},
      {"seed.gzh", "GZH ID"}, {"seed.name", "Name"}, {"seed.category", "Category"}, {"seed.count30d", "30d"}, {"seed.score", "Score"},
      {"seed.placeholder.gzh", "gh_xxx"}, {"seed.placeholder.name", "Account name"}, {"seed.add", "Add or Update Seed"}, {"seed.remove", "Remove Selected"}, {"seed.export", "Export Seeds CSV"},
      {"tip.seed.gzh", "Original Official Account ID used for deduplication."}, {"tip.seed.name", "Human-readable publisher name."}, {"tip.seed.category", "Publisher category used by filters and reports."}, {"tip.seed.add", "Create or update a seed publisher in the local database."}, {"tip.seed.remove", "Remove the selected seed from the local database."}, {"tip.seed.export", "Export all seeds to CSV."}, {"tip.seed.table", "Seed pool table. Select a row before removing it."},
      {"wechat.database", "SQLite database"}, {"wechat.plugin_dir", "Plugin directory"}, {"wechat.port", "Local bridge port"}, {"wechat.browse", "Browse"}, {"wechat.adb", "Enable ADB automation"}, {"wechat.samples", "Load sample data on startup"}, {"wechat.save", "Save runtime settings"}, {"wechat.test", "Send local bridge smoke payload"}, {"wechat.log", "Runtime log"},
      {"tip.wechat.database", "Local SQLite file that stores articles and seeds."}, {"tip.wechat.plugin_dir", "Directory containing provider plugins."}, {"tip.wechat.port", "Localhost TCP port for compact JSON ingestion."}, {"tip.wechat.adb", "Enable only with a dedicated test phone and explicit authorization."}, {"tip.wechat.samples", "Load demo records when the app starts."}, {"tip.wechat.save", "Persist settings to the local user configuration file."}, {"tip.wechat.test", "Verify the bridge with safe sample payloads."}, {"tip.wechat.log", "Sanitized runtime messages and ingestion events."},
      {"dash.accounts", "Scanned Accounts"}, {"dash.detections", "Premium Detections"}, {"dash.top_score", "Top Score"},
      {"manual.title", "Built-in User Manual"},
      {"manual.body", "1. Start with the WeChat tab. Confirm the database path, plugin directory, and bridge port. Keep ADB disabled unless you use a dedicated test phone.\n\n2. Click Save runtime settings, then Load Plugins. The WeChat provider starts a localhost bridge.\n\n3. Click Send Bridge Smoke Payload. If the article table grows, local ingestion works.\n\n4. For automatic ingestion, open the Auto Ingestion tab. Paste one WeChat article URL per line, enable the ADB safety gate, and click Run Next Now or Start Auto Ingestion. The scheduler only opens URLs on your connected test phone; metrics still enter through the local proxy bridge.\n\n5. Choose an ingestion option. Option A uses a lawful local proxy adapter. Option B imports manually prepared JSON or CSV. Option C uses ADB only to open pages on a test phone, then still imports metrics through A or B.\n\n6. Use Filters to tune category, keyword, score weights, and minimum reads. The score combines engagement, comment density, and publishing frequency.\n\n7. Select an article row. Preview opens the URL and shows the main metrics. Star Seed saves the publisher into the seed pool.\n\n8. Use Seeds to maintain target publishers. Add or update by GZH ID, name, and category. Export seeds when you need a backup.\n\n9. Use File exports to save articles as CSV or JSON. Review exports before production decisions.\n\n10. Security boundary: never store cookies, headers, tokens, certificates, or raw packet captures in the repository. Only compact sanitized JSON should be sent to 127.0.0.1."}
  };
  static const QHash<QString, QString> zh = {
      {"app.title", "全网黄金内容雷达"},
      {"dock.title", "控制中心"},
      {"tab.filters", "筛选"}, {"tab.auto", "自动采集"}, {"tab.seeds", "种子池"}, {"tab.wechat", "微信接入"}, {"tab.logs", "日志"}, {"tab.guide", "说明书"},
      {"menu.file", "文件"}, {"menu.plugins", "插件"}, {"menu.actions", "操作"}, {"menu.help", "帮助"},
      {"action.load_samples", "加载示例数据"}, {"action.export_articles_csv", "导出文章 CSV"}, {"action.export_articles_json", "导出文章 JSON"}, {"action.export_seeds_csv", "导出种子 CSV"},
      {"action.load_plugins", "加载插件"}, {"action.preview", "预览文章"}, {"action.star_seed", "收藏为种子"}, {"action.bridge_smoke", "发送本地桥冒烟载荷"}, {"action.reset", "重置控件"},
      {"action.language", "English"}, {"action.about", "关于"}, {"action.manual", "内置使用说明书"},
      {"tip.language", "在中文和英文之间切换整个软件界面，当前语言会保存到本地配置。"},
      {"tip.load_samples", "向本地 SQLite 数据库插入安全示例记录，用于演示和测试。"},
      {"tip.export_articles_csv", "把当前文章库导出为 CSV 文件。"},
      {"tip.export_articles_json", "把当前文章库导出为 JSON 文件。"},
      {"tip.load_plugins", "从已配置的插件目录加载内容提供者插件。"},
      {"tip.preview", "打开并查看当前选中文章的详情。"},
      {"tip.star_seed", "把当前选中文章的公众号保存到种子池。"},
      {"tip.bridge_smoke", "向本地桥发送示例指标和评论载荷，验证接入链路。"},
      {"tip.reset", "重置筛选条件、关键词、评分权重和最低阅读阈值。"},
      {"filter.industry", "行业"}, {"filter.keyword", "关键词"}, {"filter.engagement", "互动权重"}, {"filter.comment", "评论权重"}, {"filter.frequency", "频率权重"}, {"filter.min_read", "最低阅读"},
      {"filter.keyword.placeholder", "关键词"}, {"filter.all", "全部"}, {"filter.technology", "科技"}, {"filter.finance", "财经"}, {"filter.lifestyle", "生活"},
      {"tip.filter.industry", "按公众号分类筛选文章。"}, {"tip.filter.keyword", "搜索文章标题、账号名称、分类或链接。"}, {"tip.filter.engagement", "调整阅读数和点赞数对评分的影响。"}, {"tip.filter.comment", "调整评论密度对评分的影响。"}, {"tip.filter.frequency", "调整发文频率对评分的影响。"}, {"tip.filter.min_read", "隐藏阅读数低于该阈值的文章。"},
      {"data.title", "标题"}, {"data.account", "账号"}, {"data.category", "分类"}, {"data.publish_time", "发布时间"}, {"data.read", "阅读"}, {"data.like", "点赞"}, {"data.old_like", "在看"}, {"data.comment", "评论"}, {"data.freq", "30天频率"}, {"data.score", "评分"}, {"data.url", "链接"},
      {"tip.data.table", "可排序文章表格。先选中一行，再使用预览文章或收藏为种子。"},

      {"auto.enable", "自动化安全开关"}, {"auto.enable_checkbox", "允许本次会话使用 ADB 打开 URL"}, {"auto.interval", "打开间隔（秒）"}, {"auto.attempts", "每条 URL 最大尝试次数"}, {"auto.urls", "文章 URL 列表"}, {"auto.queue", "自动采集队列"},
      {"auto.add_urls", "添加 URL"}, {"auto.start", "开始自动采集"}, {"auto.stop", "停止"}, {"auto.run_next", "立即执行下一条"}, {"auto.clear_completed", "清理已打开"}, {"auto.clear_all", "清空全部"}, {"auto.save_queue", "保存队列"}, {"auto.load_queue", "加载队列"},
      {"auto.urls_placeholder", "每行粘贴一个微信文章链接，例如 https://mp.weixin.qq.com/s/xxxx。调度器会通过 adb 在已连接测试手机上逐篇打开；阅读、点赞、评论等指标仍由本地代理桥接入。"},
      {"auto.col.status", "状态"}, {"auto.col.attempts", "尝试"}, {"auto.col.account", "账号"}, {"auto.col.category", "分类"}, {"auto.col.last", "上次尝试"}, {"auto.col.url", "链接 / 错误"},
      {"tip.auto.enable", "显式安全开关。只有在已安装 adb、已连接专用测试手机、并配置好本地代理抓取时才打开。"}, {"tip.auto.interval", "两次 ADB 打开文章命令之间的间隔。建议保守设置，避免界面抖动和网络压力。"}, {"tip.auto.attempts", "单条 URL 失败后最多重试多少次。"}, {"tip.auto.urls", "这里只接受微信文章链接，不需要 Cookie、Token、Header 或原始抓包。"}, {"tip.auto.start", "开始用 adb 逐篇打开队列 URL。代理适配器必须已经运行，才能捕获指标。"}, {"tip.auto.run_next", "只立即打开一条可执行 URL，用于测试链路。"},
      {"seed.gzh", "公众号ID"}, {"seed.name", "名称"}, {"seed.category", "分类"}, {"seed.count30d", "30天"}, {"seed.score", "评分"},
      {"seed.placeholder.gzh", "gh_xxx"}, {"seed.placeholder.name", "账号名称"}, {"seed.add", "添加或更新种子"}, {"seed.remove", "删除选中"}, {"seed.export", "导出种子 CSV"},
      {"tip.seed.gzh", "公众号原始 ID，用于去重。"}, {"tip.seed.name", "便于识别的公众号名称。"}, {"tip.seed.category", "用于筛选和报告的公众号分类。"}, {"tip.seed.add", "在本地数据库中创建或更新种子公众号。"}, {"tip.seed.remove", "从本地数据库删除选中的种子。"}, {"tip.seed.export", "把全部种子导出为 CSV。"}, {"tip.seed.table", "种子池表格，删除前需要先选中一行。"},
      {"wechat.database", "SQLite 数据库"}, {"wechat.plugin_dir", "插件目录"}, {"wechat.port", "本地桥端口"}, {"wechat.browse", "浏览"}, {"wechat.adb", "启用 ADB 自动化"}, {"wechat.samples", "启动时加载示例数据"}, {"wechat.save", "保存运行配置"}, {"wechat.test", "发送本地桥冒烟载荷"}, {"wechat.log", "运行日志"},
      {"tip.wechat.database", "保存文章和种子的本地 SQLite 文件。"}, {"tip.wechat.plugin_dir", "内容提供者插件所在目录。"}, {"tip.wechat.port", "接收精简 JSON 的 localhost TCP 端口。"}, {"tip.wechat.adb", "只有在使用专用测试手机并明确授权时才启用。"}, {"tip.wechat.samples", "软件启动时加载演示记录。"}, {"tip.wechat.save", "把配置持久化到本地用户配置文件。"}, {"tip.wechat.test", "用安全示例载荷验证本地桥。"}, {"tip.wechat.log", "脱敏运行消息和接入事件。"},
      {"dash.accounts", "已扫描账号"}, {"dash.detections", "高价值发现"}, {"dash.top_score", "最高评分"},
      {"manual.title", "内置使用说明书"},
      {"manual.body", "1. 先进入微信接入页，确认数据库路径、插件目录和本地桥端口。除非使用专用测试手机，否则保持 ADB 关闭。\n\n2. 点击保存运行配置，然后加载插件。微信 Provider 会启动 localhost 本地桥。\n\n3. 点击发送本地桥冒烟载荷。如果文章表新增记录，说明本地接入链路可用。\n\n4. 如果要自动采集，进入自动采集页。每行粘贴一个微信文章 URL，打开 ADB 安全开关，然后点击立即执行下一条或开始自动采集。调度器只负责在已连接测试手机上打开文章，指标仍通过本地代理桥接入。\n\n5. 选择接入方案。方案 A 使用合法的本地代理适配器；方案 B 导入人工准备的 JSON 或 CSV；方案 C 只用 ADB 在测试手机上打开页面，指标仍通过 A 或 B 导入。\n\n6. 使用筛选页调整行业、关键词、评分权重和最低阅读数。评分综合互动、评论密度和发文频率。\n\n7. 选中文章行后，预览文章会打开链接并显示主要指标；收藏为种子会把该公众号保存到种子池。\n\n8. 在种子池页维护目标公众号。按公众号 ID、名称和分类添加或更新，需要备份时导出种子。\n\n9. 通过文件菜单把文章导出为 CSV 或 JSON。做生产决策前先复核导出结果。\n\n10. 安全边界：不要把 Cookie、Header、Token、证书或原始抓包保存到仓库。软件只应接收发送到 127.0.0.1 的精简脱敏 JSON。"}
  };
  const auto& dict = language == UiLanguage::Chinese ? zh : en;
  return dict.value(key, key);
}

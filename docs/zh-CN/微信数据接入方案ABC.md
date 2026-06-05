# 微信数据接入方案 A/B/C 详细实施文档

## 适用范围

本文档说明全网黄金内容雷达接入真实微信公众号文章指标的三种落地方式：

- 方案 A：本地代理适配器。
- 方案 B：浏览器或桌面端人工辅助适配器。
- 方案 C：ADB 手机自动化适配器。

三种方案都遵守同一个边界：软件只接收用户自己有权查看、分析和保存的数据；软件不保存账号密码，不提交 Cookie、Header、Token、证书、原始抓包或私有账号信息到仓库；所有数据先在本地转换成精简 JSON，再发送到 `127.0.0.1` 本地桥。

## 共同准备

### 1. 安装并启动软件

下载对应平台包，解压后启动主程序：

```bash
./premium-content-radar
```

在 WeChat 配置页确认：

- 本地桥端口：默认 `9000`。
- ADB 自动化：默认关闭。
- 数据库路径：确认写入你希望保存数据的位置。
- 插件目录：确认 WeChat Provider 插件所在目录。

### 2. 验证本地桥可用

图形界面里点击“发送本地桥冒烟载荷”。如果使用命令行，可运行：

```bash
QT_QPA_PLATFORM=offscreen ./premium-content-radar --bridge-smoke
```

通过后再接真实来源。如果冒烟失败，先修本地桥、端口、防火墙或插件加载问题。

### 3. 统一 JSON 输入协议

无论 A、B、C，最终都转换成以下两类 JSON。

文章指标：

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "文章标题",
  "url": "https://mp.weixin.qq.com/s/example",
  "account_name": "公众号名称",
  "gzh_id": "公众号ID",
  "category": "科技",
  "article_count_30d": 12,
  "appmsgstat": {
    "read_num": 24000,
    "like_num": 1200,
    "old_like_num": 300
  }
}
```

评论指标：

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "文章标题",
  "url": "https://mp.weixin.qq.com/s/example",
  "comment_count": 88
}
```

### 4. 统一发送脚本

保存为 `send_payload.py`，不要把真实 Cookie 或原始响应写进脚本：

```python
import json
import socket
import sys

port = int(sys.argv[1]) if len(sys.argv) > 1 else 9000
payload = json.load(sys.stdin)
with socket.create_connection(("127.0.0.1", port), timeout=3) as sock:
    sock.sendall(json.dumps(payload, ensure_ascii=False).encode("utf-8"))
```

发送：

```bash
python3 send_payload.py 9000 < sample_metrics.json
python3 send_payload.py 9000 < sample_comments.json
```

---

## 方案 A：本地代理适配器

### A.1 方案目标

方案 A 用本地代理或导出工具观察用户授权环境中的微信文章接口响应，提取阅读数、点赞数、在看数、评论数等指标，转换成软件 JSON 协议后发送到本地桥。

它是三种方案里自动化能力最强、接入真实指标最快的一种，但对合规边界和数据脱敏要求最高。

### A.2 推荐架构

```text
手机微信或桌面微信/浏览器
        |
        | 用户主动配置代理
        v
本地代理工具，只过滤目标接口
        |
        | 原始响应只在安全目录短暂停留
        v
本地适配器脚本，字段映射与脱敏
        |
        | 精简 JSON
        v
127.0.0.1:9000 本地桥
        |
        v
SQLite 入库、评分、筛选、导出
```

### A.3 操作步骤

#### 第一步：选择代理或导出工具

可用任何你信任且合法使用的本地代理或导出工具。工具需要能看到你自己打开的文章接口响应体，重点只过滤：

```text
/mp/getappmsgext
/mp/appmsg_comment
```

不要把代理工具的证书、Cookie、Header、Token、原始抓包文件放入项目目录。

#### 第二步：配置客户端代理

根据实际环境选择一种：

- 手机微信：手机 Wi-Fi 代理指向本机局域网 IP 和代理端口。
- 桌面微信或浏览器：系统代理或浏览器代理指向本机代理端口。
- 本机浏览器：也可以只给浏览器设置代理，避免影响系统全局流量。

如果代理工具需要 HTTPS 解析证书，只能安装到你自己控制、授权测试的设备中。证书不要提交到仓库。

#### 第三步：打开文章并捕获接口

手动打开目标公众号文章，观察代理工具中是否出现：

```text
/mp/getappmsgext
/mp/appmsg_comment
```

确认响应体是 JSON 或可解析文本。只需要保存短期临时样本用于调试，正式运行时建议流式处理，不长期保存原始响应。

#### 第四步：字段映射

从 `/mp/getappmsgext` 提取：

| 来源字段 | 软件字段 | 说明 |
| --- | --- | --- |
| `appmsgstat.read_num` | `read_num` | 阅读数 |
| `appmsgstat.like_num` | `like_num` | 点赞数 |
| `appmsgstat.old_like_num` | `old_like_num` | 在看或历史点赞指标，按来源含义解释 |
| 当前文章标题 | `title` | 可由页面标题、导出元数据或适配器上下文提供 |
| 当前文章链接 | `url` | 用作去重主键之一 |
| 公众号名称 | `account_name` | 可选但建议提供 |
| 公众号 ID | `gzh_id` | 可选但建议提供 |
| 近 30 天发文数 | `article_count_30d` | 没有则填 0 或由种子池补齐 |

从 `/mp/appmsg_comment` 提取：

| 来源字段 | 软件字段 | 说明 |
| --- | --- | --- |
| 评论列表总数或计数字段 | `comment_count` | 评论数 |
| 当前文章标题 | `title` | 用于和文章指标合并 |
| 当前文章链接 | `url` | 用于和文章指标合并 |

#### 第五步：适配器脚本最小流程

适配器应该执行：

1. 读取代理导出的单条响应。
2. 判断 URL 是否匹配目标接口。
3. 解析 JSON 响应体。
4. 从安全上下文补齐标题和链接。
5. 生成软件协议 JSON。
6. 发到 `127.0.0.1:9000`。
7. 写脱敏日志，只记录条数和错误，不记录 Cookie。

伪代码：

```python
for record in proxy_export_stream:
    if "/mp/getappmsgext" in record.url:
        payload = map_metrics(record.body, record.context)
        send_to_bridge(payload)
    elif "/mp/appmsg_comment" in record.url:
        payload = map_comments(record.body, record.context)
        send_to_bridge(payload)
```

#### 第六步：验收标准

- 本地桥冒烟通过。
- 代理能看到目标接口。
- 适配器日志没有 Cookie、Header、Token。
- 软件文章表新增真实文章。
- 阅读数、点赞数、评论数和代理工具看到的值一致。
- 导出 CSV/JSON 后字段正确。
- 关闭代理后软件仍可查看历史数据。

### A.4 风险与控制

- 风险：原始抓包泄露账号信息。控制：只保存在安全目录，调试后删除。
- 风险：代理影响其他流量。控制：尽量只给浏览器或测试设备设置代理。
- 风险：字段变化导致解析失败。控制：适配器做兼容解析和错误日志。
- 风险：重复入库。控制：用 URL、标题、发布时间或公众号 ID 去重。

---

## 方案 B：浏览器或桌面端人工辅助适配器

### B.1 方案目标

方案 B 不依赖抓包，也不控制手机。用户主动打开文章，再通过浏览器扩展、页面脚本、剪贴板、CSV 或手动表单把指标导入软件。

它的合规边界最稳，适合低频高质量采集、人工精选样本、团队运营分析和没有代理条件的环境。缺点是自动化弱，采集效率低。

### B.2 推荐架构

```text
用户手动打开文章
        |
        | 复制页面信息、导出 CSV、或使用本地浏览器辅助脚本
        v
人工辅助适配器
        |
        | 精简 JSON 或 CSV 转 JSON
        v
127.0.0.1:9000 本地桥
        |
        v
SQLite 入库、评分、筛选、导出
```

### B.3 实施方式一：JSON 文件导入

#### 第一步：创建样本文件

保存 `manual_metrics.json`：

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "人工录入的文章标题",
  "url": "https://mp.weixin.qq.com/s/example",
  "account_name": "公众号名称",
  "gzh_id": "公众号ID",
  "category": "运营",
  "article_count_30d": 8,
  "appmsgstat": {
    "read_num": 10000,
    "like_num": 500,
    "old_like_num": 100
  }
}
```

保存 `manual_comments.json`：

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "人工录入的文章标题",
  "url": "https://mp.weixin.qq.com/s/example",
  "comment_count": 35
}
```

#### 第二步：发送到本地桥

```bash
python3 send_payload.py 9000 < manual_metrics.json
python3 send_payload.py 9000 < manual_comments.json
```

#### 第三步：在软件中核验

打开文章表，检查标题、链接、账号、阅读数、点赞数、评论数是否正确。再导出 CSV，确认行数和字段。

### B.4 实施方式二：CSV 批量导入

#### 第一步：准备 CSV

保存 `manual_articles.csv`：

```csv
title,url,account_name,gzh_id,category,article_count_30d,read_num,like_num,old_like_num,comment_count
文章一,https://mp.weixin.qq.com/s/a,公众号A,gh_a,科技,10,24000,1200,300,88
文章二,https://mp.weixin.qq.com/s/b,公众号B,gh_b,财经,6,12000,300,90,25
```

#### 第二步：CSV 转本地桥 JSON

保存 `csv_to_bridge.py`：

```python
import csv
import json
import socket
import sys

port = int(sys.argv[1]) if len(sys.argv) > 1 else 9000
csv_path = sys.argv[2]

def send(payload):
    with socket.create_connection(("127.0.0.1", port), timeout=3) as sock:
        sock.sendall(json.dumps(payload, ensure_ascii=False).encode("utf-8"))

with open(csv_path, newline="", encoding="utf-8") as f:
    for row in csv.DictReader(f):
        metrics = {
            "endpoint": "/mp/getappmsgext",
            "title": row["title"],
            "url": row["url"],
            "account_name": row.get("account_name", ""),
            "gzh_id": row.get("gzh_id", ""),
            "category": row.get("category", ""),
            "article_count_30d": int(row.get("article_count_30d") or 0),
            "appmsgstat": {
                "read_num": int(row.get("read_num") or 0),
                "like_num": int(row.get("like_num") or 0),
                "old_like_num": int(row.get("old_like_num") or 0)
            }
        }
        comments = {
            "path": "/mp/appmsg_comment?action=getcomment",
            "title": row["title"],
            "url": row["url"],
            "comment_count": int(row.get("comment_count") or 0)
        }
        send(metrics)
        send(comments)
```

运行：

```bash
python3 csv_to_bridge.py 9000 manual_articles.csv
```

### B.5 实施方式三：浏览器辅助脚本

这个方式适合用户主动打开文章页面后，从页面标题、地址栏和人工填写的指标生成 JSON。

操作方法：

1. 在浏览器打开文章。
2. 复制文章标题和 URL。
3. 在本地小表单或命令行里填写阅读数、点赞数、评论数。
4. 生成 JSON 并发送到本地桥。

这种方式不读取 Cookie，不抓包，不自动访问接口。它只把用户已经看到并确认的数据录入系统。

### B.6 验收标准

- JSON 单条导入成功。
- CSV 批量导入成功。
- 重复导入不会产生不可控重复数据。
- 表格、评分、导出都能反映人工输入的数据。
- 用户不需要提供账号密码或代理证书。

### B.7 风险与控制

- 风险：人工录入错误。控制：导入前做字段校验，导入后导出复核。
- 风险：批量 CSV 格式不一致。控制：固定列名，缺失字段填 0 或空字符串。
- 风险：效率低。控制：用于精选样本或作为方案 A/C 的兜底。

---

## 方案 C：ADB 手机自动化适配器

### C.1 方案目标

方案 C 通过用户授权的测试手机和 ADB，在本地控制滚动、点击、打开文章，再配合方案 A 的本地代理或方案 B 的人工确认，把文章指标导入软件。

它适合需要较高采集效率、且有专门测试设备和测试账号的场景。它的环境依赖最高，必须单独确认手机、USB 调试、测试账号、代理策略和操作范围。

### C.2 推荐架构

```text
测试手机，已登录用户授权账号
        |
        | USB 调试，仅连接本机
        v
ADB 自动化脚本，点击、滚动、打开文章
        |
        | 可选：配合本地代理捕获指标
        v
本地适配器字段映射
        |
        | 精简 JSON
        v
127.0.0.1:9000 本地桥
        |
        v
SQLite 入库、评分、筛选、导出
```

### C.3 前置条件

- 一台专用测试手机。
- 手机上已登录允许用于测试的微信环境。
- 手机允许 USB 调试。
- 电脑安装 `adb`。
- 用户明确允许自动点击、滚动、打开文章。
- 如果需要真实指标，仍需配合方案 A 的代理或方案 B 的人工录入。
- ADB 自动化默认关闭，只有用户明确开启后才运行。

### C.4 环境准备

#### 第一步：安装 ADB

Linux 示例：

```bash
sudo apt-get update
sudo apt-get install -y android-tools-adb
```

Windows 可安装 Android Platform Tools，并把 `adb.exe` 加入 PATH。

#### 第二步：连接手机

```bash
adb devices
```

手机弹出授权时选择允许。输出应类似：

```text
List of devices attached
DEVICE_SERIAL    device
```

如果显示 `unauthorized`，需要在手机上重新授权。如果没有设备，检查 USB 线、驱动和开发者选项。

#### 第三步：限制 ADB 使用范围

建议只使用专用测试手机，不在日常主力手机上跑自动化。脚本只执行以下动作：

- 打开微信或指定页面。
- 点击用户确认范围内的文章入口。
- 滚动列表。
- 读取当前页面截图或辅助信息。
- 不读取通讯录，不发送消息，不修改账号设置。

### C.5 软件配置

在 WeChat 配置页：

1. 勾选启用 ADB 自动化。
2. 保存运行配置。
3. 确认本地桥端口仍为 `9000`。
4. 如果使用代理，把测试手机 Wi-Fi 代理指向本机代理端口。
5. 加载 WeChat Provider 插件。

也可以通过环境变量显式启用：

```bash
export PREMIUM_RADAR_ENABLE_ADB=1
export PREMIUM_RADAR_BRIDGE_PORT=9000
./premium-content-radar
```

### C.6 自动化流程

最小流程：

1. `adb devices` 确认设备在线。
2. 打开微信。
3. 用户确认目标公众号或文章列表。
4. 脚本执行点击和滚动。
5. 每打开一篇文章，等待页面加载。
6. 如果采用代理路线，由代理适配器捕获 `/mp/getappmsgext` 和 `/mp/appmsg_comment`。
7. 如果采用人工辅助路线，用户或脚本记录标题、链接和可见指标。
8. 适配器发送 JSON 到本地桥。
9. 软件入库并评分。

### C.7 ADB 命令示例

确认设备：

```bash
adb devices
```

查看当前窗口：

```bash
adb shell dumpsys window | grep -E "mCurrentFocus|mFocusedApp"
```

截图用于人工确认：

```bash
adb exec-out screencap -p > screen.png
```

点击示例：

```bash
adb shell input tap 500 1200
```

滚动示例：

```bash
adb shell input swipe 500 1600 500 600 500
```

这些命令只是示例。正式脚本必须根据测试手机分辨率、微信版本和用户确认的页面布局调整，不应盲目批量点击。

### C.8 验收标准

- `adb devices` 稳定显示设备在线。
- 软件里 ADB 开关可控，默认关闭。
- 自动化只在测试手机和用户确认范围内运行。
- 打开文章后，方案 A 或 B 能把指标送入本地桥。
- 软件文章表新增数据，导出可用。
- 停止软件或关闭 ADB 开关后，自动化停止。

### C.9 风险与控制

- 风险：误点击或影响账号。控制：只用测试手机和测试账号，脚本动作白名单化。
- 风险：微信 UI 变化导致坐标失效。控制：每次运行前截图确认，坐标配置化。
- 风险：自动化过度频繁。控制：限速、人工确认、批量上限。
- 风险：真实指标仍取不到。控制：C 只负责打开页面，指标获取仍由 A 或 B 完成。

---

## 三种方案选择建议

| 方案 | 适合场景 | 优点 | 缺点 | 推荐优先级 |
| --- | --- | --- | --- | --- |
| A 本地代理适配器 | 需要较快接入真实指标 | 自动化强，数据完整 | 需要代理和脱敏管理 | 第一优先 |
| B 人工辅助适配器 | 精选样本、合规稳妥、无代理环境 | 风险低，可控 | 效率低 | 第二优先，兜底 |
| C ADB 手机自动化 | 有测试手机，需要批量打开页面 | 可提高操作效率 | 环境依赖和误操作风险高 | 第三优先，确认后再做 |

## 推荐落地顺序

1. 先完成方案 B 的 JSON/CSV 导入验证，证明软件数据协议和评分链路稳定。
2. 再接方案 A，把真实接口指标转换成同一套 JSON 协议。
3. 最后评估方案 C，只让 ADB 负责打开页面和辅助操作，不让它保存账号秘密或原始私有数据。

## 生产上线总检查表

- 本地桥冒烟测试通过。
- 数据来源合法且由用户主动授权。
- 适配器不提交 Cookie、Header、Token、证书和原始抓包。
- JSON 字段符合统一协议。
- SQLite 数据库路径明确。
- 导出 CSV/JSON 可复核。
- 日志只包含脱敏统计。
- 停止软件后本地桥关闭。
- 如果启用 ADB，必须有测试设备、授权确认、限速和停止机制。

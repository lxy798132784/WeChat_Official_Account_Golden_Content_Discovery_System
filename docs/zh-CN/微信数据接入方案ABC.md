# 微信数据接入方案 A/B/C 详细实施文档

## 先说清楚：这份文档要解决什么问题

全网黄金内容雷达本身不登录微信、不保存微信账号密码、不保存 Cookie，也不直接破解任何平台接口。它只做一件事：

1. 你在自己有权访问的环境里打开微信公众号文章。
2. 你用本地工具拿到文章的公开展示信息或你自己能看到的指标。
3. 本地适配器把这些信息整理成软件能识别的精简 JSON。
4. JSON 发到本机 `127.0.0.1:9000`。
5. 软件入库、评分、筛选、导出。

如果你不懂“本地代理”“导出工具”“适配器”这些词，可以先记住这个最简单的类比：

- 本地代理工具：像一个本机流量观察窗口，帮你看到自己设备打开文章时产生的网络响应。
- 导出工具：像一个本地保存按钮，把你自己打开的文章内容或指标导出成文件。
- 适配器脚本：像一个翻译器，把代理或导出的数据翻译成全网黄金内容雷达能吃的 JSON。

本文把 A/B/C 三个方案都写成可以照着做的步骤，不要求用户再自己去搜索“应该用什么工具”。

---

## 共同准备：三个方案都要先做

### 1. 启动软件

下载对应平台的压缩包，解压后启动主程序。

Linux 示例：

```bash
./premium-content-radar
```

Windows 示例：

```text
双击 premium-content-radar.exe
```

Windows 版是 GUI 程序，正常启动时不应该弹出黑色命令行窗口。

### 2. 在软件里检查 WeChat 配置页

打开左侧：

```text
微信接入 / WeChat
```

确认这些配置：

| 配置项 | 推荐值 | 说明 |
| --- | --- | --- |
| SQLite 数据库 | 默认即可，或改到你的数据目录 | 保存文章、种子、评分结果 |
| 插件目录 | 解压包里的插件目录 | 存放 WeChat Provider 插件 |
| 本地桥端口 | `9000` | 适配器把 JSON 发到这里 |
| 启用 ADB 自动化 | 默认关闭 | 方案 C 才需要开启 |
| 启动时加载示例数据 | 可关可开 | 生产使用建议关闭 |

点一次：

```text
保存运行配置
```

### 3. 验证本地桥能接收数据

在软件 WeChat 配置页点击：

```text
发送本地桥冒烟载荷
```

成功表现：

- 文章表出现一条测试文章。
- 运行日志显示本地桥载荷已发送或已接入记录。

如果这一步失败，先不要接真实数据，先排查：

| 问题 | 处理方法 |
| --- | --- |
| 端口被占用 | 把本地桥端口改成 `9001`，保存后重启软件 |
| 插件没加载 | 检查插件目录，点击“加载插件” |
| 防火墙拦截 | 允许本机程序监听 localhost |
| 适配器连不上 | 确认适配器发的是 `127.0.0.1`，不是局域网 IP |

### 4. 统一 JSON 协议

不管你用 A、B、C 哪个方案，最后都要变成下面这两类 JSON。

#### 文章指标 JSON

保存为 `sample_metrics.json`：

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "文章标题",
  "url": "https://mp.weixin.qq.com/s/example",
  "account_name": "公众号名称",
  "gzh_id": "gh_xxxxxxxx",
  "category": "科技",
  "publish_time": "2026-06-05T08:30:00Z",
  "article_count_30d": 12,
  "appmsgstat": {
    "read_num": 24000,
    "like_num": 1200,
    "old_like_num": 300
  }
}
```

字段解释：

| 字段 | 必填 | 含义 | 没有时怎么办 |
| --- | --- | --- | --- |
| `endpoint` | 是 | 固定写 `/mp/getappmsgext` | 不要改 |
| `title` | 建议填 | 文章标题 | 手动复制或从页面标题取 |
| `url` | 是 | 文章链接，软件用它去重 | 必须填 |
| `account_name` | 建议填 | 公众号名称 | 不知道就先填空字符串 |
| `gzh_id` | 建议填 | 公众号原始 ID | 不知道就先填空字符串 |
| `category` | 建议填 | 分类，例如科技、财经、运营 | 不知道可填“未分类” |
| `publish_time` | 建议填 | 发布时间 | 支持 ISO、日期、秒/毫秒时间戳 |
| `article_count_30d` | 可选 | 近 30 天发文数 | 不知道填 `0` |
| `read_num` | 建议填 | 阅读数 | 看不到就填 `0` |
| `like_num` | 建议填 | 点赞数 | 看不到就填 `0` |
| `old_like_num` | 可选 | 在看/历史点赞类指标 | 看不到填 `0` |

#### 评论指标 JSON

保存为 `sample_comments.json`：

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "文章标题",
  "url": "https://mp.weixin.qq.com/s/example",
  "comment_count": 88
}
```

字段解释：

| 字段 | 必填 | 含义 |
| --- | --- | --- |
| `path` | 是 | 固定包含 `/mp/appmsg_comment` |
| `title` | 建议填 | 用于人工核对 |
| `url` | 是 | 用于和文章指标合并、去重 |
| `comment_count` | 建议填 | 评论数 |

### 5. 统一发送脚本

这个脚本负责把一个 JSON 文件发给软件。

保存为：

```text
send_payload.py
```

内容：

```python
import json
import socket
import sys

port = int(sys.argv[1]) if len(sys.argv) > 1 else 9000
payload = json.load(sys.stdin)

with socket.create_connection(("127.0.0.1", port), timeout=3) as sock:
    sock.sendall(json.dumps(payload, ensure_ascii=False).encode("utf-8"))

print("sent")
```

发送测试：

```bash
python3 send_payload.py 9000 < sample_metrics.json
python3 send_payload.py 9000 < sample_comments.json
```

Windows 如果没有 `python3`，通常用：

```powershell
python send_payload.py 9000 < sample_metrics.json
python send_payload.py 9000 < sample_comments.json
```

---

# 方案 A：本地代理适配器

## A.0 适合谁

方案 A 适合你想尽量自动化地接入真实文章指标。它的优点是：

- 可以接真实阅读数、点赞数、评论数。
- 打开文章后自动捕获接口响应。
- 适合后续做批量采集。

缺点是：

- 需要配置代理工具。
- 需要理解 HTTPS 证书安装。
- 必须非常注意 Cookie、Header、证书、原始抓包不要泄露。

如果你完全不想碰代理，先用方案 B。

## A.1 推荐工具清单

下面这些工具都能做本地 HTTP/HTTPS 代理或抓包观察。按易用程度推荐：

| 工具 | 支持平台 | 推荐程度 | 适合用户 | 作用 |
| --- | --- | --- | --- | --- |
| Reqable | Windows、macOS、Linux、Android、iOS | 首选 | 不想写代码、想图形界面 | 图形化抓 HTTP/HTTPS，过滤接口，复制响应 |
| mitmproxy / mitmweb | Windows、macOS、Linux | 首选 | 能接受命令行或网页界面 | 本地代理，脚本化能力强，最适合做自动适配器 |
| Fiddler Everywhere | Windows、macOS、Linux | 推荐 | Windows 用户、喜欢图形界面 | 抓包、过滤、查看响应 |
| Charles Proxy | Windows、macOS、Linux | 推荐 | 已经有 Charles 或习惯商业工具 | 抓包、SSL Proxying、保存响应 |
| Proxyman | macOS | 推荐 | macOS 用户 | 图形化代理，操作简单 |
| whistle | Windows、macOS、Linux | 进阶 | 会 Node.js 的用户 | 规则代理，适合长期自动化 |
| HTTP Toolkit | Windows、macOS、Linux | 可选 | 想图形界面调试 HTTP | 拦截 HTTP/HTTPS，适合开发调试 |

不建议新手直接用 Wireshark。Wireshark 很强，但它是底层网络包分析工具，HTTPS 内容解析不直观，新手很难直接拿到 JSON 响应体。

## A.2 最推荐路线：Reqable 图形界面路线

### A.2.1 适合场景

如果你是普通运营或产品用户，不想写代理脚本，建议先用 Reqable。它的优点是：

- 有图形界面。
- 可以直接看到请求列表。
- 可以搜索 `/mp/getappmsgext` 和 `/mp/appmsg_comment`。
- 可以复制响应体。

### A.2.2 安装

1. 打开 Reqable 官网下载安装包。
2. 安装桌面端。
3. 启动 Reqable。
4. 在 Reqable 里开启代理监听。

通常你会看到一个代理地址，例如：

```text
电脑 IP：192.168.1.20
代理端口：9000、8888、8080 或 Reqable 显示的端口
```

注意：这个代理端口不是全网黄金内容雷达的 `9000`。为了避免冲突，建议：

| 用途 | 推荐端口 |
| --- | --- |
| 全网黄金内容雷达本地桥 | `9000` |
| Reqable 代理 | `8888` |

如果 Reqable 默认也是 `9000`，请把 Reqable 改成 `8888`，或者把软件本地桥改成 `9001`。

### A.2.3 配置手机微信走 Reqable 代理

手机和电脑要在同一个 Wi-Fi 下。

在手机上：

1. 打开 Wi-Fi 设置。
2. 点当前 Wi-Fi。
3. 找到“代理”。
4. 选择“手动”。
5. 服务器填电脑局域网 IP，例如：

```text
192.168.1.20
```

6. 端口填 Reqable 代理端口，例如：

```text
8888
```

7. 保存。

然后在 Reqable 里按提示给手机安装 HTTPS 证书。只在你自己控制的测试手机上安装证书，不要把证书上传到仓库或发给别人。

### A.2.4 打开文章并过滤接口

1. 手机微信打开一篇公众号文章。
2. 回到 Reqable。
3. 在搜索框输入：

```text
getappmsgext
```

如果看到了包含下面路径的请求，就说明文章指标接口捕获到了：

```text
/mp/getappmsgext
```

再搜索：

```text
appmsg_comment
```

如果看到了包含下面路径的请求，就说明评论接口捕获到了：

```text
/mp/appmsg_comment
```

### A.2.5 从 Reqable 复制字段

点开 `/mp/getappmsgext` 请求，看响应体。通常你要找：

```json
{
  "appmsgstat": {
    "read_num": 24000,
    "like_num": 1200,
    "old_like_num": 300
  }
}
```

把它整理成软件协议：

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "你打开的文章标题",
  "url": "你打开的文章链接",
  "account_name": "公众号名称",
  "gzh_id": "",
  "category": "未分类",
  "publish_time": "2026-06-05T08:30:00Z",
  "article_count_30d": 0,
  "appmsgstat": {
    "read_num": 24000,
    "like_num": 1200,
    "old_like_num": 300
  }
}
```

评论接口 `/mp/appmsg_comment` 里，如果能看到总评论数，就整理成：

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "你打开的文章标题",
  "url": "你打开的文章链接",
  "comment_count": 88
}
```

### A.2.6 发送到软件

把上面两个 JSON 分别保存为：

```text
article_metrics.json
article_comments.json
```

发送：

```bash
python3 send_payload.py 9000 < article_metrics.json
python3 send_payload.py 9000 < article_comments.json
```

软件文章表出现新记录，就说明 Reqable 路线通了。

### A.2.7 Reqable 常见问题

| 现象 | 原因 | 解决方法 |
| --- | --- | --- |
| 手机上不了网 | 代理 IP 或端口错了 | 检查电脑 IP、Reqable 端口、防火墙 |
| 只能看到 CONNECT，看不到 JSON | HTTPS 证书没装好或没信任 | 按 Reqable 提示安装并信任证书 |
| 看不到 `/mp/getappmsgext` | 文章没完全加载，或搜索条件不对 | 重新打开文章，下拉刷新，再搜 `getappmsgext` |
| 软件没新增文章 | JSON 没发到本地桥 | 先点软件里的冒烟测试，再检查端口 |
| 文章重复 | URL 一样会覆盖更新 | 这是正常去重行为 |

## A.3 最适合自动化路线：mitmproxy / mitmweb

### A.3.1 适合场景

mitmproxy 适合后续做长期自动化，因为它可以挂 Python 脚本，自动把目标接口响应转成软件 JSON。

### A.3.2 安装

Linux：

```bash
python3 -m pip install --user mitmproxy
```

macOS 如果有 Homebrew：

```bash
brew install mitmproxy
```

Windows：

```powershell
python -m pip install mitmproxy
```

验证：

```bash
mitmproxy --version
mitmweb --version
```

### A.3.3 启动 mitmweb 图形界面

建议新手先用 `mitmweb`，它会打开一个网页界面：

```bash
mitmweb --listen-host 0.0.0.0 --listen-port 8888
```

含义：

| 参数 | 含义 |
| --- | --- |
| `--listen-host 0.0.0.0` | 允许手机连接电脑代理 |
| `--listen-port 8888` | 代理端口是 `8888` |

浏览器会打开 mitmweb 控制台，通常地址是：

```text
http://127.0.0.1:8081
```

### A.3.4 手机配置代理

和 Reqable 一样：

- 手机 Wi-Fi 代理服务器：电脑局域网 IP
- 端口：`8888`

然后手机浏览器打开：

```text
http://mitm.it
```

选择对应系统安装证书。

只在你自己的测试手机上安装证书。证书不要提交到仓库。

### A.3.5 在 mitmweb 里确认接口

1. 手机微信打开公众号文章。
2. mitmweb 过滤框输入：

```text
~u getappmsgext
```

3. 再试：

```text
~u appmsg_comment
```

如果能看到请求，点开 Response 查看 JSON。

### A.3.6 mitmproxy 自动适配脚本

保存为：

```text
mitm_wechat_to_radar.py
```

内容：

```python
import json
import socket
from urllib.parse import urlparse

BRIDGE_HOST = "127.0.0.1"
BRIDGE_PORT = 9000

# 这里可以按你的业务默认分类修改
DEFAULT_CATEGORY = "未分类"


def send_to_bridge(payload):
    data = json.dumps(payload, ensure_ascii=False).encode("utf-8")
    with socket.create_connection((BRIDGE_HOST, BRIDGE_PORT), timeout=3) as sock:
        sock.sendall(data)


def safe_json(text):
    try:
        return json.loads(text)
    except Exception:
        return {}


def find_comment_count(body):
    # 不同响应可能字段不同，这里做宽松兼容
    for key in ["comment_count", "elected_comment_total_cnt", "total_count", "count"]:
        value = body.get(key)
        if isinstance(value, int):
            return value
        if isinstance(value, str) and value.isdigit():
            return int(value)

    # 有些接口把评论列表放在 elected_comment 或 comment 数组里
    for key in ["elected_comment", "comment", "comments", "list"]:
        value = body.get(key)
        if isinstance(value, list):
            return len(value)
    return 0


def response(flow):
    url = flow.request.pretty_url
    if "/mp/getappmsgext" not in url and "/mp/appmsg_comment" not in url:
        return

    body = safe_json(flow.response.get_text(strict=False))
    if not body:
        return

    # 从请求头或 URL 里尽量补上下文；拿不到就填空，后面可以人工补齐
    referer = flow.request.headers.get("Referer", "")
    article_url = referer or url
    title = flow.request.headers.get("X-Radar-Title", "Captured Article")

    if "/mp/getappmsgext" in url:
        stat = body.get("appmsgstat", {})
        payload = {
            "endpoint": "/mp/getappmsgext",
            "title": title,
            "url": article_url,
            "account_name": "",
            "gzh_id": "",
            "category": DEFAULT_CATEGORY,
            "publish_time": "",
            "article_count_30d": 0,
            "appmsgstat": {
                "read_num": int(stat.get("read_num") or body.get("read_num") or 0),
                "like_num": int(stat.get("like_num") or body.get("like_num") or 0),
                "old_like_num": int(stat.get("old_like_num") or body.get("old_like_num") or 0),
            },
        }
        send_to_bridge(payload)
        print("sent metrics", payload["url"])

    if "/mp/appmsg_comment" in url:
        payload = {
            "path": "/mp/appmsg_comment?action=getcomment",
            "title": title,
            "url": article_url,
            "comment_count": find_comment_count(body),
        }
        send_to_bridge(payload)
        print("sent comments", payload["url"])
```

启动：

```bash
mitmproxy --listen-host 0.0.0.0 --listen-port 8888 -s mitm_wechat_to_radar.py
```

然后手机打开文章。命令行出现：

```text
sent metrics ...
sent comments ...
```

软件文章表出现数据，说明自动适配成功。

### A.3.7 mitmproxy 路线注意点

mitmproxy 自动脚本不一定能自动拿到文章标题和发布时间，因为标题和发布时间常在 HTML 页面里，而指标接口响应里可能只有指标。解决方法有三种：

1. 初期手动补标题和发布时间。
2. 让适配器另外捕获文章 HTML 页面，从页面里解析标题和发布时间。
3. 先用 URL 去重，后续在软件里或 CSV 里补全标题和发布时间。

建议第一阶段先保证阅读、点赞、评论数打通；第二阶段再做标题、发布时间自动补齐。

## A.4 Windows 用户路线：Fiddler Everywhere

### A.4.1 安装和启动

1. 安装 Fiddler Everywhere。
2. 启动后打开 Capturing。
3. 在 Settings 里开启 HTTPS traffic capture。
4. 按提示安装并信任证书。

### A.4.2 配置代理

如果抓本机浏览器：

- Fiddler 通常会自动配置系统代理。

如果抓手机微信：

- 手机 Wi-Fi 代理服务器填电脑 IP。
- 端口填 Fiddler 显示的代理端口，常见是 `8866` 或软件界面显示的端口。

### A.4.3 过滤目标接口

在 Fiddler 搜索或过滤 URL：

```text
getappmsgext
appmsg_comment
```

点开请求，看 Response Body。

### A.4.4 导出和发送

Fiddler 可以复制响应体。把响应体里的 `appmsgstat` 字段整理成前面的 `article_metrics.json`，再用：

```powershell
python send_payload.py 9000 < article_metrics.json
python send_payload.py 9000 < article_comments.json
```

发送到软件。

## A.5 Charles Proxy 路线

### A.5.1 安装和配置

1. 安装 Charles。
2. 打开 Proxy。
3. 启用 macOS/Windows Proxy 或设置手机 Wi-Fi 代理到电脑 IP。
4. 打开：

```text
Proxy -> SSL Proxying Settings
```

5. 添加 Host：

```text
mp.weixin.qq.com
```

6. Port：

```text
443
```

7. 在手机或电脑上安装 Charles 证书并信任。

### A.5.2 捕获接口

打开文章后，在 Charles 左侧域名里找：

```text
mp.weixin.qq.com
```

再找路径：

```text
/mp/getappmsgext
/mp/appmsg_comment
```

复制 JSON 响应，整理成软件协议后发送。

## A.6 whistle 路线

### A.6.1 适合场景

whistle 适合会 Node.js 的用户。它可以长期作为本地代理规则系统，适合后续把规则和适配器固定下来。

### A.6.2 安装

```bash
npm install -g whistle
w2 start -p 8888
```

打开管理界面：

```text
http://127.0.0.1:8899
```

手机或浏览器代理指向电脑 IP 的 `8888` 端口。

安装 HTTPS 根证书按 whistle 页面提示操作。

### A.6.3 过滤接口

在 whistle 的 Network 面板搜索：

```text
getappmsgext
appmsg_comment
```

复制响应体，整理成软件 JSON。

进阶做法是写 whistle 插件或规则，把响应转发给本地适配器；第一阶段不建议新手直接做插件，先人工复制验证链路。

## A.7 Proxyman 路线

Proxyman 主要适合 macOS 用户。

步骤：

1. 安装 Proxyman。
2. 启动代理捕获。
3. 对 `mp.weixin.qq.com` 开启 SSL Proxying。
4. 如果抓手机，手机 Wi-Fi 代理指向 Mac 的 IP 和 Proxyman 端口。
5. 安装并信任证书。
6. 打开文章。
7. 搜索：

```text
getappmsgext
appmsg_comment
```

8. 复制响应体，整理成软件 JSON，发送到本地桥。

## A.8 HTTP Toolkit 路线

HTTP Toolkit 适合开发者调试 HTTP。

步骤：

1. 安装 HTTP Toolkit。
2. 选择拦截浏览器或系统代理。
3. 如需手机，按软件提示配置手机代理和证书。
4. 打开文章。
5. 搜索目标接口。
6. 复制响应体并转换成 JSON。

## A.9 字段映射完整说明

### `/mp/getappmsgext` 常见字段

| 可能来源字段 | 软件字段 | 示例 |
| --- | --- | --- |
| `appmsgstat.read_num` | `appmsgstat.read_num` | `24000` |
| `appmsgstat.like_num` | `appmsgstat.like_num` | `1200` |
| `appmsgstat.old_like_num` | `appmsgstat.old_like_num` | `300` |
| `read_num` | `appmsgstat.read_num` | 有些导出工具会拍平成顶层 |
| `like_num` | `appmsgstat.like_num` | 有些导出工具会拍平成顶层 |
| `old_like_num` | `appmsgstat.old_like_num` | 有些导出工具会拍平成顶层 |

### `/mp/appmsg_comment` 常见字段

不同工具看到的评论接口结构可能不同，优先级如下：

| 可能来源字段 | 软件字段 | 说明 |
| --- | --- | --- |
| `comment_count` | `comment_count` | 最理想 |
| `elected_comment_total_cnt` | `comment_count` | 常见候选字段 |
| `total_count` | `comment_count` | 常见候选字段 |
| `count` | `comment_count` | 常见候选字段 |
| `elected_comment` 数组长度 | `comment_count` | 没有总数时可用列表长度兜底 |
| `comment` 数组长度 | `comment_count` | 没有总数时可用列表长度兜底 |

### 发布时间字段

软件支持以下输入字段：

```text
publish_time
publishTime
publish_date
publishDate
create_time
createTime
datetime
date
```

支持格式：

```text
2026-06-05T08:30:00Z
2026-06-05 08:30:00
2026-06-05
1717576200
1717576200000
```

如果代理响应里没有发布时间，建议从文章页面手动复制，先填到 `publish_time`。

## A.10 从代理响应到软件 JSON 的完整手工示例

假设代理看到 `/mp/getappmsgext` 响应：

```json
{
  "appmsgstat": {
    "read_num": 35678,
    "like_num": 1350,
    "old_like_num": 260
  }
}
```

你在文章页面看到：

```text
标题：AI 产品如何做增长
公众号：增长研究所
链接：https://mp.weixin.qq.com/s/abc123
发布时间：2026-06-05 08:30:00
评论数：96
```

整理出 `metrics.json`：

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "AI 产品如何做增长",
  "url": "https://mp.weixin.qq.com/s/abc123",
  "account_name": "增长研究所",
  "gzh_id": "",
  "category": "运营",
  "publish_time": "2026-06-05 08:30:00",
  "article_count_30d": 0,
  "appmsgstat": {
    "read_num": 35678,
    "like_num": 1350,
    "old_like_num": 260
  }
}
```

整理出 `comments.json`：

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "AI 产品如何做增长",
  "url": "https://mp.weixin.qq.com/s/abc123",
  "comment_count": 96
}
```

发送：

```bash
python3 send_payload.py 9000 < metrics.json
python3 send_payload.py 9000 < comments.json
```

到软件中核对：

| 软件列 | 应看到 |
| --- | --- |
| 标题 | AI 产品如何做增长 |
| 账号 | 增长研究所 |
| 发布时间 | 2026-06-05 08:30 |
| 阅读 | 35678 |
| 点赞 | 1350 |
| 在看 | 260 |
| 评论 | 96 |

## A.11 安全边界，必须照做

不要保存这些东西到项目目录：

```text
Cookie
Header
Token
证书
原始抓包 har 文件
原始响应全集
账号密码
二维码截图
包含个人信息的日志
```

建议目录：

```text
~/premium-radar-ingestion/raw-temp/      临时原始样本，用完删除
~/premium-radar-ingestion/sanitized/     脱敏后的 JSON
~/premium-radar-ingestion/scripts/       适配器脚本
```

临时样本用完删除：

```bash
rm -rf ~/premium-radar-ingestion/raw-temp/*
```

---

# 方案 B：人工辅助适配器

## B.0 适合谁

方案 B 适合不想配置代理、只想稳定导入一批精选文章的人。它不抓包，不装证书，不控制手机。

你只需要：

1. 手动打开文章。
2. 复制标题、链接、发布时间。
3. 手动填阅读、点赞、评论。
4. 用 CSV 或 JSON 发到软件。

## B.1 单篇 JSON 导入

### 第一步：创建 `manual_metrics.json`

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "人工录入的文章标题",
  "url": "https://mp.weixin.qq.com/s/example",
  "account_name": "公众号名称",
  "gzh_id": "",
  "category": "运营",
  "publish_time": "2026-06-05 08:30:00",
  "article_count_30d": 0,
  "appmsgstat": {
    "read_num": 10000,
    "like_num": 500,
    "old_like_num": 100
  }
}
```

### 第二步：创建 `manual_comments.json`

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "人工录入的文章标题",
  "url": "https://mp.weixin.qq.com/s/example",
  "comment_count": 35
}
```

### 第三步：发送

```bash
python3 send_payload.py 9000 < manual_metrics.json
python3 send_payload.py 9000 < manual_comments.json
```

## B.2 CSV 批量导入

### 第一步：准备 CSV

保存为：

```text
manual_articles.csv
```

内容：

```csv
title,url,account_name,gzh_id,category,publish_time,article_count_30d,read_num,like_num,old_like_num,comment_count
文章一,https://mp.weixin.qq.com/s/a,公众号A,gh_a,科技,2026-06-05 08:30:00,10,24000,1200,300,88
文章二,https://mp.weixin.qq.com/s/b,公众号B,gh_b,财经,2026-06-04 12:00:00,6,12000,300,90,25
```

### 第二步：保存转换脚本

保存为：

```text
csv_to_bridge.py
```

内容：

```python
import csv
import json
import socket
import sys

port = int(sys.argv[1]) if len(sys.argv) > 1 else 9000
csv_path = sys.argv[2]


def to_int(value):
    try:
        return int(value or 0)
    except Exception:
        return 0


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
            "category": row.get("category", "未分类"),
            "publish_time": row.get("publish_time", ""),
            "article_count_30d": to_int(row.get("article_count_30d")),
            "appmsgstat": {
                "read_num": to_int(row.get("read_num")),
                "like_num": to_int(row.get("like_num")),
                "old_like_num": to_int(row.get("old_like_num")),
            },
        }
        comments = {
            "path": "/mp/appmsg_comment?action=getcomment",
            "title": row["title"],
            "url": row["url"],
            "comment_count": to_int(row.get("comment_count")),
        }
        send(metrics)
        send(comments)
        print("sent", row["title"])
```

### 第三步：运行

```bash
python3 csv_to_bridge.py 9000 manual_articles.csv
```

Windows：

```powershell
python csv_to_bridge.py 9000 manual_articles.csv
```

## B.3 表格软件导入路线

如果你不会写 CSV，可以用 Excel、WPS 或飞书表格建表。列名必须和下面一致：

```text
title
url
account_name
gzh_id
category
publish_time
article_count_30d
read_num
like_num
old_like_num
comment_count
```

填完后导出为 UTF-8 CSV，再用 `csv_to_bridge.py` 导入。

## B.4 B 方案验收

| 检查项 | 成功标准 |
| --- | --- |
| 单篇 JSON | 软件出现一篇文章 |
| CSV 批量 | CSV 有几行，软件就导入几篇或覆盖更新几篇 |
| 发布时间 | 表格里显示发布时间 |
| 评论数 | 评论列有值 |
| 导出 | 软件导出的 CSV/JSON 包含 `publish_time` |

---

# 方案 C：ADB 手机自动化适配器

## C.0 适合谁

方案 C 不直接获取指标。它只负责帮你自动打开文章、滚动、点击，提高操作效率。真实指标仍然要通过：

- 方案 A 的代理捕获；或
- 方案 B 的人工/CSV 导入。

所以 C 不是 A 的替代品，而是 A 或 B 的辅助器。

## C.1 准备专用测试手机

建议只用测试手机，不要用主力手机。

手机要求：

| 要求 | 说明 |
| --- | --- |
| 已登录允许测试的微信 | 不要用高风险主账号做大批量测试 |
| 开启开发者选项 | 为了启用 USB 调试 |
| 开启 USB 调试 | 电脑才能用 adb 控制 |
| 和电脑连接稳定 | 数据线质量要好 |
| 如用方案 A | 手机 Wi-Fi 代理指向电脑代理端口 |

## C.2 安装 ADB

Linux：

```bash
sudo apt-get update
sudo apt-get install -y android-tools-adb
```

Windows：

1. 下载 Android Platform Tools。
2. 解压到例如：

```text
C:\platform-tools
```

3. 把这个目录加入 PATH。
4. 打开 PowerShell：

```powershell
adb devices
```

macOS：

```bash
brew install android-platform-tools
```

## C.3 连接手机

运行：

```bash
adb devices
```

正常输出：

```text
List of devices attached
DEVICE_SERIAL    device
```

异常处理：

| 输出 | 说明 | 处理 |
| --- | --- | --- |
| `unauthorized` | 手机没授权 | 看手机弹窗，点允许 |
| 空列表 | 电脑没识别手机 | 换线、装驱动、重开 USB 调试 |
| `offline` | 连接状态异常 | 拔插 USB，执行 `adb kill-server && adb start-server` |

## C.4 软件里开启 ADB

在软件 WeChat 配置页：

1. 勾选“启用 ADB 自动化”。
2. 点击“保存运行配置”。
3. 重新加载插件或重启软件。

如果你只是用 A 或 B，不需要开启。

## C.5 常用 ADB 操作

查看当前手机页面：

```bash
adb shell dumpsys window | grep -E "mCurrentFocus|mFocusedApp"
```

截图：

```bash
adb exec-out screencap -p > screen.png
```

点击：

```bash
adb shell input tap 500 1200
```

滚动：

```bash
adb shell input swipe 500 1600 500 600 500
```

返回：

```bash
adb shell input keyevent KEYCODE_BACK
```

打开微信，包名可能随版本变化，常见命令：

```bash
adb shell monkey -p com.tencent.mm 1
```

## C.6 C 配合 A 的实际流程

1. 电脑启动全网黄金内容雷达。
2. 软件本地桥端口保持 `9000`。
3. 电脑启动 Reqable、mitmproxy、Charles 或 Fiddler，代理端口例如 `8888`。
4. 测试手机 Wi-Fi 代理指向电脑 IP 和 `8888`。
5. 手机安装并信任代理证书。
6. ADB 打开微信。
7. ADB 点击目标公众号文章。
8. 代理工具捕获 `/mp/getappmsgext` 和 `/mp/appmsg_comment`。
9. 适配器发送 JSON 到 `127.0.0.1:9000`。
10. 软件入库。

## C.7 C 配合 B 的实际流程

1. ADB 打开文章。
2. ADB 截图保存给人工确认。
3. 用户从页面复制标题、链接、发布时间和可见指标。
4. 填入 CSV。
5. `csv_to_bridge.py` 导入软件。

## C.8 C 方案不要做的事

不要让脚本：

- 自动发送微信消息。
- 自动加好友。
- 自动修改账号设置。
- 读取通讯录。
- 大批量无延迟点击。
- 在主力手机上长期无人值守运行。

建议每批最多先测试 5 到 10 篇，确认稳定后再增加。

---

# 三种方案怎么选

| 需求 | 推荐方案 |
| --- | --- |
| 完全不想配置代理 | B |
| 想最快看到真实数据进软件 | A，用 Reqable 手工复制起步 |
| 想长期自动化 | A，用 mitmproxy 脚本化 |
| 想批量打开文章但不自动抓指标 | C + B |
| 想批量打开文章并自动抓指标 | C + A |
| 合规风险最低 | B |
| 效率最高 | A 或 C + A |

推荐落地顺序：

1. 先用 B 的 JSON 或 CSV 跑通软件入库。
2. 再用 A 的 Reqable 手工复制跑通真实指标。
3. 再用 A 的 mitmproxy 脚本自动转发。
4. 最后再考虑 C，让 ADB 只负责打开文章。

---

# 生产上线检查表

| 检查项 | 必须达标 |
| --- | --- |
| 本地桥冒烟测试 | 通过 |
| 软件文章表 | 能看到真实或人工导入文章 |
| 发布时间 | 有值并能导出 |
| 阅读/点赞/评论 | 与来源一致 |
| CSV/JSON 导出 | 字段完整 |
| 原始抓包 | 不进仓库、不长期保存 |
| Cookie/Header/Token | 不进日志、不进脚本、不进仓库 |
| 代理证书 | 只安装在授权测试设备 |
| ADB | 默认关闭，需要时才开启 |
| Windows 启动 | 不弹命令行窗口 |

---

# 最小可行建议

如果你今天就要开始，按这个最小路径：

1. 软件里点击“发送本地桥冒烟载荷”。
2. 用 B 方案手写一条 `manual_metrics.json` 和 `manual_comments.json`。
3. 确认软件表格出现文章和发布时间。
4. 安装 Reqable。
5. 手机 Wi-Fi 代理指向 Reqable。
6. 打开一篇文章，搜索 `getappmsgext`。
7. 复制阅读、点赞、在看，整理成 JSON。
8. 发到软件。
9. 导出 CSV 复核。
10. 再决定是否上 mitmproxy 自动化或 ADB。

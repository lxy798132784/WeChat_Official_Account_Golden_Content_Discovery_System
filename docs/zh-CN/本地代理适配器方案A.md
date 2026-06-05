# 本地代理适配器方案A操作方法

## 目标

方案A的目标，是把真实微信文章指标通过合法、本地、用户主动配置的代理或导出工具接入软件。代理或适配器负责看到用户授权范围内的微信文章接口响应，把有用字段转换成全网黄金内容雷达约定的 JSON，然后发送到软件的 localhost 桥。

软件本身不内置代理，不保存账号密码，不保存 Cookie，也不绕过平台控制。这个方案只适合你自己有权查看、分析和保存的数据流。

## 整体链路

```text
微信客户端或浏览器
        |
        | 用户主动配置的本地代理/导出工具
        v
本地适配器脚本
        |
        | 精简 JSON，通过 TCP 发送
        v
127.0.0.1:9000 全网黄金内容雷达本地桥
        |
        v
SQLite 入库 + 评分 + 筛选 + 导出
```

## 第一步：启动软件并确认桥端口

1. 打开全网黄金内容雷达。
2. 进入 WeChat 配置页。
3. 设置本地桥端口，默认是 `9000`。
4. 保持 ADB 自动化关闭。
5. 点击保存运行配置。
6. 加载插件。
7. 点击 Actions 菜单里的发送本地桥冒烟载荷。

命令行也可以验证：

```bash
QT_QPA_PLATFORM=offscreen ./build/premium-content-radar --bridge-smoke
```

如果这个测试不通过，先不要接真实数据，先修本地桥。

## 第二步：准备本地代理或导出来源

需要一个能合法看到以下微信接口响应体的本地工具：

- `/mp/getappmsgext`
- `/mp/appmsg_comment`

典型操作流程：

1. 代理只运行在你自己的机器上。
2. 手机或桌面微信/浏览器手动设置这个代理。
3. 如果工具需要 HTTPS 解析证书，必须确认这是你有权测试的设备和账号环境。
4. 手动打开公众号文章。
5. 在代理工具里确认能看到 JSON 响应体。
6. 不要把原始抓包、Cookie、Header、账号信息提交到 Git 仓库。

## 第三步：把代理数据转换成软件协议

文章指标载荷：

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

评论载荷：

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "文章标题",
  "url": "https://mp.weixin.qq.com/s/example",
  "comment_count": 88
}
```

软件只需要这些字段。Cookie、请求头、设备信息、账号信息都不要发送给软件。

## 第四步：最小发送脚本

保存为 `send_payload.py`：

```python
import json
import socket
import sys

port = int(sys.argv[1]) if len(sys.argv) > 1 else 9000
payload = json.load(sys.stdin)
with socket.create_connection(("127.0.0.1", port), timeout=3) as sock:
    sock.sendall(json.dumps(payload, ensure_ascii=False).encode("utf-8"))
```

发送测试：

```bash
python3 send_payload.py 9000 < sample_metrics.json
python3 send_payload.py 9000 < sample_comments.json
```

发送后看软件日志和文章表格。如果文章数量增加，说明链路打通。

## 第五步：正式适配器应该做什么

正式适配器需要：

- 在仓库外读取代理工具导出的响应。
- 只提取软件需要的字段。
- 发送到 `127.0.0.1`。
- 丢弃 Cookie、Header、Token 和无关响应。
- 日志只记录脱敏数量和错误。
- 失败时带退避重试。
- 适配器本身不要把原始私有数据写进项目目录。

## 第六步：上线检查

上线前确认：

- 本地桥冒烟测试通过。
- 代理能看到授权范围内的目标接口响应。
- 适配器输出的是精简脱敏 JSON。
- 软件文章数量会增长。
- 导出的 CSV/JSON 行数和字段正确。
- 原始抓包文件已删除或放入安全目录。

## B 和 C 后续再讨论

方案B是人工或浏览器辅助导入，合规边界更稳，但自动化弱。

方案C是 ADB 手机自动化，效率高但环境依赖和风险更大，需要单独确认手机、USB 调试、测试账号和代理策略。

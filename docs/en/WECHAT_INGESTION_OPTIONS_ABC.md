# WeChat Ingestion Options A/B/C Detailed Implementation Guide

## What this document solves

Premium Content Radar does not log in to WeChat, does not store WeChat passwords, does not store cookies, and does not attempt to bypass platform protections. The application accepts compact local JSON records and then stores, scores, filters, and exports them.

The practical workflow is:

1. You open a WeChat Official Account article in an environment you are allowed to use.
2. A local tool captures or exports article information that you can legally view.
3. A local adapter converts that information into compact JSON.
4. The JSON is sent to `127.0.0.1:9000`.
5. Premium Content Radar stores the article, calculates scores, displays it in the table, and exports CSV or JSON.

If terms such as local proxy, export tool, or adapter are unfamiliar, use this plain explanation:

- A local proxy tool is a traffic inspection window running on your own computer. It lets you inspect responses created when your own device opens an article.
- An export tool saves content or metrics from articles you opened yourself.
- An adapter script is a translator. It turns proxy or export data into the JSON format accepted by Premium Content Radar.

This guide gives concrete tool choices and step-by-step instructions so users do not have to search for missing setup details.

---

## Shared preparation for all options

### 1. Start the application

Download the package for your platform, extract it, and start the main executable.

Linux example:

```bash
./premium-content-radar
```

Windows example:

```text
Double-click premium-content-radar.exe
```

The Windows build is a GUI application. It should not open a black command prompt window during normal startup.

### 2. Check the WeChat settings page

Open the left-side page:

```text
WeChat
```

Verify these settings:

| Setting | Recommended value | Purpose |
| --- | --- | --- |
| SQLite database | Default, or your chosen data directory | Stores articles, seeds, and scores |
| Plugin directory | The plugin directory inside the extracted package | Contains the WeChat Provider plugin |
| Local bridge port | `9000` | Adapters send JSON to this port |
| Enable ADB automation | Disabled by default | Needed only for Option C |
| Load sample data on startup | Optional | Disable for production use |

Click:

```text
Save Runtime Settings
```

### 3. Verify the local bridge

In the WeChat settings page, click:

```text
Send Bridge Smoke Payload
```

Expected result:

- A test article appears in the article table.
- The runtime log shows that a local bridge payload was sent or a provider record was ingested.

If this step fails, do not connect real sources yet. Fix the local bridge first.

| Problem | Fix |
| --- | --- |
| Port conflict | Change the bridge port to `9001`, save settings, and restart the app |
| Plugin not loaded | Check the plugin directory and click the plugin load action |
| Firewall block | Allow the app to listen on localhost |
| Adapter cannot connect | Confirm the adapter sends to `127.0.0.1`, not to a LAN IP |

### 4. Unified JSON contract

All three options must eventually produce one or both of the following JSON payloads.

#### Article metrics JSON

Save as `sample_metrics.json`:

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "Article title",
  "url": "https://mp.weixin.qq.com/s/example",
  "account_name": "Publisher name",
  "gzh_id": "gh_xxxxxxxx",
  "category": "Technology",
  "publish_time": "2026-06-05T08:30:00Z",
  "article_count_30d": 12,
  "appmsgstat": {
    "read_num": 24000,
    "like_num": 1200,
    "old_like_num": 300
  }
}
```

Field meaning:

| Field | Required | Meaning | If unavailable |
| --- | --- | --- | --- |
| `endpoint` | Yes | Always `/mp/getappmsgext` | Do not change it |
| `title` | Recommended | Article title | Copy manually or parse from page title |
| `url` | Yes | Article URL and deduplication key | Must be provided |
| `account_name` | Recommended | Official Account display name | Use an empty string if unknown |
| `gzh_id` | Recommended | Official Account original ID | Use an empty string if unknown |
| `category` | Recommended | Business category | Use `Uncategorized` if unknown |
| `publish_time` | Recommended | Article publish time | Supports ISO, date strings, seconds, and milliseconds |
| `article_count_30d` | Optional | Article count in the last 30 days | Use `0` if unknown |
| `read_num` | Recommended | Read count | Use `0` if unavailable |
| `like_num` | Recommended | Like count | Use `0` if unavailable |
| `old_like_num` | Optional | Old-like or wow-style metric | Use `0` if unavailable |

#### Comment metrics JSON

Save as `sample_comments.json`:

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "Article title",
  "url": "https://mp.weixin.qq.com/s/example",
  "comment_count": 88
}
```

Field meaning:

| Field | Required | Meaning |
| --- | --- | --- |
| `path` | Yes | Must contain `/mp/appmsg_comment` |
| `title` | Recommended | Used for manual verification |
| `url` | Yes | Used to merge comments with article metrics |
| `comment_count` | Recommended | Comment count |

### 5. Shared sender script

This script sends one JSON file to the application.

Save as:

```text
send_payload.py
```

Content:

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

Send test payloads:

```bash
python3 send_payload.py 9000 < sample_metrics.json
python3 send_payload.py 9000 < sample_comments.json
```

On Windows, if `python3` is unavailable, use:

```powershell
python send_payload.py 9000 < sample_metrics.json
python send_payload.py 9000 < sample_comments.json
```

---

# Option A: local proxy adapter

## A.0 Who should use Option A

Option A is for users who want to ingest real article metrics with more automation.

Advantages:

- It can capture real read, like, old-like, and comment metrics.
- It works while you open articles in your own authorized environment.
- It is the best foundation for later batch ingestion.

Tradeoffs:

- You must configure a proxy tool.
- HTTPS certificate setup may be required.
- Cookies, headers, certificates, and raw captures must be protected and never committed.

If you do not want to configure a proxy, start with Option B.

## A.1 Recommended tools

The following tools can inspect local HTTP or HTTPS traffic. They are listed by practical usefulness.

| Tool | Platforms | Recommendation | Best for | Purpose |
| --- | --- | --- | --- | --- |
| Reqable | Windows, macOS, Linux, Android, iOS | First choice | Users who want a graphical interface | Inspect HTTP and HTTPS, filter endpoints, copy responses |
| mitmproxy / mitmweb | Windows, macOS, Linux | First choice for automation | Users comfortable with scripts | Local proxy with strong Python scripting support |
| Fiddler Everywhere | Windows, macOS, Linux | Recommended | Windows users and GUI users | Capture, filter, and inspect responses |
| Charles Proxy | Windows, macOS, Linux | Recommended | Users already familiar with Charles | SSL proxying, response inspection, response export |
| Proxyman | macOS | Recommended | macOS users | Simple graphical proxy workflow |
| whistle | Windows, macOS, Linux | Advanced | Users familiar with Node.js | Rule-based proxy and long-running automation |
| HTTP Toolkit | Windows, macOS, Linux | Optional | Developers debugging HTTP | HTTP and HTTPS interception for development |

Wireshark is not recommended for beginners. It is powerful, but it works at a lower network-packet level and is not convenient for extracting HTTPS JSON response bodies.

## A.2 Recommended beginner route: Reqable

### A.2.1 When to use Reqable

Use Reqable if you want a graphical tool and do not want to write a proxy script at the beginning.

It can:

- Show a request list.
- Search for `/mp/getappmsgext` and `/mp/appmsg_comment`.
- Display response bodies.
- Let you copy response JSON for manual conversion.

### A.2.2 Install and start

1. Download and install Reqable from its official website.
2. Start the desktop application.
3. Enable proxy listening in Reqable.
4. Note the displayed computer IP and proxy port.

You may see values similar to:

```text
Computer IP: 192.168.1.20
Proxy port: 8888
```

Important: the proxy port is not the same as the Premium Content Radar bridge port. To avoid conflicts, use:

| Purpose | Recommended port |
| --- | --- |
| Premium Content Radar local bridge | `9000` |
| Reqable proxy | `8888` |

If Reqable uses `9000`, change Reqable to `8888`, or change the application bridge to `9001`.

### A.2.3 Configure phone WeChat to use Reqable

The phone and the computer must be on the same Wi-Fi network.

On the phone:

1. Open Wi-Fi settings.
2. Tap the current Wi-Fi network.
3. Find the proxy setting.
4. Choose manual proxy.
5. Enter the computer LAN IP, for example:

```text
192.168.1.20
```

6. Enter the Reqable proxy port, for example:

```text
8888
```

7. Save.

Then follow Reqable instructions to install and trust its HTTPS certificate on the phone. Install the certificate only on a device you control and are authorized to test. Never upload the certificate to the repository or send it to others.

### A.2.4 Open an article and filter endpoints

1. Open a WeChat Official Account article on the phone.
2. Return to Reqable.
3. Search for:

```text
getappmsgext
```

If you see a request containing the following path, article metrics were captured:

```text
/mp/getappmsgext
```

Then search for:

```text
appmsg_comment
```

If you see a request containing the following path, comment data was captured:

```text
/mp/appmsg_comment
```

### A.2.5 Copy fields from Reqable

Open the `/mp/getappmsgext` request and inspect the response body. Look for fields similar to:

```json
{
  "appmsgstat": {
    "read_num": 24000,
    "like_num": 1200,
    "old_like_num": 300
  }
}
```

Convert it into the application contract:

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "The article title you opened",
  "url": "The article URL you opened",
  "account_name": "Publisher name",
  "gzh_id": "",
  "category": "Uncategorized",
  "publish_time": "2026-06-05T08:30:00Z",
  "article_count_30d": 0,
  "appmsgstat": {
    "read_num": 24000,
    "like_num": 1200,
    "old_like_num": 300
  }
}
```

If the `/mp/appmsg_comment` response exposes a total comment count, convert it into:

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "The article title you opened",
  "url": "The article URL you opened",
  "comment_count": 88
}
```

### A.2.6 Send to the application

Save the two JSON files as:

```text
article_metrics.json
article_comments.json
```

Send them:

```bash
python3 send_payload.py 9000 < article_metrics.json
python3 send_payload.py 9000 < article_comments.json
```

If the article appears in the table, the Reqable route works.

### A.2.7 Reqable troubleshooting

| Symptom | Likely cause | Fix |
| --- | --- | --- |
| Phone cannot access the internet | Proxy IP or port is wrong | Check computer IP, Reqable port, and firewall |
| Only CONNECT requests are visible, no JSON | HTTPS certificate is not installed or trusted | Install and trust the Reqable certificate |
| `/mp/getappmsgext` is not visible | Article did not fully load or filter is wrong | Reopen the article and search for `getappmsgext` |
| No article appears in the app | JSON was not sent to the local bridge | Run the bridge smoke test and check the port |
| Repeated articles overwrite earlier records | Same URL deduplication is working | This is expected |

## A.3 Best automation route: mitmproxy / mitmweb

### A.3.1 When to use mitmproxy

Use mitmproxy when you want a long-term automated adapter. It can run a Python script that automatically converts target responses into application JSON.

### A.3.2 Install

Linux:

```bash
python3 -m pip install --user mitmproxy
```

macOS with Homebrew:

```bash
brew install mitmproxy
```

Windows:

```powershell
python -m pip install mitmproxy
```

Verify:

```bash
mitmproxy --version
mitmweb --version
```

### A.3.3 Start mitmweb

For beginners, start with `mitmweb` because it opens a web interface:

```bash
mitmweb --listen-host 0.0.0.0 --listen-port 8888
```

Parameter meaning:

| Parameter | Meaning |
| --- | --- |
| `--listen-host 0.0.0.0` | Allows the phone to connect to the computer proxy |
| `--listen-port 8888` | Proxy listens on port `8888` |

The mitmweb console usually opens at:

```text
http://127.0.0.1:8081
```

### A.3.4 Configure the phone proxy

Use the same phone proxy setup as Reqable:

- Proxy server: computer LAN IP.
- Proxy port: `8888`.

Then open this address in the phone browser:

```text
http://mitm.it
```

Install the certificate for your system. Install it only on your own test device. Do not commit the certificate.

### A.3.5 Confirm target endpoints in mitmweb

1. Open a WeChat Official Account article on the phone.
2. In mitmweb, use this filter:

```text
~u getappmsgext
```

3. Then try:

```text
~u appmsg_comment
```

If requests are visible, open the Response tab to inspect JSON.

### A.3.6 mitmproxy automatic adapter script

Save as:

```text
mitm_wechat_to_radar.py
```

Content:

```python
import json
import socket

BRIDGE_HOST = "127.0.0.1"
BRIDGE_PORT = 9000
DEFAULT_CATEGORY = "Uncategorized"


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
    for key in ["comment_count", "elected_comment_total_cnt", "total_count", "count"]:
        value = body.get(key)
        if isinstance(value, int):
            return value
        if isinstance(value, str) and value.isdigit():
            return int(value)

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

Start mitmproxy with the script:

```bash
mitmproxy --listen-host 0.0.0.0 --listen-port 8888 -s mitm_wechat_to_radar.py
```

Open an article on the phone. If the terminal prints:

```text
sent metrics ...
sent comments ...
```

and the article appears in the application table, the automated adapter works.

### A.3.7 mitmproxy notes

The automatic script may not always know the article title or publish time, because those values are often in the article HTML page rather than in the metrics response.

You have three choices:

1. Fill title and publish time manually during the first phase.
2. Extend the adapter to also capture and parse the article HTML page.
3. Use URL deduplication first and enrich title and publish time later through CSV or manual editing.

Recommended rollout: first make read, like, old-like, and comment metrics work; then automate title and publish-time enrichment.

## A.4 Windows GUI route: Fiddler Everywhere

### A.4.1 Install and start

1. Install Fiddler Everywhere.
2. Start it and enable capturing.
3. In settings, enable HTTPS traffic capture.
4. Follow the prompts to install and trust the certificate.

### A.4.2 Configure proxy

For local browser capture, Fiddler often configures the system proxy automatically.

For phone WeChat capture:

- Phone Wi-Fi proxy server: computer LAN IP.
- Proxy port: the port shown by Fiddler, often `8866` or the value displayed in the UI.

### A.4.3 Filter target endpoints

Search or filter URLs:

```text
getappmsgext
appmsg_comment
```

Open the request and inspect the response body.

### A.4.4 Export and send

Copy the response body, extract `appmsgstat`, convert it to `article_metrics.json`, and send:

```powershell
python send_payload.py 9000 < article_metrics.json
python send_payload.py 9000 < article_comments.json
```

## A.5 Charles Proxy route

### A.5.1 Install and configure

1. Install Charles.
2. Open Proxy settings.
3. Enable the system proxy or configure phone Wi-Fi proxy to the computer IP.
4. Open:

```text
Proxy -> SSL Proxying Settings
```

5. Add Host:

```text
mp.weixin.qq.com
```

6. Set Port:

```text
443
```

7. Install and trust the Charles certificate on the phone or computer.

### A.5.2 Capture endpoints

After opening an article, find this host in Charles:

```text
mp.weixin.qq.com
```

Then find paths:

```text
/mp/getappmsgext
/mp/appmsg_comment
```

Copy JSON responses, convert them to the application JSON contract, and send them to the local bridge.

## A.6 whistle route

### A.6.1 When to use whistle

whistle is useful for users who know Node.js and want a long-running rule-based proxy.

### A.6.2 Install

```bash
npm install -g whistle
w2 start -p 8888
```

Open the management UI:

```text
http://127.0.0.1:8899
```

Configure your browser or phone proxy to the computer IP and port `8888`.

Install the HTTPS root certificate following the whistle UI instructions.

### A.6.3 Filter endpoints

In the whistle Network panel, search:

```text
getappmsgext
appmsg_comment
```

Copy the response body and convert it into the application JSON contract.

Advanced users can write a whistle plugin or rule to forward target responses to a local adapter. Beginners should first validate the manual copy workflow.

## A.7 Proxyman route

Proxyman is mainly for macOS users.

Steps:

1. Install Proxyman.
2. Start proxy capture.
3. Enable SSL Proxying for `mp.weixin.qq.com`.
4. For phone capture, configure phone Wi-Fi proxy to the Mac IP and Proxyman port.
5. Install and trust the certificate.
6. Open an article.
7. Search:

```text
getappmsgext
appmsg_comment
```

8. Copy the response body, convert it into JSON, and send it to the local bridge.

## A.8 HTTP Toolkit route

HTTP Toolkit is suitable for developers debugging HTTP traffic.

Steps:

1. Install HTTP Toolkit.
2. Choose browser interception or system proxy interception.
3. For phone capture, follow its instructions to configure phone proxy and certificate.
4. Open an article.
5. Search for the target endpoints.
6. Copy the response body and convert it to JSON.

## A.9 Complete field mapping

### Common `/mp/getappmsgext` fields

| Source field | Application field | Example |
| --- | --- | --- |
| `appmsgstat.read_num` | `appmsgstat.read_num` | `24000` |
| `appmsgstat.like_num` | `appmsgstat.like_num` | `1200` |
| `appmsgstat.old_like_num` | `appmsgstat.old_like_num` | `300` |
| `read_num` | `appmsgstat.read_num` | Some tools flatten the field |
| `like_num` | `appmsgstat.like_num` | Some tools flatten the field |
| `old_like_num` | `appmsgstat.old_like_num` | Some tools flatten the field |

### Common `/mp/appmsg_comment` fields

Different responses may use different comment structures. Use this priority order:

| Source field | Application field | Note |
| --- | --- | --- |
| `comment_count` | `comment_count` | Best case |
| `elected_comment_total_cnt` | `comment_count` | Common candidate |
| `total_count` | `comment_count` | Common candidate |
| `count` | `comment_count` | Common candidate |
| length of `elected_comment` array | `comment_count` | Fallback when total count is missing |
| length of `comment` array | `comment_count` | Fallback when total count is missing |

### Publish-time fields

The application accepts these input field names:

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

Supported formats:

```text
2026-06-05T08:30:00Z
2026-06-05 08:30:00
2026-06-05
1717576200
1717576200000
```

If the proxy response does not contain publish time, copy it manually from the article page and put it into `publish_time`.

## A.10 Full manual example from proxy response to app JSON

Assume the proxy shows this `/mp/getappmsgext` response:

```json
{
  "appmsgstat": {
    "read_num": 35678,
    "like_num": 1350,
    "old_like_num": 260
  }
}
```

You see this article information on the page:

```text
Title: AI Product Growth Playbook
Publisher: Growth Lab
URL: https://mp.weixin.qq.com/s/abc123
Publish time: 2026-06-05 08:30:00
Comment count: 96
```

Create `metrics.json`:

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "AI Product Growth Playbook",
  "url": "https://mp.weixin.qq.com/s/abc123",
  "account_name": "Growth Lab",
  "gzh_id": "",
  "category": "Operations",
  "publish_time": "2026-06-05 08:30:00",
  "article_count_30d": 0,
  "appmsgstat": {
    "read_num": 35678,
    "like_num": 1350,
    "old_like_num": 260
  }
}
```

Create `comments.json`:

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "AI Product Growth Playbook",
  "url": "https://mp.weixin.qq.com/s/abc123",
  "comment_count": 96
}
```

Send:

```bash
python3 send_payload.py 9000 < metrics.json
python3 send_payload.py 9000 < comments.json
```

Verify in the app:

| Table column | Expected value |
| --- | --- |
| Title | AI Product Growth Playbook |
| Account | Growth Lab |
| Publish Time | 2026-06-05 08:30 |
| Read | 35678 |
| Like | 1350 |
| Old Like | 260 |
| Comment | 96 |

## A.11 Security boundaries

Never save the following items in the project directory:

```text
Cookie
Header
Token
Certificate
Raw HAR capture
Full raw response archive
Account password
QR-code screenshot
Logs containing personal information
```

Recommended local directories:

```text
~/premium-radar-ingestion/raw-temp/      temporary raw samples, delete after debugging
~/premium-radar-ingestion/sanitized/     sanitized JSON payloads
~/premium-radar-ingestion/scripts/       adapter scripts
```

Delete temporary samples after debugging:

```bash
rm -rf ~/premium-radar-ingestion/raw-temp/*
```

---

# Option B: manual assisted adapter

## B.0 Who should use Option B

Option B is for users who do not want to configure a proxy and only need stable ingestion of selected articles. It does not capture packets, install proxy certificates, or control a phone.

You only need to:

1. Open the article manually.
2. Copy title, URL, and publish time.
3. Enter read count, like count, old-like count, and comment count.
4. Send CSV or JSON to the app.

## B.1 Single-article JSON import

### Step 1: Create `manual_metrics.json`

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "Manually entered article title",
  "url": "https://mp.weixin.qq.com/s/example",
  "account_name": "Publisher name",
  "gzh_id": "",
  "category": "Operations",
  "publish_time": "2026-06-05 08:30:00",
  "article_count_30d": 0,
  "appmsgstat": {
    "read_num": 10000,
    "like_num": 500,
    "old_like_num": 100
  }
}
```

### Step 2: Create `manual_comments.json`

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "Manually entered article title",
  "url": "https://mp.weixin.qq.com/s/example",
  "comment_count": 35
}
```

### Step 3: Send

```bash
python3 send_payload.py 9000 < manual_metrics.json
python3 send_payload.py 9000 < manual_comments.json
```

## B.2 CSV batch import

### Step 1: Prepare CSV

Save as:

```text
manual_articles.csv
```

Content:

```csv
title,url,account_name,gzh_id,category,publish_time,article_count_30d,read_num,like_num,old_like_num,comment_count
Article A,https://mp.weixin.qq.com/s/a,Publisher A,gh_a,Technology,2026-06-05 08:30:00,10,24000,1200,300,88
Article B,https://mp.weixin.qq.com/s/b,Publisher B,gh_b,Finance,2026-06-04 12:00:00,6,12000,300,90,25
```

### Step 2: Save conversion script

Save as:

```text
csv_to_bridge.py
```

Content:

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
            "category": row.get("category", "Uncategorized"),
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

### Step 3: Run

```bash
python3 csv_to_bridge.py 9000 manual_articles.csv
```

Windows:

```powershell
python csv_to_bridge.py 9000 manual_articles.csv
```

## B.3 Spreadsheet import route

If you prefer Excel, WPS, or another spreadsheet tool, create a sheet with exactly these column names:

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

Export the sheet as UTF-8 CSV, then import it with `csv_to_bridge.py`.

## B.4 Option B acceptance checks

| Check | Success criteria |
| --- | --- |
| Single JSON import | One article appears in the app |
| CSV batch import | Each CSV row is inserted or used to update an existing URL |
| Publish time | The table shows publish time |
| Comment count | The comment column has a value |
| Export | Exported CSV and JSON contain `publish_time` |

---

# Option C: ADB phone automation adapter

## C.0 Who should use Option C

Option C does not directly obtain metrics. It only helps open articles, scroll, and tap on a dedicated test phone. Real metrics still come from:

- Option A proxy capture; or
- Option B manual or CSV import.

So Option C is not a replacement for Option A. It is an assistant for Option A or Option B.

## C.1 Prepare a dedicated test phone

Use a test phone instead of your daily phone.

Phone requirements:

| Requirement | Explanation |
| --- | --- |
| Logged in to an approved test WeChat environment | Avoid high-risk batch tests on your main account |
| Developer options enabled | Required for USB debugging |
| USB debugging enabled | Allows the computer to control the phone with adb |
| Stable connection to the computer | Use a reliable USB cable |
| If using Option A | Phone Wi-Fi proxy points to the computer proxy port |

## C.2 Install ADB

Linux:

```bash
sudo apt-get update
sudo apt-get install -y android-tools-adb
```

Windows:

1. Download Android Platform Tools.
2. Extract it to a directory such as:

```text
C:\platform-tools
```

3. Add that directory to PATH.
4. Open PowerShell and run:

```powershell
adb devices
```

macOS:

```bash
brew install android-platform-tools
```

## C.3 Connect the phone

Run:

```bash
adb devices
```

Expected output:

```text
List of devices attached
DEVICE_SERIAL    device
```

Troubleshooting:

| Output | Meaning | Fix |
| --- | --- | --- |
| `unauthorized` | Phone has not approved the computer | Approve the prompt on the phone |
| Empty list | Computer cannot detect the phone | Change cable, install driver, re-enable USB debugging |
| `offline` | ADB connection is unstable | Replug USB and run `adb kill-server && adb start-server` |

## C.4 Enable ADB in the app

In the WeChat settings page:

1. Check Enable ADB Automation.
2. Click Save Runtime Settings.
3. Reload the plugin or restart the application.

If you only use Option A or B, keep ADB disabled.

## C.5 Common ADB commands

Inspect the current phone window:

```bash
adb shell dumpsys window | grep -E "mCurrentFocus|mFocusedApp"
```

Take a screenshot:

```bash
adb exec-out screencap -p > screen.png
```

Tap:

```bash
adb shell input tap 500 1200
```

Scroll:

```bash
adb shell input swipe 500 1600 500 600 500
```

Go back:

```bash
adb shell input keyevent KEYCODE_BACK
```

Open WeChat. The package name may vary by version, but this is a common command:

```bash
adb shell monkey -p com.tencent.mm 1
```

## C.6 Real workflow: C plus A

1. Start Premium Content Radar on the computer.
2. Keep the application bridge port as `9000`.
3. Start Reqable, mitmproxy, Charles, or Fiddler on the computer with a proxy port such as `8888`.
4. Configure the test phone Wi-Fi proxy to the computer IP and `8888`.
5. Install and trust the proxy certificate on the test phone.
6. Use ADB to open WeChat.
7. Use ADB to tap the target Official Account article.
8. The proxy captures `/mp/getappmsgext` and `/mp/appmsg_comment`.
9. The adapter sends JSON to `127.0.0.1:9000`.
10. The application stores the article.

## C.7 Real workflow: C plus B

1. Use ADB to open an article.
2. Use ADB screenshot for human confirmation.
3. Copy title, URL, publish time, and visible metrics.
4. Fill the CSV file.
5. Import with `csv_to_bridge.py`.

## C.8 What Option C must not do

Do not allow scripts to:

- Automatically send WeChat messages.
- Automatically add contacts.
- Change account settings.
- Read contacts.
- Run high-frequency blind taps.
- Run unattended for long periods on your main phone.

Start with batches of 5 to 10 articles. Increase only after the process is stable.

---

# Choosing between A/B/C

| Need | Recommended option |
| --- | --- |
| No proxy setup | B |
| Fastest path to real data | A with Reqable manual copy first |
| Long-term automation | A with mitmproxy scripting |
| Batch article opening without automatic metric capture | C plus B |
| Batch article opening with automatic metric capture | C plus A |
| Lowest compliance risk | B |
| Highest efficiency | A or C plus A |

Recommended rollout order:

1. Use Option B JSON or CSV to prove ingestion.
2. Use Option A with Reqable to manually capture real metrics.
3. Use Option A with mitmproxy for automatic forwarding.
4. Consider Option C only after that, and let ADB only open articles.

---

# Production checklist

| Check | Required result |
| --- | --- |
| Bridge smoke test | Passed |
| Article table | Shows real or manually imported articles |
| Publish time | Present and exportable |
| Read, like, comment metrics | Match the source view |
| CSV and JSON export | Complete fields |
| Raw captures | Not committed and not stored long-term |
| Cookies, headers, tokens | Not logged, not scripted, not committed |
| Proxy certificates | Installed only on authorized test devices |
| ADB | Disabled by default and enabled only when needed |
| Windows startup | No command prompt window |

---

# Minimal practical path

If you want to start today, use this shortest path:

1. In the app, click Send Bridge Smoke Payload.
2. Use Option B to create one `manual_metrics.json` and one `manual_comments.json`.
3. Confirm that the article and publish time appear in the table.
4. Install Reqable.
5. Configure phone Wi-Fi proxy to Reqable.
6. Open one article and search for `getappmsgext`.
7. Copy read, like, and old-like values into JSON.
8. Send the JSON to the app.
9. Export CSV and review the fields.
10. Decide whether to move to mitmproxy automation or ADB later.

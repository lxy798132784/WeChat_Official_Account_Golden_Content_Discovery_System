# WeChat Ingestion Options A/B/C Implementation Guide

## Scope

This document describes three implementation options for connecting real WeChat Official Account article metrics to Premium Content Radar:

- Option A: local proxy adapter.
- Option B: browser or desktop assisted manual adapter.
- Option C: ADB phone automation adapter.

All options follow the same boundary: the app accepts only data the user is allowed to view, analyze, and store. The app does not store account passwords, cookies, headers, tokens, certificates, raw packet captures, or private account data in the repository. Every data source must first be converted locally into compact JSON and then sent to the `127.0.0.1` bridge.

## Shared preparation

### 1. Install and start the app

Download the package for your platform, extract it, and start the main executable:

```bash
./premium-content-radar
```

In the WeChat settings page, verify:

- Bridge port: default `9000`.
- ADB automation: disabled by default.
- Database path: points to the location where you want to keep local data.
- Plugin directory: points to the WeChat Provider plugin directory.

### 2. Verify the local bridge

In the graphical app, click the bridge smoke payload action. From the command line, run:

```bash
QT_QPA_PLATFORM=offscreen ./premium-content-radar --bridge-smoke
```

Connect real sources only after the smoke test passes. If it fails, fix bridge startup, port conflicts, firewall rules, or plugin loading first.

### 3. Unified JSON input contract

Options A, B, and C all end with the same JSON contract.

Article metrics:

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "Article title",
  "url": "https://mp.weixin.qq.com/s/example",
  "account_name": "Publisher name",
  "gzh_id": "publisher_id",
  "category": "Technology",
  "article_count_30d": 12,
  "appmsgstat": {
    "read_num": 24000,
    "like_num": 1200,
    "old_like_num": 300
  }
}
```

Comment metrics:

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "Article title",
  "url": "https://mp.weixin.qq.com/s/example",
  "comment_count": 88
}
```

### 4. Shared sender script

Save as `send_payload.py`. Do not put real cookies or raw responses in the script:

```python
import json
import socket
import sys

port = int(sys.argv[1]) if len(sys.argv) > 1 else 9000
payload = json.load(sys.stdin)
with socket.create_connection(("127.0.0.1", port), timeout=3) as sock:
    sock.sendall(json.dumps(payload, ensure_ascii=False).encode("utf-8"))
```

Send payloads:

```bash
python3 send_payload.py 9000 < sample_metrics.json
python3 send_payload.py 9000 < sample_comments.json
```

---

## Option A: local proxy adapter

### A.1 Goal

Option A uses a local proxy or export tool to observe WeChat article metric responses in a user-authorized environment. The adapter extracts read count, likes, old likes, comment count, and article metadata, maps them to the app JSON contract, and sends them to the local bridge.

This is the fastest route to real metrics and has the strongest automation potential, but it also requires the strictest compliance and sanitization discipline.

### A.2 Recommended architecture

```text
Phone WeChat, desktop WeChat, or browser
        |
        | user-configured proxy
        v
local proxy tool, filtering target endpoints only
        |
        | raw responses stay briefly in a secure location
        v
local adapter script, field mapping and sanitization
        |
        | compact JSON
        v
127.0.0.1:9000 local bridge
        |
        v
SQLite storage, scoring, filtering, export
```

### A.3 Steps

#### Step 1: Choose a proxy or export tool

Use any local tool you trust and are allowed to use. It must be able to expose response bodies for articles you open yourself. Filter only these endpoints:

```text
/mp/getappmsgext
/mp/appmsg_comment
```

Do not put proxy certificates, cookies, headers, tokens, or raw captures in the project directory.

#### Step 2: Configure the client proxy

Choose one path for your environment:

- Phone WeChat: point the phone Wi-Fi proxy to the computer LAN IP and proxy port.
- Desktop WeChat or browser: point the system or browser proxy to the local proxy port.
- Local browser only: configure browser-level proxy to avoid changing system-wide traffic.

If the tool requires an HTTPS inspection certificate, install it only on devices and accounts you control and are authorized to test. Never commit the certificate.

#### Step 3: Open articles and capture target endpoints

Open the target Official Account article manually. Confirm the proxy sees:

```text
/mp/getappmsgext
/mp/appmsg_comment
```

Confirm the response body is JSON or parseable text. Keep short temporary samples only for debugging. In production, prefer streaming processing and avoid long-term raw capture storage.

#### Step 4: Map fields

From `/mp/getappmsgext`:

| Source field | App field | Meaning |
| --- | --- | --- |
| `appmsgstat.read_num` | `read_num` | Read count |
| `appmsgstat.like_num` | `like_num` | Like count |
| `appmsgstat.old_like_num` | `old_like_num` | Old-like or wow-style metric, depending on source semantics |
| Current article title | `title` | From page title, export metadata, or adapter context |
| Current article URL | `url` | Used as a deduplication key |
| Publisher name | `account_name` | Optional but recommended |
| Publisher ID | `gzh_id` | Optional but recommended |
| Thirty-day article count | `article_count_30d` | Use 0 or seed metadata if unavailable |

From `/mp/appmsg_comment`:

| Source field | App field | Meaning |
| --- | --- | --- |
| Total comment count or list count | `comment_count` | Comment count |
| Current article title | `title` | Used to merge with article metrics |
| Current article URL | `url` | Used to merge with article metrics |

#### Step 5: Minimal adapter flow

The adapter should:

1. Read one proxy-exported response.
2. Check whether the URL matches a target endpoint.
3. Parse the JSON response body.
4. Fill title and URL from safe context.
5. Build the app contract JSON.
6. Send it to `127.0.0.1:9000`.
7. Log only sanitized counts and errors.

Pseudocode:

```python
for record in proxy_export_stream:
    if "/mp/getappmsgext" in record.url:
        payload = map_metrics(record.body, record.context)
        send_to_bridge(payload)
    elif "/mp/appmsg_comment" in record.url:
        payload = map_comments(record.body, record.context)
        send_to_bridge(payload)
```

#### Step 6: Acceptance checklist

- Bridge smoke test passes.
- The proxy sees target endpoints.
- Adapter logs contain no cookies, headers, or tokens.
- Real articles appear in the app table.
- Read, like, and comment counts match the proxy view.
- CSV and JSON exports contain expected fields.
- Historical data remains visible after the proxy is stopped.

### A.4 Risks and controls

- Risk: raw captures expose account data. Control: keep them in a secure directory and delete after debugging.
- Risk: proxy affects unrelated traffic. Control: configure proxy only for the browser or test device.
- Risk: endpoint fields change. Control: implement tolerant parsing and clear error logs.
- Risk: duplicate ingestion. Control: deduplicate by URL, title, publish time, or publisher ID.

---

## Option B: browser or desktop assisted manual adapter

### B.1 Goal

Option B avoids packet capture and phone control. The user opens articles manually and imports metrics through a browser helper, page script, clipboard, CSV, or manual form.

It has the safest compliance boundary and works well for curated samples, team analysis, and environments where proxying is not available. The tradeoff is lower automation and lower throughput.

### B.2 Recommended architecture

```text
User opens an article manually
        |
        | copy page data, export CSV, or use a local browser helper
        v
manual assisted adapter
        |
        | compact JSON or CSV-to-JSON
        v
127.0.0.1:9000 local bridge
        |
        v
SQLite storage, scoring, filtering, export
```

### B.3 Method 1: JSON file import

#### Step 1: Create sample files

Save `manual_metrics.json`:

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "Manually entered article title",
  "url": "https://mp.weixin.qq.com/s/example",
  "account_name": "Publisher name",
  "gzh_id": "publisher_id",
  "category": "Operations",
  "article_count_30d": 8,
  "appmsgstat": {
    "read_num": 10000,
    "like_num": 500,
    "old_like_num": 100
  }
}
```

Save `manual_comments.json`:

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "Manually entered article title",
  "url": "https://mp.weixin.qq.com/s/example",
  "comment_count": 35
}
```

#### Step 2: Send to the local bridge

```bash
python3 send_payload.py 9000 < manual_metrics.json
python3 send_payload.py 9000 < manual_comments.json
```

#### Step 3: Verify in the app

Open the article table and verify title, URL, publisher, reads, likes, and comments. Export CSV and confirm rows and columns.

### B.4 Method 2: CSV batch import

#### Step 1: Prepare CSV

Save `manual_articles.csv`:

```csv
title,url,account_name,gzh_id,category,article_count_30d,read_num,like_num,old_like_num,comment_count
Article A,https://mp.weixin.qq.com/s/a,Publisher A,gh_a,Technology,10,24000,1200,300,88
Article B,https://mp.weixin.qq.com/s/b,Publisher B,gh_b,Finance,6,12000,300,90,25
```

#### Step 2: Convert CSV rows to bridge JSON

Save `csv_to_bridge.py`:

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

Run:

```bash
python3 csv_to_bridge.py 9000 manual_articles.csv
```

### B.5 Method 3: browser helper workflow

Use this when the user opens an article and a small local helper builds JSON from the page title, URL, and manually entered metrics.

Steps:

1. Open the article in a browser.
2. Copy the article title and URL.
3. Enter read count, like count, old-like count, and comment count in a local form or command line.
4. Generate JSON and send it to the local bridge.

This approach does not read cookies, capture packets, or automatically call platform endpoints. It only records data that the user has already viewed and confirmed.

### B.6 Acceptance checklist

- Single JSON import works.
- CSV batch import works.
- Repeated imports do not create uncontrolled duplicates.
- Table, scoring, and export reflect manually entered data.
- No account password or proxy certificate is required.

### B.7 Risks and controls

- Risk: manual data entry errors. Control: validate fields before import and export for review after import.
- Risk: inconsistent CSV format. Control: use fixed column names and default missing numbers to 0.
- Risk: low throughput. Control: use it for curated samples or as a fallback for Options A and C.

---

## Option C: ADB phone automation adapter

### C.1 Goal

Option C uses a user-authorized test phone and ADB to scroll, tap, and open articles locally. It then combines with Option A or B to import metrics into the app.

This option is useful when higher operational throughput is needed and a dedicated test device/account is available. It has the highest environment dependency and must be explicitly confirmed before enabling.

### C.2 Recommended architecture

```text
Test phone with user-authorized account
        |
        | USB debugging, local computer only
        v
ADB automation script, taps, scrolls, opens articles
        |
        | optionally combined with local proxy capture
        v
local adapter field mapping
        |
        | compact JSON
        v
127.0.0.1:9000 local bridge
        |
        v
SQLite storage, scoring, filtering, export
```

### C.3 Prerequisites

- A dedicated test phone.
- A WeChat environment that is allowed to be used for testing.
- USB debugging enabled on the phone.
- `adb` installed on the computer.
- Explicit user approval for automated taps, scrolling, and article opening.
- For real metrics, combine this with Option A proxy capture or Option B manual import.
- ADB automation remains disabled by default and runs only after explicit enablement.

### C.4 Environment setup

#### Step 1: Install ADB

Linux example:

```bash
sudo apt-get update
sudo apt-get install -y android-tools-adb
```

On Windows, install Android Platform Tools and add `adb.exe` to PATH.

#### Step 2: Connect the phone

```bash
adb devices
```

Approve the prompt on the phone. Expected output:

```text
List of devices attached
DEVICE_SERIAL    device
```

If it shows `unauthorized`, approve or reset USB debugging authorization. If no device appears, check cable, driver, and developer options.

#### Step 3: Limit ADB scope

Use a dedicated test phone instead of a daily phone. Scripts should only perform whitelisted actions:

- Open WeChat or a confirmed page.
- Tap article entries within the confirmed scope.
- Scroll lists.
- Capture screenshots for human confirmation.
- Do not read contacts, send messages, or change account settings.

### C.5 App configuration

In the WeChat settings page:

1. Enable ADB automation.
2. Save runtime settings.
3. Confirm the bridge port remains `9000`.
4. If using proxy capture, point the test phone Wi-Fi proxy to the local proxy port.
5. Load the WeChat Provider plugin.

Environment variables can also enable it explicitly:

```bash
export PREMIUM_RADAR_ENABLE_ADB=1
export PREMIUM_RADAR_BRIDGE_PORT=9000
./premium-content-radar
```

### C.6 Automation flow

Minimal flow:

1. `adb devices` confirms the device is online.
2. Open WeChat.
3. The user confirms the target account or article list.
4. The script taps and scrolls.
5. After each article opens, wait for page loading.
6. If using the proxy path, the local proxy adapter captures `/mp/getappmsgext` and `/mp/appmsg_comment`.
7. If using the manual path, the user or helper records title, URL, and visible metrics.
8. The adapter sends JSON to the local bridge.
9. The app stores and scores the article.

### C.7 ADB command examples

Check devices:

```bash
adb devices
```

Inspect current window:

```bash
adb shell dumpsys window | grep -E "mCurrentFocus|mFocusedApp"
```

Capture screenshot for confirmation:

```bash
adb exec-out screencap -p > screen.png
```

Tap example:

```bash
adb shell input tap 500 1200
```

Scroll example:

```bash
adb shell input swipe 500 1600 500 600 500
```

These commands are examples only. Production scripts must be adapted to the phone resolution, app version, and confirmed layout. Do not run blind bulk tapping.

### C.8 Acceptance checklist

- `adb devices` consistently shows the device online.
- The ADB switch in the app is controllable and disabled by default.
- Automation runs only on a test phone and inside the user-confirmed scope.
- After an article opens, Option A or B sends metrics to the bridge.
- New article rows appear in the app and exports work.
- Automation stops when the app closes or ADB is disabled.

### C.9 Risks and controls

- Risk: mistaken taps or account impact. Control: use a test phone/account and whitelist actions.
- Risk: UI changes break coordinates. Control: capture screenshots before each run and make coordinates configurable.
- Risk: excessive automation. Control: rate limit, human confirmation, and batch limits.
- Risk: real metrics are still unavailable. Control: C only opens pages; metrics still come from Option A or B.

---

## Choosing between the options

| Option | Best for | Strength | Weakness | Priority |
| --- | --- | --- | --- | --- |
| A local proxy adapter | Fast real metric ingestion | Strong automation and complete metrics | Requires proxy and sanitization discipline | First |
| B manual assisted adapter | Curated samples, safest boundary, no proxy | Low risk and controllable | Low throughput | Second and fallback |
| C ADB phone automation | Dedicated test device and batch page opening | Higher operational efficiency | Highest environment and mis-tap risk | Third, after confirmation |

## Recommended rollout order

1. Validate Option B JSON/CSV import first. This proves the app contract and scoring pipeline.
2. Implement Option A next. Convert real endpoint metrics into the same JSON contract.
3. Evaluate Option C last. Let ADB open pages and assist operations only; do not let it store account secrets or raw private data.

## Production launch checklist

- Bridge smoke test passes.
- Data source is lawful and user-authorized.
- Adapter does not commit cookies, headers, tokens, certificates, or raw captures.
- JSON fields match the unified contract.
- SQLite database path is explicit.
- CSV/JSON exports can be reviewed.
- Logs contain sanitized statistics only.
- The local bridge closes when the app stops.
- If ADB is enabled, a test device, explicit approval, rate limits, and a stop mechanism are in place.

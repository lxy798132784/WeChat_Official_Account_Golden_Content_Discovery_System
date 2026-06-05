# Local Proxy Adapter Guide

## Goal

Option A connects real metrics through a lawful local proxy or adapter. The adapter watches user-authorized WeChat article metric responses, converts the useful fields into the Premium Content Radar JSON contract, and sends them to the app's localhost bridge.

The app does not ship a proxy, does not embed credentials, and does not bypass platform controls. Use this only for accounts, devices, and data flows you are allowed to inspect.

## Architecture

```text
WeChat client or browser
        |
        | user-authorized local proxy/exporter
        v
local adapter script
        |
        | compact JSON over TCP
        v
127.0.0.1:9000 Premium Content Radar bridge
        |
        v
SQLite + scoring + export
```

## Step 1: Start the app

1. Open Premium Content Radar.
2. Go to the WeChat settings tab.
3. Set the bridge port, default `9000`.
4. Keep ADB automation disabled.
5. Click Save runtime settings.
6. Load plugins.
7. Run Actions -> Send Bridge Smoke Payload.

Command-line smoke test:

```bash
QT_QPA_PLATFORM=offscreen ./build/premium-content-radar --bridge-smoke
```

## Step 2: Prepare a local proxy or export source

Use a tool that can legally expose the response body for these WeChat endpoints:

- `/mp/getappmsgext`
- `/mp/appmsg_comment`

Typical proxy workflow:

1. Run the proxy only on your own machine.
2. Configure the phone or desktop client to use the proxy.
3. Install the proxy certificate only if the tool and environment require HTTPS inspection and you are authorized to do so.
4. Open the article manually.
5. Confirm that the proxy can see the JSON response body.
6. Do not save raw private captures in the repository.

## Step 3: Convert proxy records

Send one compact JSON object per captured response to the local bridge.

Metrics object:

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

Comment object:

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "Article title",
  "url": "https://mp.weixin.qq.com/s/example",
  "comment_count": 88
}
```

## Step 4: Minimal sender

Save as `send_payload.py`:

```python
import json
import socket
import sys

port = int(sys.argv[1]) if len(sys.argv) > 1 else 9000
payload = json.load(sys.stdin)
with socket.create_connection(("127.0.0.1", port), timeout=3) as sock:
    sock.sendall(json.dumps(payload, ensure_ascii=False).encode("utf-8"))
```

Run:

```bash
python3 send_payload.py 9000 < sample_metrics.json
python3 send_payload.py 9000 < sample_comments.json
```

## Step 5: Adapter responsibilities

The adapter should:

- Read raw proxy/export responses outside the repository.
- Map fields into the documented contract.
- Send only useful fields to `127.0.0.1`.
- Drop credentials, cookies, headers, and unrelated payloads.
- Log sanitized counts and errors only.
- Retry failed sends with backoff.

## Step 6: Production checklist

- Bridge smoke test passes.
- Proxy can see authorized endpoint responses.
- Adapter emits only compact sanitized JSON.
- Article count increases in the app.
- Exports contain expected rows.
- Raw captures are deleted or stored in an approved secure location.

## Deferred options

Option B, manual/browser-assisted import, and Option C, ADB phone automation, are documented as future integration paths and should be discussed separately before implementation.

# Auto Ingestion Setup and Runbook

## Current auto-ingestion capability

v1.1.0 provides a controlled local automation lane. It is not a hidden WeChat crawler and it does not bypass normal access control.

The workflow is:

1. Prepare WeChat article URLs that you are allowed to open.
2. Put those URLs into the software's auto-ingestion queue.
3. The software uses ADB to open each URL on your connected Android test phone.
4. When WeChat loads the article, your local proxy adapter captures allowed article metric endpoints.
5. The proxy adapter sends sanitized compact JSON to the software's localhost bridge.
6. The software stores, scores, displays, and exports the article records.

After the phone, proxy, and queue are configured, the operator no longer needs to open every article manually.

The software does not store WeChat accounts, passwords, cookies, request headers, certificates, or raw packet captures.

---

## 1. Required environment

### 1.1 Dedicated Android test phone

Use a dedicated test phone if possible. Do not use your main personal phone for production experiments.

The phone should:

- Have WeChat installed.
- Be logged in to your own authorized WeChat environment.
- Be able to open WeChat official account articles normally.
- Allow USB debugging.
- Be on a network where a local proxy can be configured.

### 1.2 ADB tool

ADB lets the desktop app ask the test phone to open an article URL.

#### Windows

1. Download Android Platform Tools.
2. Extract it to a directory such as:

```text
C:\platform-tools
```

3. Add that directory to the system PATH.
4. Open PowerShell and run:

```powershell
adb version
```

If a version is printed, ADB is installed.

#### Linux

Ubuntu or Debian:

```bash
sudo apt update
sudo apt install -y adb
adb version
```

#### macOS

With Homebrew:

```bash
brew install android-platform-tools
adb version
```

### 1.3 Enable USB debugging on the phone

The exact menu names vary by phone vendor, but the usual flow is:

1. Open Settings.
2. Open About phone.
3. Tap Build number or Version number seven times to enable developer options.
4. Return to Settings and open Developer options.
5. Enable USB debugging.
6. Connect the phone to the computer using USB.
7. When the phone asks whether to allow USB debugging, allow it.

On the computer, run:

```bash
adb devices
```

A healthy result looks like:

```text
List of devices attached
ABCDEF123456    device
```

If it shows `unauthorized`, confirm the authorization prompt on the phone and run the command again.

### 1.4 Local proxy adapter

Opening the article is only the first step. Reads, likes, comments, and related metrics still enter through a local proxy adapter.

Recommended chain:

```text
mitmproxy / Reqable / Charles / Fiddler / whistle
        ↓
capture /mp/getappmsgext and /mp/appmsg_comment
        ↓
convert to compact JSON
        ↓
send to 127.0.0.1:9000
        ↓
Premium Content Radar stores the record
```

Suggested tools:

- Beginner-friendly: Reqable.
- Automation script path: mitmproxy.
- Windows desktop preference: Fiddler Everywhere.
- macOS desktop preference: Charles or Proxyman.

Detailed proxy instructions are in:

```text
docs/en/WECHAT_INGESTION_OPTIONS_ABC.md
```

---

## 2. Configure auto ingestion inside the software

### Step 1: Start the software

Run `premium-content-radar`.

The left control center now contains a page named:

```text
Auto Ingestion
```

### Step 2: Check the WeChat page

Open the WeChat page and verify:

- SQLite database path is correct.
- Plugin directory is correct.
- Local bridge port is usually `9000`.
- ADB automation can stay disabled at first.
- Click Save runtime settings.
- Click Load Plugins.
- Click Send local bridge smoke payload.

If the article table receives a test record, the local bridge works.

### Step 3: Prepare article URLs

Open the Auto Ingestion page. Paste one WeChat article URL per line.

Example:

```text
https://mp.weixin.qq.com/s/xxxxxxxxxxxxxxxxxxxxxx
https://mp.weixin.qq.com/s/yyyyyyyyyyyyyyyyyyyyyy
https://mp.weixin.qq.com/s/zzzzzzzzzzzzzzzzzzzzzz
```

Parameterized article URLs are also accepted, for example:

```text
https://mp.weixin.qq.com/s?__biz=xxx&mid=xxx&idx=1&sn=xxx
```

The software accepts WeChat article URLs only. Normal website URLs are rejected.

### Step 4: Add URLs to the queue

Click:

```text
Add URLs
```

Valid URLs are added to the queue. Duplicate URLs are skipped.

Queue columns:

| Column | Meaning |
|---|---|
| Status | pending, opening, opened, failed |
| Attempts | Number of ADB open attempts |
| Account | Reserved metadata field |
| Category | Reserved metadata field |
| Last Attempt | Last ADB open time |
| URL / Error | Article URL or failure reason |

### Step 5: Enable the automation safety gate

Check:

```text
Allow ADB URL opening for this session
```

This is an explicit safety gate. Without it, the software will not run ADB open commands.

### Step 6: Test one URL first

Click:

```text
Run Next Now
```

The software runs a command equivalent to:

```bash
adb shell am start -a android.intent.action.VIEW -d "https://mp.weixin.qq.com/s/xxxx"
```

Expected result:

- The phone opens the WeChat article.
- The local proxy sees WeChat article metric requests.
- The proxy adapter sends compact metric JSON to the software.
- The article table receives or updates a record.

If the phone does nothing, run:

```bash
adb devices
```

and confirm that the device is online.

### Step 7: Start automatic ingestion

After the single-URL test succeeds, click:

```text
Start Auto Ingestion
```

The software opens queued URLs one by one according to the configured interval.

Recommended initial settings:

```text
Open interval: 30 seconds
Max attempts per URL: 3
```

Do not start with an aggressive interval. WeChat loading, proxy capture, and database ingestion all need time.

### Step 8: Stop automatic ingestion

Click:

```text
Stop
```

The scheduler stops opening the next URL. Already opened articles are not undone.

---

## 3. End-to-end checklist

### 3.1 Check ADB

```bash
adb devices
```

It must show:

```text
device
```

### 3.2 Check phone proxy

Open any web page on the phone. The proxy tool should display requests.

### 3.3 Check WeChat article loading

Open one official account article manually inside WeChat and confirm it loads.

### 3.4 Check the software bridge

In the software, click:

```text
Send local bridge smoke payload
```

The article table should receive test records.

### 3.5 Check automatic URL opening

In Auto Ingestion, click:

```text
Run Next Now
```

The phone should open the article automatically.

### 3.6 Check metric ingestion

After the proxy adapter captures `/mp/getappmsgext`, the article table should receive the real article record.

---

## 4. Troubleshooting

### Problem 1: Run Next Now does nothing

ADB is usually not connected.

Run:

```bash
adb devices
```

If no device appears:

- Check the USB cable.
- Check USB debugging.
- Confirm the phone authorization prompt.
- On Windows, check the phone driver.

### Problem 2: The phone opens the article, but the software receives no data

ADB worked, but proxy ingestion did not.

Check in this order:

1. The proxy tool is running.
2. The phone Wi-Fi proxy points to the computer IP and proxy port.
3. The proxy certificate is installed and trusted where required.
4. The proxy tool sees `/mp/getappmsgext`.
5. The adapter script sends JSON to `127.0.0.1:9000`.
6. The WeChat provider plugin is loaded in the software.

### Problem 3: The queue shows failed

The ADB command failed.

Common causes:

- `adb` is not in PATH.
- The phone is disconnected.
- The device is unauthorized.
- The phone does not allow external intents for the URL.
- The URL is not a valid WeChat article URL.

### Problem 4: Proxy data exists, but fields are incomplete

Check that the adapter JSON contains the expected fields:

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "Article title",
  "url": "Article URL",
  "account_name": "Official account name",
  "gzh_id": "Official account original ID",
  "category": "Category",
  "publish_time": "2026-06-05T08:30:00Z",
  "read_num": 10000,
  "like_num": 300,
  "old_like_num": 80,
  "comment_num": 25,
  "article_count_30d": 10
}
```

The `url` field is required because the software uses it for deduplication.

### Problem 5: The first URL opens, then nothing else happens

Check:

- Auto ingestion is still running.
- The queue still has pending or failed tasks below the retry limit.
- The interval is not set too high.
- Runtime logs do not show ADB errors.

---

## 5. Safety boundary

Auto ingestion only opens article URLs that you provide. It should not:

- Store WeChat passwords.
- Store cookies.
- Store request headers.
- Store proxy certificates.
- Store raw packet captures.
- Bypass access control.
- Scan data that you are not allowed to access.

Recommended stored fields are:

- Article URL.
- Title.
- Official account name.
- Publish time.
- Read count.
- Like count.
- Old-like count.
- Comment count.
- Category.
- Ingestion timestamp.

---

## 6. Recommended production flow

First-time setup:

```text
1. Install ADB
2. Connect the test phone
3. Configure the phone proxy
4. Start the proxy adapter
5. Start Premium Content Radar
6. Save WeChat ingestion settings
7. Load plugins
8. Send the local bridge smoke payload
9. Add URLs on the Auto Ingestion page
10. Enable the ADB safety gate
11. Run Next Now
12. Confirm that the article is stored
13. Start Auto Ingestion
```

Daily run:

```text
1. Connect the test phone
2. Start the proxy adapter
3. Start the software
4. Load or paste the URL queue
5. Click Start Auto Ingestion
6. Export CSV or JSON after ingestion
```

---

## 7. Current limitations

v1.1.0 Auto Ingestion Preview can automatically open a URL queue, but it still has limits:

1. Automatic discovery of the newest articles from an official account is not implemented yet.
2. The queue currently comes from user-provided URLs or imported queue files.
3. Metric capture still depends on a local proxy adapter.
4. ADB automation opens article URLs only; it does not perform complex UI recognition.
5. High-frequency batch opening is not recommended. Use at least a 30-second interval at first.

Planned upgrades:

- Official account article discovery.
- Generate pending URLs from the seed pool.
- Stronger proxy adapter scripts.
- Automatic status marking for stored, not captured, and retry-needed tasks.
- Daily caps, time windows, and more detailed rate limits.

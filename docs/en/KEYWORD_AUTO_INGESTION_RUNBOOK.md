# Keyword Auto Ingestion Runbook

## Product goal

The primary path is keyword-first auto ingestion. The operator should only need to:

1. Connect an authorized Android test phone.
2. Enter market keywords.
3. Set hot-article metric thresholds.
4. Click **Start Keyword Auto Ingestion**.

The app then automatically searches keyword result pages, extracts WeChat article candidate URLs, applies the hot-article controls, enqueues matching candidates, opens them on the phone through ADB, and waits for the local proxy adapter to send real article metrics to the localhost bridge.

## Hot-article metric controls

The Keyword Discovery page exposes these controls:

- **Minimum reads**: minimum `read_num` accepted for auto enqueue.
- **Minimum likes**: minimum `like_num` accepted for auto enqueue.
- **Minimum comments**: minimum `comment_num` accepted for auto enqueue.
- **Minimum hot score**: minimum estimated score accepted for auto enqueue.
- **Max per keyword**: maximum candidate article links parsed from each keyword search page.

Search pages may not expose real reads, likes, or comments. When those metrics are unavailable, candidates can still be opened when thresholds are set to zero; final hotness must be judged after the proxy captures `/mp/getappmsgext` and `/mp/appmsg_comment` from the loaded article.

The current estimated hot score is:

```text
hot_score = read_num + like_num * 20 + comment_num * 50
```

## Recommended production settings

For first-time validation:

```text
Minimum reads: 0
Minimum likes: 0
Minimum comments: 0
Minimum hot score: 0
Max per keyword: 5
```

This proves search, queueing, ADB opening, proxy capture, and database ingestion.

After proxy metrics are confirmed:

```text
Minimum reads: 10000
Minimum likes: 100
Minimum comments: 10
Minimum hot score: 15000
Max per keyword: 10
```

Tune thresholds by topic. Narrow B2B topics may need lower read thresholds; mass-market entertainment or consumer topics can use higher thresholds.

## Operation flow

1. Open the app.
2. Reopen the left panel with **Actions -> Show Control Center** or `Ctrl+B` if it was closed.
3. Open **WeChat Integration**.
4. Save database path, plugin directory, bridge port, and proxy/ADB settings.
5. Start the local proxy adapter and confirm it can send compact JSON to `127.0.0.1:9000`.
6. Connect the Android test phone and allow USB debugging.
7. Open **Keyword Discovery**.
8. Enter keywords, one per line.
9. Set the hot-article metric controls.
10. Click **Start Keyword Auto Ingestion**.
11. Watch **Runtime Logs**, **Auto Ingestion**, and the article table.

## What is automatic now

After the button click, the app automatically:

- Checks `adb devices`.
- Searches the keyword result page.
- Parses WeChat article URLs.
- Deduplicates URLs.
- Applies the hot-article metric controls.
- Adds matched URLs to the auto-ingestion queue.
- Starts the scheduler.
- Opens article URLs on the connected phone through ADB.

The local proxy adapter still provides the real metrics because WeChat article engagement fields are only reliably available when the article is opened in the authorized phone/browser environment.

## Troubleshooting

### No phone found

Run:

```bash
adb devices
```

Expected:

```text
SERIAL_NUMBER    device
```

If it says `unauthorized`, unlock the phone and allow USB debugging.

### Search returns zero candidates

Possible reasons:

- Search endpoint blocked automation or returned a captcha page.
- Network/proxy failed.
- Keyword too narrow.
- Search page markup changed.

Workarounds:

- Use broader keywords.
- Lower `Max per keyword` only after a successful test.
- Use **Generate Search URLs** for debugging.
- Use **Import Results JSON** as a fallback adapter path.

### Articles open but metrics do not appear

This means ADB opening worked but the proxy bridge did not ingest metrics. Check:

- Phone Wi-Fi proxy is pointing to the local proxy host/port.
- Proxy certificate is trusted if HTTPS interception is used.
- Adapter recognizes `/mp/getappmsgext` and `/mp/appmsg_comment`.
- App bridge port matches adapter target.

## Safety notes

- Do not store raw WeChat credentials, cookies, private keys, or proxy certificate secrets in this repository.
- Use only accounts and data you are authorized to analyze.
- Start with small limits to avoid noisy traffic and account risk.

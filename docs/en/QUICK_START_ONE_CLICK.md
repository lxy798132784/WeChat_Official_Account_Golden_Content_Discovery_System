# Quick Start: Connect Phone, Enter Keywords, One Click

## What this page does

The Quick Start page turns the previous Phone Diagnostics, Keyword Discovery, Auto Ingestion, and Production Suite steps into one guided flow. A normal user connects an Android phone, enters keywords, and clicks **One-click Discover & Open**.

## Steps

1. Enable Developer options and USB debugging on the phone.
2. Connect the phone with a data-capable USB cable and tap **Allow USB debugging**.
3. Open the **Quick Start** tab in the left Control Center.
4. Enter keywords, one per line.
5. Leave advanced filters at their defaults for the first run.
6. Click **One-click Discover & Open**.
7. Watch four status lines: phone readiness, keyword discovery, open queue, and metric payloads/recommendations.

## What the software does automatically

After the button is clicked, the app automatically:

- Checks whether ADB is available.
- Checks whether the phone is authorized.
- Loads local plugins and the localhost bridge.
- Searches keyword article candidates.
- Applies read, like, comment, and hot-score filters.
- Adds matched WeChat article URLs to the queue.
- Opens article URLs one by one on the phone through ADB.
- Waits for a lawful proxy adapter or replay data to send sanitized metrics to the local bridge.
- Refreshes the article library, data quality, trend tracking, scoring, and report entry points.

## Important boundary

Quick Start does not bypass WeChat and does not need cookies, request headers, tokens, certificates, or account passwords.

Without a lawful proxy adapter or sanitized imported data, the app can still automate:

- Keyword candidate discovery
- URL filtering and queueing
- Phone article opening

But real read, like, and comment metrics must come from data the user is allowed to access.

## Troubleshooting

### Phone preflight failed

Check:

- Android Platform Tools / adb is installed.
- USB debugging is enabled.
- The phone authorization dialog was accepted.
- The USB cable supports data transfer.
- If multiple phones are connected, select a target phone.

### No candidate URL was enqueued

The filters may be too strict. Lower minimum reads, likes, comments, or hot score and retry.

### Articles open but no metrics arrive

The phone opening path works, but metrics are not coming back. Start a lawful local proxy adapter, or import sanitized JSON/JSONL data in Replay Center.

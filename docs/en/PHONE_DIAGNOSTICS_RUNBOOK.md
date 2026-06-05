# Phone Diagnostics Runbook

## Purpose

Phone Diagnostics makes the automatic keyword ingestion path understandable for non-developer operators. Instead of saying only "ADB failed", the software checks every layer and explains what to fix.

The expected production path is:

1. Connect an authorized Android test phone.
2. Enter keywords.
3. Configure hot-article metrics.
4. Start keyword auto ingestion.
5. The app searches candidates, filters them, opens article URLs on the phone, and receives metrics through the local proxy bridge.

Phone Diagnostics protects that path by checking the phone before ingestion starts.

## Diagnostic layers

### 1. ADB tool

What it checks:

```bash
adb version
```

Why it matters:

If ADB is missing or not executable, the app cannot detect or control any Android phone.

Fix guidance:

- Install Android Platform Tools.
- Add the directory containing `adb` to PATH.
- On Windows, also install the phone vendor USB driver or Google USB Driver.

### 2. ADB server

What it checks:

```bash
adb start-server
adb devices -l
```

Why it matters:

ADB may be installed but the server may be stopped, blocked, or stale.

Available UI action:

```text
Restart ADB
```

It runs:

```bash
adb kill-server
adb start-server
```

### 3. Phone detection

What it checks:

```bash
adb devices -l
```

Supported states:

- `device`: phone is connected and authorized.
- `unauthorized`: phone is connected but USB debugging is not authorized.
- `offline`: phone is visible but not controllable.
- no rows: no phone is visible to ADB.
- multiple devices: choose one target serial in the UI.

### 4. USB authorization

If the state is `unauthorized`, unlock the phone, enable Developer options and USB debugging, replug USB, then tap **Allow** on the phone authorization dialog.

### 5. Shell control

What it checks:

```bash
adb -s SERIAL shell getprop ro.product.model
adb -s SERIAL shell getprop ro.build.version.release
adb -s SERIAL shell echo ok
```

Why it matters:

Auto ingestion needs real shell control to open article URLs.

### 6. Open article link

Available UI action:

```text
Test Open Link
```

It runs an Android VIEW intent:

```bash
adb -s SERIAL shell am start -a android.intent.action.VIEW -d "https://mp.weixin.qq.com/s/test"
```

This test may change the phone screen, so the full diagnostics panel allows you to skip it unless explicitly requested.

### 7. Local proxy port

If you configure a proxy port, the app checks whether `127.0.0.1:PORT` is reachable on the computer.

This only verifies that the proxy process is listening. You still need to configure the phone Wi-Fi proxy so phone traffic reaches the proxy adapter.

### 8. Local bridge port

The app checks whether the configured local bridge port is reachable. The bridge receives compact sanitized JSON payloads from the proxy adapter.

Use the WeChat Integration page's bridge smoke button to send sample `/mp/getappmsgext` and `/mp/appmsg_comment` payloads.

## Platform-specific guidance

### Windows

Install:

- Android Platform Tools
- Phone vendor USB driver or Google USB Driver

Check Device Manager. The phone should appear as Android ADB Interface, not Unknown Device.

### Linux

Install `android-tools-adb`. If ADB reports permission problems, configure udev rules for the phone vendor id, reload udev, replug the phone, then restart ADB.

Typical commands:

```bash
sudo usermod -aG plugdev $USER
sudo udevadm control --reload-rules
sudo udevadm trigger
adb kill-server
adb start-server
```

### macOS

Install Android Platform Tools. Vendor USB drivers are usually not required. Use a data-capable USB cable and make sure the phone trusts this computer.

## Preflight gate

When the user clicks **Start Keyword Auto Ingestion**, the app now runs a phone preflight check first. The ingestion does not start unless these core items pass:

- ADB tool
- ADB server
- phone detection
- USB authorization
- shell control
- target serial selection

If preflight fails, the app opens the Phone Diagnostics tab and explains the blocker.

## Reports

The UI provides:

- Copy Report: text report for support.
- Export JSON: structured report with status, target serial, devices, details, fix hints, and raw command output.

Do not paste private tokens, cookies, certificates, or raw WeChat captures into diagnostics reports.

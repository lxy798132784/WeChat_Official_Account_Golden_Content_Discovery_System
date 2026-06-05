# Developer Guide

## Architecture

Premium Content Radar is a Qt Widgets application with a source-first CMake layout.

Main components:

- `premium-content-radar`: desktop host executable.
- `radar_core`: static core library for records, scoring, storage, settings, export, plugin manager, and bridge helpers.
- `WeChatProviderPlugin`: runtime content provider loaded through `QPluginLoader`.
- `radar_tests`: Qt Test suite for storage, export, scoring, and bridge parsing.

## Data flow

1. The app loads runtime settings from a user config JSON file.
2. The plugin manager scans the configured plugin directory.
3. Providers start and expose records through `drainRecords()`.
4. Records are written to SQLite with URL-based upsert behavior.
5. The table is refreshed through `PremiumContentFilterProxyModel`.
6. Exports are generated from database query results.

## Runtime settings

`AppSettingsController` owns defaults and persistence. It stores:

- SQLite database path.
- Plugin directory.
- Local bridge port.
- ADB automation toggle.
- Sample-data startup toggle.

The WeChat provider also reads:

- `PREMIUM_RADAR_BRIDGE_PORT`.
- `PREMIUM_RADAR_ENABLE_ADB`.

The host sets these environment variables before loading plugins.

## Threading model

- UI work runs on the Qt main thread.
- `ProxyTrafficBridge` uses `QTcpServer` on localhost and a mutex-protected double buffer.
- `AdbAutomationEngine` is a `QThread` and is disabled by default.
- Database operations are executed by the host thread.

## Bridge parser contract

Accepted endpoints:

- `/mp/getappmsgext`
- `/mp/appmsg_comment`

Accepted metric keys:

- `read_num`
- `like_num`
- `old_like_num`
- `comment_num`
- `comment_count`
- nested `appmsgstat`
- nested `comment_info`

Unknown endpoints return no record.

## Testing

```bash
./scripts/verify-all.sh
```

The gate runs:

- CMake configure and build.
- CTest.
- Application self-test.
- Local bridge smoke test.
- Screenshot generation.
- Secret scan.
- Requirement audit.
- Documentation language split audit.

## Coding conventions

- Use C++20 and Qt idioms.
- Keep runtime artifacts out of git.
- Keep user-facing English docs free of Chinese text.
- Keep Chinese docs under `docs/zh-CN/`.
- Do not embed credentials or traffic captures.

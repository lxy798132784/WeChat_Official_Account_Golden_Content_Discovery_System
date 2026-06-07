# Changelog

## v1.4.11 - 2026-06-06

- Added stable WeChat Articles-tab detection for phone-side search automation.
- Added official-account article result selection that rejects ads, mini programs, account-only rows, and other non-article results.
- Added article metrics visibility detection for read/like/share/favorite/comment evidence after opening a WeChat article.
- Routed the phone-side automation through Articles tab selection before tapping an article, with fallback to the previous generic article-entry detector.
- Added adaptive ADB fallback coordinates based on the current phone resolution, including the verified ChatGPT result-card tap ratio.
- Added user-controllable multi-article collection criteria: max articles, minimum read/like/old-like/comment counts, per-article wait, and retry budget.
- Added collection summaries that separate attempted/opened/captured/accepted, threshold rejections, duplicate rejections, and true failures.
- Added CLI smoke gates for keyword search (`--wechat-search-smoke`) and controlled batch collection (`--wechat-collect-smoke`).
- Added regression coverage for Articles-tab detection, non-ad official-account article selection, metrics-visible article pages, adaptive layout, WebView/Finder rejection, and collection filtering.

## v1.4.10 - 2026-06-06

- Closed the WeChat search-to-article automation loop by detecting article entries in search results and tapping the best scored candidate.
- Added article page recognition so the automation can distinguish search result pages from opened article pages before waiting for metrics.
- Added phone screen unlock detection to the P0 preflight gate, preventing one-click automation from starting while the device is still on the lock screen.
- Added regression tests for article-entry detection, article-page detection, and the screen-unlocked readiness gate.
- Verified the localhost metrics bridge path with an offscreen smoke test: bridge payload ingestion persisted article metrics into SQLite.

## v1.0.1 - 2026-06-05

- Added persistent runtime settings for database path, plugin directory, bridge port, ADB toggle, and sample-data startup behavior.
- Added UI and CLI localhost bridge smoke testing.
- Strengthened production runbook documentation and clarified the lawful local adapter boundary.
- Split English and Chinese documentation and added a language separation audit gate.

## v1.0.0 - 2026-06-05

- Completed the Qt6/C++20 desktop host, plugin runtime, WeChat provider module, localhost bridge, scoring model, SQLite storage, seed pool management, CSV/JSON export, runtime logs, CI, package scripts, Dockerfile, governance docs, and GitHub Release workflow.


## 1.0.2 - UI i18n readiness

- Added a runtime English/Chinese language switch in the Help menu.
- Added localized labels, table headers, placeholders, action text, and tooltips for main operational controls.
- Added an in-app user manual tab and localized About manual text.
- Added detailed English and Chinese Option A/B/C ingestion implementation documents.

# Changelog

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

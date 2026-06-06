# Quick Start P0/P1/P2 Operation Guide

## P0 primary route: desktop candidate search plus phone opening

The default and recommended path remains:

1. Enter market keywords.
2. Connect an authorized Android phone.
3. Click one-click discovery and opening.
4. The app checks phone diagnostics, searches external WeChat article candidate pages, filters and de-duplicates candidates, enqueues supported WeChat article URLs or Sogou WeChat redirect candidates, and opens them on the phone.

Sogou WeChat result pages can provide candidate titles, accounts, publish-time snippets, and redirect URLs. They do **not** provide reliable read, like, comment, or old-like metrics. Those metrics still require a lawful local adapter/bridge callback, sanitized replay data, or another authorized data source after the article is opened.

## P1 supplemental sources

If the live search source returns too few candidates, paste either of the following into **supplemental sources** and enable **Use supplemental JSON / pasted links**:

- sanitized JSON result array, object with `articles/items/results/data`, or
- one WeChat article URL per line.

The app merges supplemental JSON candidates with live search results, deduplicates URLs, and keeps the same hot-score filters. Plain links are added directly to the phone-opening queue.

## P2 advanced in-WeChat search

Advanced in-WeChat search is off by default. It is a fallback, not the main route.

When enabled, the app can run conservative ADB commands that:

1. launch WeChat with `monkey -p com.tencent.mm 1`,
2. dump the current UI tree to `/sdcard/window.xml`,
3. tap the operator-provided search-box coordinates,
4. input the keyword with Android `input text`,
5. press Enter, and
6. optionally tap a result coordinate.

This mode needs manual coordinates because WeChat UI varies by version, device, resolution, input method, login state, and popups. The stable desktop-search path still runs as fallback.

## Privacy and safety boundary

The feature does not collect cookies, request headers, access tokens, certificates, chat content, or account passwords. Real read/like/comment metrics still need to come from a lawful local adapter or sanitized replay/import data.

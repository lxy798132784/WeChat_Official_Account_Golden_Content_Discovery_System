# User Guide

## Purpose

Premium Content Radar helps operators discover high-value WeChat Official Account articles from a local, user-controlled ingestion flow. The application is built for analysis, review, seed management, and export. It is not a credential store and it is not a remote crawler.

## Main screen

The application has four working areas:

1. Dashboard: seed count, article count, and top score.
2. Article table: title, publisher, category, metrics, score, and URL.
3. Control Center: filters, seed pool, WeChat runtime settings, and logs.
4. Menus: sample loading, export, plugin loading, preview, star seed, and bridge smoke test.

## First run

1. Start the app.
2. Open the WeChat tab in the Control Center.
3. Confirm the SQLite database path.
4. Confirm the plugin directory.
5. Keep ADB automation disabled unless a real phone workflow has been prepared.
6. Click Save runtime settings.
7. Click Load Plugins from the Plugins menu.

## Local bridge smoke test

Use Actions -> Send Bridge Smoke Payload, or run:

```bash
QT_QPA_PLATFORM=offscreen ./build/premium-content-radar --bridge-smoke
```

A successful smoke test proves the localhost JSON bridge accepts payloads and converts them into records.

## Real data ingestion

A lawful local data adapter should send compact JSON to `127.0.0.1:<bridge-port>`. The default port is `9000`.

Metrics payload:

```json
{
  "endpoint": "/mp/getappmsgext",
  "title": "Article title",
  "url": "https://example.local/article",
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

Comment payload:

```json
{
  "path": "/mp/appmsg_comment?action=getcomment",
  "title": "Article title",
  "url": "https://example.local/article",
  "comment_count": 88
}
```

Unknown endpoints are ignored.

## Filtering and scoring

The scoring model is:

```text
Score = W_eng * EngagementRate + W_comm * CommentDensity + W_freq * FrequencyScore
EngagementRate = (like_num + old_like_num) / read_num
CommentDensity = comment_num / read_num * 10000
FrequencyScore = min(100, article_count_30d / 30 * 100)
```

Use the filter panel to adjust industry, keyword, weights, and minimum read threshold.

## Seed management

The Seeds tab supports:

- Add or update a publisher seed.
- Delete a selected seed.
- Export seeds as CSV.
- Use Ctrl+S or Actions -> Star Seed to save the publisher from a selected article.

## Export

Use the File menu to export:

- Article CSV.
- Article JSON.
- Seed CSV.

Exports are local files selected by the user.

## Shortcuts

- Space: preview selected article.
- Ctrl+S: save selected publisher as a seed.
- Esc: reset filters.


## Language switch and tooltips

The app provides a runtime language switch in the Help menu. Use the Help menu language action to switch to Chinese, and use the corresponding Chinese Help menu action to switch back to English. The selected language is saved in the local runtime settings file and is restored the next time the app starts.

The user interface keeps languages separated. English mode shows English labels, menus, table headers, placeholders, tooltips, and the built-in guide. Chinese mode shows Chinese labels, menus, table headers, placeholders, tooltips, and the built-in guide. Controls should not show mixed Chinese and English strings in normal use.

Most operational controls include tooltips. Hover over filters, seed inputs, WeChat runtime settings, export actions, preview actions, and the bridge smoke action to see what the control does, how to use it, and the safety boundary.

## Built-in user manual

The left control center includes a `Guide` tab. This tab contains the in-app user manual for day-to-day operation:

1. Configure the WeChat runtime settings.
2. Save settings and load plugins.
3. Run the local bridge smoke test.
4. Choose ingestion Option A, B, or C.
5. Tune filters and scoring weights.
6. Preview selected articles and save publishers as seeds.
7. Maintain the seed pool.
8. Export articles or seeds.
9. Follow the local privacy and credential boundary.

The About dialog also shows the manual in the currently selected language.

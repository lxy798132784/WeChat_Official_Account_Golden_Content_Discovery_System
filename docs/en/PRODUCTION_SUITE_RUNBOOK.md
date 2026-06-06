# Production Suite Runbook

This document explains the production-oriented P0/P1/P2/P3 feature set in Premium Content Radar.

## P0: Onboarding and Data Quality

### Proxy Adapter Wizard

The Proxy Wizard validates Option A, the lawful local proxy adapter path. It checks:

1. local proxy port,
2. local bridge port,
3. whether the phone can reach this computer,
4. whether `/mp/getappmsgext` metric traffic has been observed,
5. whether `/mp/appmsg_comment` comment traffic has been observed.

The wizard does not capture private traffic by itself. It records visible readiness signals and produces a fix-oriented report.

### Data Quality

Before scoring or exporting reports, use Data Quality to inspect whether the article library is trustworthy:

- duplicate URLs,
- missing titles,
- missing accounts,
- missing URLs,
- suspicious metric ratios, such as likes or comments greater than reads,
- invalid publish times.

Treat blocked or warning rows as data problems to fix before business decisions.

## P1: Replay, Health, Trends, and Queue Observability

The Replay Center accepts sanitized JSON, JSON arrays, or JSONL records. Use it to test ingestion, scoring, reports, and UI without depending on a live phone session every time.

The Health Monitor summarizes:

- phone readiness,
- proxy readiness,
- local bridge readiness,
- database readiness,
- task queue status.

Trend Tracking compares repeated snapshots for the same article URL. It highlights read growth and recommends whether the article should be reviewed first, watched, or deprioritized. This turns the system from a static ranking table into an early hot-content discovery workflow.

## P2: Scoring Studio and Local Content Analysis

The Scoring Studio allows operators to tune:

- read weight,
- like weight,
- comment weight,
- old-like weight,
- originality weight.

It previews score formulas and ranks current records. It also produces compact account and keyword intelligence lines.

Content Analysis runs deterministic local rules before any AI integration. It checks:

- title length,
- value/tension trigger words,
- engagement density.

This page is AI-ready, but it does not fabricate model output. If no strong rule signal is found, it says so and asks for manual review.

## P3: Workspace, Reports, Privacy, and Delivery

The workspace page lets users name a business workspace and generate Markdown reports.

The diagnostic snapshot exports structured state without private credentials or raw packet captures.

The Privacy Center clearly states the production boundary:

- no cookies are stored,
- no headers are stored,
- no tokens are stored,
- no certificates are stored,
- no raw packet captures are stored,
- ADB only opens user-queued article URLs,
- the local bridge accepts sanitized JSON only.

The Delivery page provides a release handoff checklist for packaged Windows and Linux builds. Use it before handing the tool to non-developer operators.

## Recommended production flow

1. Run Phone Diagnostics.
2. Run the Proxy Wizard.
3. Confirm metric and comment interface hits.
4. Load or replay sanitized article samples.
5. Open Data Quality and fix warnings.
6. Use Trend Tracking to find fast-growing articles.
7. Tune scoring in Scoring Studio.
8. Run Content Analysis for title and engagement signals.
9. Generate a workspace report.
10. Review Privacy Center and Delivery before team handoff.

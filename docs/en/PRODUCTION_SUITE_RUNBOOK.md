# Production Suite Runbook

This document explains the production-oriented P0/P1/P2/P3 feature set added to Premium Content Radar.

## P0: Proxy Adapter Wizard

The Proxy Wizard helps operators validate Option A, the lawful local proxy adapter path.

It checks and explains:

1. local proxy port,
2. local bridge port,
3. whether the phone can reach this computer,
4. whether `/mp/getappmsgext` metric traffic has been observed,
5. whether `/mp/appmsg_comment` comment traffic has been observed.

The wizard does not capture private traffic by itself. It only records visible readiness signals and produces a fix-oriented report.

## P1: Replay Center and Health Monitor

The Replay Center accepts sanitized JSON, JSON arrays, or JSONL records. Use it to test ingestion, scoring, reports, and UI without depending on a live phone session every time.

The Health Monitor summarizes:

- phone readiness,
- proxy readiness,
- local bridge readiness,
- database readiness,
- task queue status.

This makes production operation observable instead of opaque.

## P2: Scoring Studio and Intelligence

The Scoring Studio allows operators to tune:

- read weight,
- like weight,
- comment weight,
- old-like weight,
- originality weight.

It previews score formulas and ranks the current records. It also produces compact account and keyword intelligence lines so users can see which accounts or categories are worth deeper tracking.

## P3: Workspace, Reports, and Privacy Center

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

## Recommended production flow

1. Run Phone Diagnostics.
2. Run the Proxy Wizard.
3. Confirm metric and comment interface hits.
4. Save a sanitized replay sample.
5. Use Replay Center to reproduce the ingestion path.
6. Tune scoring in Scoring Studio.
7. Generate a workspace report.
8. Review Privacy Center before team handoff.

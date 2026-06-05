# Production Runbook

## Production definition

A production deployment means a user-controlled desktop host with a lawful local data adapter sending documented JSON metrics to the localhost bridge. The app stores and analyzes the data locally.

## Preflight checklist

1. Confirm the operator owns or is authorized to access the data source.
2. Confirm the bridge port is local-only and not exposed to the LAN.
3. Confirm the SQLite database path is on durable storage.
4. Confirm exports are written to an approved local directory.
5. Keep ADB automation disabled until a phone test plan is approved.
6. Run `./scripts/verify-all.sh` on the target machine.
7. Run `--bridge-smoke` before connecting a real adapter.

## Runtime steps

1. Start the app.
2. Save runtime settings in the WeChat tab.
3. Load providers.
4. Run the bridge smoke payload.
5. Start the lawful local adapter.
6. Watch the Logs tab for ingestion count.
7. Review and export selected results.

## Monitoring

Watch:

- Logs tab for provider start failures.
- Article count for ingestion progress.
- SQLite database file size and disk free space.
- Export output checksums if files are shared downstream.

## Backup

Back up:

- SQLite database.
- Runtime settings JSON.
- Exported CSV and JSON files.

Do not back up credentials into the repository.

## Incident response

If data looks wrong:

1. Stop the local adapter.
2. Export current records for evidence.
3. Check adapter payload samples.
4. Run `--bridge-smoke`.
5. Verify parser unit tests.
6. Resume only after the payload contract is corrected.

## What still needs operator input

- Real WeChat account and device environment.
- Lawful local adapter or proxy that can emit the documented JSON payloads.
- Target OS for native production packaging.
- Production database location and backup policy.
- Whether ADB automation is allowed in the environment.

# Contributing

Thank you for improving Premium Content Radar.

## Before opening a pull request

Run the full local gate:

```bash
./scripts/verify-all.sh
```

## Rules

- Do not commit credentials, cookies, database files, traffic captures, logs, or packaged artifacts.
- Add tests or a clear manual verification note for behavior changes.
- Keep English documentation in English-only files.
- Keep Chinese documentation under `docs/README.zh-CN.md` and `docs/zh-CN/`.
- Update user, developer, plugin, and production documentation when changing the bridge contract.

## Pull request checklist

1. The branch is based on `main`.
2. The code builds locally.
3. Tests pass.
4. Documentation is updated.
5. No runtime artifacts or secrets are staged.

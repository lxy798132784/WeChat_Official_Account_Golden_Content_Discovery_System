# Security Policy

## Data boundary

Premium Content Radar is a local desktop analysis host. The repository does not include account passwords, cookies, private captures, or remote collection services.

## Local bridge

The bridge is designed for `127.0.0.1` only. Do not expose the bridge port to a LAN or the public internet. Unknown endpoints are ignored by design.

## Credential handling

Keep credentials in user-controlled secure storage, environment variables, or local configuration outside the repository. Never commit tokens, cookies, private traffic payloads, or production databases.

## Reporting a vulnerability

When reporting a security issue, include:

- A description of the issue.
- Reproduction steps using synthetic data.
- The affected commit or release.
- Expected and actual behavior.

Do not attach real credentials, cookies, private article data, or traffic captures.

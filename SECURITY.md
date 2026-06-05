# Security Policy / 安全策略

## Supported Versions / 支持版本

The latest `main` branch and published releases receive security fixes.

最新 `main` 分支和已发布版本接收安全修复。

## Reporting / 报告方式

Please open a private security advisory or contact the repository owner. Do not publish exploit details before a fix is available.

请通过 GitHub 私有安全公告或联系仓库所有者报告。修复前不要公开漏洞利用细节。

## Data Boundary / 数据边界

This application is local-first. The bridge only accepts local user-controlled JSON payloads and rejects unknown endpoints. Never commit real account credentials, decrypted private traffic, cookies, or tokens.

本应用本地优先。本地桥只接收用户控制的本地 JSON 载荷，并拒绝未知端点。禁止提交真实账号凭证、解密私有流量、Cookie 或 Token。

# Install and Uninstall / 安装与卸载

## Linux tarball / Linux 压缩包

```bash
tar -xzf premium-content-radar-*-linux-*.tar.gz
cd premium-content-radar-*-linux-*
QT_PLUGIN_PATH=./plugins ./bin/premium-content-radar
```

Uninstall by deleting the extracted directory and optional runtime database `wechat_radar.db`.

卸载：删除解压目录和可选运行数据库 `wechat_radar.db`。

## Build from source / 源码构建

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
ctest --test-dir build --output-on-failure
./build/premium-content-radar
```

## Windows / Windows

Use `scripts/package-windows.ps1` from a Windows machine with Qt 6 and CMake installed.

在安装 Qt 6 和 CMake 的 Windows 机器上运行 `scripts/package-windows.ps1`。

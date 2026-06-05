#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VERSION="${VERSION:-1.0.0}"
ARCH="$(uname -m)"
case "$ARCH" in
  x86_64) PLATFORM="linux-amd64" ;;
  aarch64|arm64) PLATFORM="linux-arm64" ;;
  *) PLATFORM="linux-$ARCH" ;;
esac
BUILD_DIR="$ROOT/build"
DIST_DIR="$ROOT/dist/$PLATFORM"
PACKAGE_ROOT="$DIST_DIR/premium-content-radar-$VERSION-$PLATFORM"

cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j"${JOBS:-2}"
ctest --test-dir "$BUILD_DIR" --output-on-failure
QT_QPA_PLATFORM=offscreen "$BUILD_DIR/premium-content-radar" --self-test

rm -rf "$DIST_DIR"
mkdir -p "$PACKAGE_ROOT/bin" "$PACKAGE_ROOT/plugins" "$PACKAGE_ROOT/docs"
cp "$BUILD_DIR/premium-content-radar" "$PACKAGE_ROOT/bin/"
cp "$BUILD_DIR"/plugins/*WeChatProviderPlugin* "$PACKAGE_ROOT/plugins/"
cp "$ROOT/README.md" "$ROOT/DEVELOPER_GUIDE.md" "$ROOT/LICENSE" \
  "$ROOT/CHANGELOG.md" "$ROOT/SECURITY.md" "$ROOT/CONTRIBUTING.md" "$ROOT/CODE_OF_CONDUCT.md" \
  "$PACKAGE_ROOT/docs/"
cp -R "$ROOT/docs" "$PACKAGE_ROOT/docs/project-docs"
cat > "$PACKAGE_ROOT/README-RUN.txt" <<'TXT'
Run:
  QT_PLUGIN_PATH=./plugins ./bin/premium-content-radar

运行：
  QT_PLUGIN_PATH=./plugins ./bin/premium-content-radar
TXT
(
  cd "$DIST_DIR"
  tar -czf "premium-content-radar-$VERSION-$PLATFORM.tar.gz" "premium-content-radar-$VERSION-$PLATFORM"
  sha256sum "premium-content-radar-$VERSION-$PLATFORM.tar.gz" > SHA256SUMS
)
printf 'Package written: %s\n' "$DIST_DIR/premium-content-radar-$VERSION-$PLATFORM.tar.gz"

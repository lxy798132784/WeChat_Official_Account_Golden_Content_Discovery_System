# syntax=docker/dockerfile:1.7
# Buildx examples / Buildx 示例：
#   docker buildx build --platform linux/amd64,linux/arm64 -t premium-content-radar:latest .

FROM --platform=$BUILDPLATFORM ubuntu:24.04 AS build
ARG TARGETPLATFORM
ARG BUILDPLATFORM
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    cmake g++ ninja-build qt6-base-dev qt6-tools-dev libgl1-mesa-dev \
    libxkbcommon-dev openssl ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build \
    && ctest --test-dir build --output-on-failure \
    && QT_QPA_PLATFORM=offscreen ./build/premium-content-radar --self-test

FROM ubuntu:24.04 AS runtime
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    libqt6widgets6 libqt6sql6 libqt6sql6-sqlite libqt6network6 \
    libgl1-mesa-dri mesa-utils xvfb x11-apps x11vnc fluxbox \
    && rm -rf /var/lib/apt/lists/*
# Runtime includes Mesa OpenGL plus x11vnc-based VNC projection support.
COPY --from=build /src/build/premium-content-radar /usr/local/bin/premium-content-radar
COPY --from=build /src/build/plugins /usr/local/lib/premium-content-radar/plugins
ENV QT_PLUGIN_PATH=/usr/local/lib/premium-content-radar/plugins
ENTRYPOINT ["/usr/local/bin/premium-content-radar"]

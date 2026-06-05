FROM ubuntu:24.04 AS build
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y cmake g++ ninja-build qt6-base-dev qt6-tools-dev libgl1-mesa-dev && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build && ctest --test-dir build --output-on-failure

FROM ubuntu:24.04 AS runtime
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y libqt6widgets6 libqt6sql6 libqt6sql6-sqlite libqt6network6 libgl1-mesa-dri xvfb x11-apps && rm -rf /var/lib/apt/lists/*
COPY --from=build /src/build/premium-content-radar /usr/local/bin/premium-content-radar
COPY --from=build /src/build/plugins /usr/local/lib/premium-content-radar/plugins
ENTRYPOINT ["/usr/local/bin/premium-content-radar"]

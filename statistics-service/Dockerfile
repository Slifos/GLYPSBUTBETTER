FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install --yes --no-install-recommends \
        build-essential \
        cmake \
        libgrpc++-dev \
        libgtest-dev \
        libprotobuf-dev \
        libspdlog-dev \
        ninja-build \
        pkg-config \
        protobuf-compiler \
        protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

COPY . .

RUN cmake -S . -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=ON \
    && cmake --build build --parallel \
    && ctest --test-dir build --output-on-failure

FROM ubuntu:24.04 AS runtime

RUN apt-get update \
    && apt-get install --yes --no-install-recommends \
        libgrpc++1.51t64 \
        libprotobuf32t64 \
        libspdlog1.12 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/statistics_service /usr/local/bin/statistics_service

EXPOSE 9090

ENTRYPOINT ["/usr/local/bin/statistics_service"]

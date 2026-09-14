FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install --yes --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        git \
        ninja-build \
        pkg-config \
        tar \
        unzip \
        zip \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --depth 1 https://github.com/microsoft/vcpkg.git /opt/vcpkg \
    && /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics

WORKDIR /src

COPY vcpkg.json ./
RUN /opt/vcpkg/vcpkg install --triplet x64-linux

COPY . .

RUN cmake -S . -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DVCPKG_TARGET_TRIPLET=x64-linux \
        -DBUILD_TESTING=OFF \
    && cmake --build build

FROM ubuntu:24.04 AS runtime

RUN apt-get update \
    && apt-get install --yes --no-install-recommends libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/statistics_service /usr/local/bin/statistics_service

EXPOSE 9090

ENTRYPOINT ["/usr/local/bin/statistics_service"]

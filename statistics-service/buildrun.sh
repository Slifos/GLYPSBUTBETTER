cmake -S . -B build && cmake --build build -j "$(nproc)" && ./build/statistics_service

# Statistics Service

`statistics-service` is a stateless C++ gRPC microservice for the Campus Connect
event management system. Given an event capacity and its current participant
count, it calculates the remaining places, occupancy percentage, and whether the
event is full. It stores no data and has no database.

## Architecture

The code follows a small layered architecture:

```text
gRPC transport layer
        ↓
business service layer
        ↓
domain/model layer
```

- `StatisticsGrpcService` handles protobuf messages, status codes, and logging.
- `StatisticsService` validates inputs and performs calculations without any gRPC
  dependency.
- `Statistics` is the business result model.

The business service is constructor-injected into the gRPC adapter. This keeps
transport concerns separate and allows unit tests to run without a server.

## Technologies

- C++20
- gRPC
- Protocol Buffers
- CMake 3.20 or newer
- vcpkg
- spdlog
- GoogleTest

## Build

Install the platform build prerequisites. On Fedora:

```bash
sudo dnf install gcc-c++ cmake git curl zip unzip tar ninja-build
```

Clone and bootstrap vcpkg if it is not already installed:

```bash
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh -disableMetrics
```

Configure and build with the vcpkg toolchain. Replace `/path/to/vcpkg` with the
absolute path to the cloned repository:

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build -j "$(nproc)"
```

The vcpkg manifest installs gRPC, Protobuf, spdlog, and GoogleTest. CMake runs
`protoc` automatically and writes all generated C++ files under `build/generated`.

## Editor support

CMake exports `build/compile_commands.json`, and the project `.clangd` file points
clangd-based editors such as Zed to it. Configure and build the project at least
once so generated protobuf headers exist, then restart the editor's language
server if necessary.

## Run

Start the server from the repository root:

```bash
./build/statistics_service
```

It listens on `0.0.0.0:9090`.

To build and run it with Docker instead:

```bash
docker build -t statistics-service .
docker run --rm -p 9090:9090 statistics-service
```

## Tests

Run all business unit tests with:

```bash
ctest --test-dir build --output-on-failure
```

The business tests call `StatisticsService` directly. Separate transport tests
verify protobuf response mapping and gRPC error status conversion. None of the
tests starts a network server.

## API

The service exposes the unary RPC:

```text
campusconnect.statistics.StatisticsService/GetEventStatistics
```

Request fields:

- `event_id` (`int64`): event identifier used for request logs
- `max_participants` (`int32`): event capacity; must be greater than zero
- `current_participants` (`int32`): registered participants; cannot be negative

Response fields:

- `remaining_places` (`int32`): available capacity, never negative
- `occupancy_rate` (`double`): percentage occupied; values above 100 are allowed
- `full` (`bool`): true when current participants meet or exceed capacity

Example request:

```text
event_id: 42
max_participants: 10
current_participants: 7
```

Example response:

```text
remaining_places: 3
occupancy_rate: 70
full: false
```

With `grpcurl` installed, the same request can be sent without server reflection
by supplying the proto file:

```bash
grpcurl -plaintext \
  -import-path proto \
  -proto statistics.proto \
  -d '{"eventId":42,"maxParticipants":10,"currentParticipants":7}' \
  localhost:9090 \
  campusconnect.statistics.StatisticsService/GetEventStatistics
```

Invalid participant values return gRPC `INVALID_ARGUMENT`. Unexpected service
errors are logged and returned as `INTERNAL` without exposing implementation
details.

cmake_minimum_required(VERSION 3.20)

# Use a developer's vcpkg installation automatically when VCPKG_ROOT is set.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE
        "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE FILEPATH "vcpkg toolchain file")
endif()

project(statistics_service VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proto/statistics.proto")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(GENERATED_SOURCES
    "${GENERATED_DIR}/statistics.pb.cc"
    "${GENERATED_DIR}/statistics.grpc.pb.cc")

add_custom_command(
    OUTPUT ${GENERATED_SOURCES}
           "${GENERATED_DIR}/statistics.pb.h"
           "${GENERATED_DIR}/statistics.grpc.pb.h"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${GENERATED_DIR}"
    COMMAND $<TARGET_FILE:protobuf::protoc>
    ARGS --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
         --cpp_out=${GENERATED_DIR}
         --grpc_out=${GENERATED_DIR}
         --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
         "${PROTO_FILE}"
    DEPENDS "${PROTO_FILE}" protobuf::protoc gRPC::grpc_cpp_plugin
    COMMENT "Generating C++ protobuf and gRPC sources"
    VERBATIM)

add_library(statistics_core STATIC
    src/StatisticsService.cpp
    src/StatisticsGrpcService.cpp
    ${GENERATED_SOURCES})

target_include_directories(statistics_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/headers"
    "${GENERATED_DIR}")

target_link_libraries(statistics_core PUBLIC
    gRPC::grpc++
    protobuf::libprotobuf
    spdlog::spdlog)

add_executable(statistics_service src/main.cpp)
target_link_libraries(statistics_service PRIVATE statistics_core)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    add_executable(statistics_tests
        tests/StatisticsServiceTest.cpp
        tests/StatisticsGrpcServiceTest.cpp)
    target_link_libraries(statistics_tests PRIVATE
        statistics_core
        GTest::gtest_main)
    gtest_discover_tests(statistics_tests)
endif()

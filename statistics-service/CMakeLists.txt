cmake_minimum_required(VERSION 3.20)

project(statistics_service VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(PROTO_FILE "${PROTO_DIR}/statistics.proto")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(PROTO_SOURCE "${GENERATED_DIR}/statistics.pb.cc")
set(PROTO_HEADER "${GENERATED_DIR}/statistics.pb.h")
set(GRPC_SOURCE "${GENERATED_DIR}/statistics.grpc.pb.cc")
set(GRPC_HEADER "${GENERATED_DIR}/statistics.grpc.pb.h")

add_custom_command(
    OUTPUT
        "${PROTO_SOURCE}"
        "${PROTO_HEADER}"
        "${GRPC_SOURCE}"
        "${GRPC_HEADER}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${GENERATED_DIR}"
    COMMAND $<TARGET_FILE:protobuf::protoc>
    ARGS
        --proto_path=${PROTO_DIR}
        --cpp_out=${GENERATED_DIR}
        --grpc_out=${GENERATED_DIR}
        --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
        "${PROTO_FILE}"
    DEPENDS
        "${PROTO_FILE}"
        protobuf::protoc
        gRPC::grpc_cpp_plugin
    COMMENT "Generating C++ protobuf and gRPC sources"
    VERBATIM
)

add_library(statistics_proto STATIC
    "${PROTO_SOURCE}"
    "${GRPC_SOURCE}"
)

target_include_directories(statistics_proto
    PUBLIC
        "${GENERATED_DIR}"
)

target_link_libraries(statistics_proto
    PUBLIC
        gRPC::grpc++
        protobuf::libprotobuf
)

add_library(statistics_business STATIC
    src/service/StatisticsService.cpp
)

target_include_directories(statistics_business
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_library(statistics_grpc STATIC
    src/grpc/StatisticsGrpcService.cpp
)

target_include_directories(statistics_grpc
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(statistics_grpc
    PUBLIC
        statistics_business
        statistics_proto
        spdlog::spdlog
)

add_executable(statistics_service
    src/main.cpp
)

target_link_libraries(statistics_service
    PRIVATE
        statistics_grpc
        spdlog::spdlog
)

include(CTest)

if(BUILD_TESTING)
    add_executable(statistics_service_tests
        tests/StatisticsServiceTest.cpp
    )

    target_link_libraries(statistics_service_tests
        PRIVATE
            statistics_business
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(statistics_service_tests)

    add_executable(statistics_grpc_tests
        tests/StatisticsGrpcServiceTest.cpp
    )

    target_link_libraries(statistics_grpc_tests
        PRIVATE
            statistics_grpc
            GTest::gtest_main
    )

    gtest_discover_tests(statistics_grpc_tests)
endif()

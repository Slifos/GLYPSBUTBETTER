#include <cstdlib>
#include <exception>
#include <memory>

#include <grpcpp/grpcpp.h>
#include <spdlog/spdlog.h>

#include "StatisticsGrpcService.hpp"
#include "StatisticsService.hpp"

/** Configures the gRPC server, binds port 9090, and blocks until shutdown. */
int main() {
    constexpr auto serverAddress = "0.0.0.0:9090";

    try {
        StatisticsService statisticsService;
        StatisticsGrpcService grpcService{statisticsService};

        grpc::ServerBuilder builder;
        builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
        builder.RegisterService(&grpcService);

        std::unique_ptr<grpc::Server> server = builder.BuildAndStart();
        if (!server) {
            spdlog::error("Failed to start statistics service on {}", serverAddress);
            return EXIT_FAILURE;
        }

        spdlog::info("Statistics service starting on {}", serverAddress);
        server->Wait();
    } catch (const std::exception& exception) {
        spdlog::error("Statistics service terminated unexpectedly: {}", exception.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

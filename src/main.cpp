#include <spdlog/spdlog.h>

#include "include/app.h"

int main(int argc, char* argv[]) {
    try {
        App app;
        app.run();
        return 0;
    } catch (const std::exception& e) {
        spdlog::critical("Fatal error: {}", e.what());
        return 1;
    }
}

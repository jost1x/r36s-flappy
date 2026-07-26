#include "include/app.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        App app;
        app.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}

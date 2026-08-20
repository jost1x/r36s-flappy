#include <exception>
#include <iostream>

#include "app.h"

int main() {
    try {
        App app;
        app.run();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }
}

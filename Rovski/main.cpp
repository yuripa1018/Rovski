#include "Rovski.hpp"
#include <stdexcept>
#include <iostream>
#include <cstdlib>

int main() {
    Rovski rovski;
    rovski.Init(800, 600);
    try{
        rovski.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    rovski.Clean();
    return EXIT_SUCCESS;
}

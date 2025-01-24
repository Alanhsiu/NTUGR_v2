#include <iostream>
#include <chrono>
#include "global.h"
#include "../basic/design.h"
#include "../gr/GlobalRouter.h"

extern "C" {
    #include "flute/flute.h"
}

int main(int argc, char* argv[]) {
    std::ios::sync_with_stdio(false);

    auto start_time = std::chrono::high_resolution_clock::now();
    std::cout << "=====================================" << std::endl;
    std::cout << "          GLOBAL ROUTING START       " << std::endl;
    std::cout << "=====================================" << std::endl;

    // Initialize parameters and design
    Parameters parameters(argc, argv);
    Design design(parameters);

    auto read_duration = std::chrono::high_resolution_clock::now() - start_time;
    std::cout << "[INFO] Time for Input Reading: " 
              << std::chrono::duration<double>(read_duration).count() 
              << " seconds" << std::endl;
    std::cout << "=====================================" << std::endl;

    // Execute global routing
    std::cout << "[INFO] Starting Global Routing..." << std::endl;
    GlobalRouter globalRouter(design, parameters);
    globalRouter.route();
    std::cout << "[INFO] Global Routing Completed." << std::endl;

    // Write routing result
    auto write_start_time = std::chrono::high_resolution_clock::now();
    std::cout << "[INFO] Writing Routing Results..." << std::endl;
    globalRouter.write();
    auto write_duration = std::chrono::high_resolution_clock::now() - write_start_time;
    std::cout << "[INFO] Time for Output Writing: " 
              << std::chrono::duration<double>(write_duration).count() 
              << " seconds" << std::endl;

    // Display total runtime
    auto total_duration = std::chrono::high_resolution_clock::now() - start_time;
    std::cout << "=====================================" << std::endl;
    std::cout << "           GLOBAL ROUTING END        " << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "[INFO] Total Runtime: " 
              << std::chrono::duration<double>(total_duration).count() 
              << " seconds" << std::endl;

    return 0;
}

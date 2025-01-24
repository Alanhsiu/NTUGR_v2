#pragma once

#include "utils/utils.h"
using utils::log;
using utils::logeol;
using utils::loghline;
using utils::logmem;
using namespace std;

// STL libraries
#include <iostream>
#include <string>
#include <csignal>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <set>
#include <tuple>
#include <bitset>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <climits>
#include <iomanip>
#include <map>
#include <queue>
#include <typeinfo>
#include <omp.h>

#include "utils/robin_hood.h"

struct Parameters {
    std::string cap_file;
    std::string net_file;
    std::string out_file;

    // Global routing parameters
    const int num_threads = 8;
    const bool stage2 = true;
    const bool stage3 = false;

    const int min_routing_layer = 1;
    const double max_detour_ratio = 0.1; // May change
    const int target_detour_count = 10;  // May change
    const double via_multiplier = 1.5;  // Adjustable (e.g., 1.0, 1.5, 2.0)

    const double cost_logistic_slope1 = 1.5;
    const double cost_logistic_slope2 = 0.5;
    // const double maze_logistic_slope = 0.5;
    const bool write_heatmap = false;
    const bool write_capacity = false;
    std::string heatmap_file = "/home/b09901066/ISPD-NTUEE/NTUGR_v2/heatmaps/heatmap.txt";
    std::string capacity_file = "/home/b09901066/ISPD-NTUEE/NTUGR_v2/heatmaps/capacity.txt";

    double UnitViaCost = 4.0; // Must be updated with actual value
    const double UnitViaDemand = 0.5; // Magic number

    Parameters(int argc, char* argv[]) {
        if (argc <= 1) {
            std::cerr << "[ERROR] Too few arguments provided.\n";
            exit(1);
        }

        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], "-cap") == 0) {
                cap_file = argv[++i];
            } else if (strcmp(argv[i], "-net") == 0) {
                net_file = argv[++i];
            } else if (strcmp(argv[i], "-output") == 0) {
                out_file = argv[++i];
            } else if (strcmp(argv[i], "library") == 0 || strcmp(argv[i], "-def") == 0 ||
                       strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-sdc") == 0) {
                // Skip unrecognized or unnecessary arguments
                ++i;
            } else {
                std::cerr << "[WARNING] Unrecognized argument: " << argv[i] << '\n';
            }
        }

        // Display parsed parameters
        std::cout << "=====================================\n";
        std::cout << "          PARAMETERS LOADED          \n";
        std::cout << "=====================================\n";
        std::cout << "Cap File : " << cap_file << '\n';
        std::cout << "Net File : " << net_file << '\n';
        std::cout << "Output   : " << out_file << '\n';
        std::cout << "=====================================\n";
    }
};

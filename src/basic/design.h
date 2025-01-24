#ifndef DESIGN_H
#define DESIGN_H

#include <string>
#include <vector>
#include <chrono>
#include <iostream>

#include "../global.h"
#include "dimension.h"
#include "layer.h"
#include "metrics.h"
#include "netlist.h"

class Design {
public:
    explicit Design(Parameters& params)
        : parameters(params) {
        auto start = std::chrono::high_resolution_clock::now();
        if (readCap(params.cap_file)) {
            std::cout << "[INFO] Time taken to read CAP: "
                      << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count()
                      << " seconds." << std::endl;
        }
        
        start = std::chrono::high_resolution_clock::now();
        if (readNet(params.net_file)) {
            std::cout << "[INFO] Time taken to read NET: "
                      << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count()
                      << " seconds." << std::endl;
        }
    }

    ~Design() = default;

    // Member functions
    bool readCap(const std::string& filename);
    bool readNet(const std::string& filename);

    void replaceChars(char* str);

    // Member variables
    Parameters& parameters;
    NetList netlist;
    std::vector<Layer> layers;
    Dimension dimension;
    Metrics metrics;
};

#endif  // DESIGN_H

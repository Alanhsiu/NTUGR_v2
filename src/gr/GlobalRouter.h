#pragma once
#include "../global.h"
#include "../basic/design.h"
#include "GridGraph.h"
#include "GRNet.h"

class GlobalRouter {
public:
    GlobalRouter(const Design& design, const Parameters& params);
    void route();
    void write();
    std::string cap_file_name = "partial_cap.txt";

private:
    const Parameters& parameters;
    GridGraph gridGraph;
    std::vector<GRNet> nets;

    // Routing
    void stagePatternRouting(std::vector<int>& netIndices, int threadNum, int& n1);
    void stagePatternRoutingWithDetours(std::vector<int>& netIndices, int threadNum, int& n2);
    void stageMazeRouting(std::vector<int>& netIndices);

    // Helper functions
    void separateNetIndices(std::vector<int>& netIndices, std::vector<std::vector<int>>& nonoverlapNetIndices) const;
    void sortNetIndices(std::vector<int>& netIndices) const;
    
    // Analysis
    void printStatistics() const;
    void writeExtractNetToFile(const std::vector<std::pair<Point, Point>>& extract_net, const std::string& filename) const;
    void write_partial_cap(const std::vector<std::vector<std::vector<double>>>& cap) const;
};

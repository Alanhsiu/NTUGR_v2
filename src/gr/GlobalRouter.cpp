
#include "GlobalRouter.h"
#include <chrono>
#include "MazeRoute.h"
#include "PatternRoute.h"

GlobalRouter::GlobalRouter(const Design& design, const Parameters& params)
    : gridGraph(design, params), parameters(params) {
    // Instantiate the global routing netlist
    const size_t numNets = design.netlist.nets.size();
    nets.reserve(numNets);
    for (const Net& baseNet : design.netlist.nets) {
        nets.emplace_back(baseNet, design, gridGraph);
    }
}

void GlobalRouter::route() {
    int n1 = 0, n2 = 0, n3 = 0; // For statistics
    int threadNum = parameters.num_threads;
    bool stage2 = parameters.stage2;
    bool stage3 = parameters.stage3;

    std::vector<int> netIndices;
    netIndices.reserve(nets.size());
    for (const auto& net : nets) {
        netIndices.emplace_back(net.getIndex());
    }

    PatternRoute::readFluteLUT();

    // Stage 1
    n1 = netIndices.size();
    auto t1 = std::chrono::high_resolution_clock::now();
    stagePatternRouting(netIndices, threadNum, n1);
    std::cout << "[INFO] Stage 1 completed in "
              << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t1).count()
              << " seconds." << std::endl;
    std::cout << "======================" << std::endl;

    if (stage2){    
        netIndices.clear();
        for (const auto& net : nets) {
            if (gridGraph.checkOverflow(net.getRoutingTree(), 0) > 0) {
                netIndices.push_back(net.getIndex());
            }
        }
        std::cout << "[INFO] " << netIndices.size() << " / " << nets.size() << " nets have overflows after Stage 1." << std::endl;
        std::cout << "======================" << std::endl;

        // Stage 2
        if (!netIndices.empty()) {
            n2 = netIndices.size();
            auto t2 = std::chrono::high_resolution_clock::now();
            stagePatternRoutingWithDetours(netIndices, threadNum, n2);
            std::cout << "[INFO] Stage 2 completed in "
                    << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t2).count()
                    << " seconds." << std::endl;
            std::cout << "======================" << std::endl;
        }
    }

    if (stage3){
        netIndices.clear();
        for (const auto& net : nets) {
            if (gridGraph.checkOverflow(net.getRoutingTree(), 2) > 0) {
                netIndices.push_back(net.getIndex());
            }
        }
        std::cout << "[INFO] " << netIndices.size() << " / " << nets.size() << " nets have overflows after Stage 2." << std::endl;
        std::cout << "======================" << std::endl;

        // Stage 3
        if (!netIndices.empty()) {
            n3 = netIndices.size();
            auto t3 = std::chrono::high_resolution_clock::now();
            stageMazeRouting(netIndices);
            std::cout << "[INFO] Stage 3 completed in "
                    << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t3).count()
                    << " seconds." << std::endl;
            std::cout << "======================" << std::endl;
        }
    }

    // Final Statistics and Outputs
    printStatistics();
    if (parameters.write_heatmap)
        gridGraph.writeHeatmap(parameters.heatmap_file);
    if (parameters.write_capacity)
        gridGraph.writeCapacity(parameters.capacity_file);
}

void GlobalRouter::stagePatternRouting(std::vector<int>& netIndices, int threadNum, int& n1) {
    std::cout << "[INFO] Stage 1: Pattern Routing" << std::endl;
    GridGraphView<bool> congestionView;
    gridGraph.extractCongestionView(congestionView);

    sortNetIndices(netIndices);

    std::vector<std::vector<int>> nonoverlapNetIndices(threadNum + 1);
    separateNetIndices(netIndices, nonoverlapNetIndices);

    for (size_t i = 0; i < nonoverlapNetIndices.size(); ++i) {
        std::cout << "[INFO] Thread " << i << " size: " << nonoverlapNetIndices[i].size() << std::endl;
    }

    omp_lock_t lock;
    omp_init_lock(&lock);

#pragma omp parallel for
    for (int i = 0; i < threadNum; ++i) {
        for (int j : nonoverlapNetIndices[i]) {
            PatternRoute patternRoute(nets[j], gridGraph, parameters);
            omp_set_lock(&lock);
            patternRoute.constructSteinerTree();
            omp_unset_lock(&lock);
            patternRoute.constructRoutingDAG();
            patternRoute.run();
            gridGraph.commitTree(nets[j].getRoutingTree());
        }
    }
    omp_destroy_lock(&lock);

    for (int j : nonoverlapNetIndices[threadNum]) {
        PatternRoute patternRoute(nets[j], gridGraph, parameters);
        patternRoute.constructSteinerTree();
        patternRoute.constructRoutingDAG();
        patternRoute.run();
        gridGraph.commitTree(nets[j].getRoutingTree());
    }
}

void GlobalRouter::stagePatternRoutingWithDetours(std::vector<int>& netIndices, int threadNum, int& n2) {
    std::cout << "[INFO] Stage 2: Pattern Routing with Detours" << std::endl;
    GridGraphView<bool> congestionView;
    gridGraph.extractCongestionView(congestionView);

    sortNetIndices(netIndices);

    std::vector<std::vector<int>> nonoverlapNetIndices(threadNum + 1);
    separateNetIndices(netIndices, nonoverlapNetIndices);

    for (size_t i = 0; i < nonoverlapNetIndices.size(); ++i) {
        std::cout << "[INFO] Thread " << i << " size: " << nonoverlapNetIndices[i].size() << std::endl;
    }

    omp_lock_t lock;
    omp_init_lock(&lock);

#pragma omp parallel for
    for (int i = 0; i < threadNum; ++i) {
        for (int j : nonoverlapNetIndices[i]) {
            GRNet& net = nets[j];
            gridGraph.commitTree(net.getRoutingTree(), true);
            PatternRoute patternRoute(net, gridGraph, parameters);
            omp_set_lock(&lock);
            patternRoute.constructSteinerTree();
            omp_unset_lock(&lock);
            patternRoute.constructRoutingDAG();
            patternRoute.constructDetours(congestionView);
            patternRoute.run();
            gridGraph.commitTree(net.getRoutingTree());
        }
    }
    omp_destroy_lock(&lock);

    for (int j : nonoverlapNetIndices[threadNum]) {
        GRNet& net = nets[j];
        gridGraph.commitTree(net.getRoutingTree(), true);
        PatternRoute patternRoute(net, gridGraph, parameters);
        patternRoute.constructSteinerTree();
        patternRoute.constructRoutingDAG();
        patternRoute.constructDetours(congestionView);
        patternRoute.run();
        gridGraph.commitTree(net.getRoutingTree());
    }
}

void GlobalRouter::stageMazeRouting(std::vector<int>& netIndices) {
    std::cout << "[INFO] Stage 3: Maze Routing" << std::endl;
    GridGraphView<CostT> wireCostView;
    gridGraph.extractWireCostView(wireCostView);

    sortNetIndices(netIndices);
    SparseGrid grid(1, 1, 0, 0);

    for (int netIndex : netIndices) {
        MazeRoute mazeRoute(nets[netIndex], gridGraph, parameters);
        mazeRoute.constructSparsifiedGraph(wireCostView, grid);
        mazeRoute.run();
        gridGraph.commitTree(nets[netIndex].getRoutingTree());
        gridGraph.updateWireCostView(wireCostView, nets[netIndex].getRoutingTree());
        grid.step();
    }
}

void GlobalRouter::write() {
    std::cout << "[INFO] Generating route guides..." << std::endl;
    const std::string guide_file = parameters.out_file;

    auto start = std::chrono::high_resolution_clock::now();
    int netSize = nets.size();
    // #pragma omp parallel for
    for (int i = 0; i < netSize; i++) {
        nets[i].getGuides();
    }
    std::cout << "[INFO] Elapsed time for getGuides: " << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count() << " s" << endl;

    FILE* out;
    out = fopen(guide_file.c_str(), "w");
    for (const GRNet& net : nets) {
        fprintf(out, "%s", net.guide_string.c_str());
    }
    fclose(out);

    std::cout << "[INFO] Finished writing output..." << std::endl;
}

// Helper functions
void GlobalRouter::separateNetIndices(std::vector<int>& netIndices, std::vector<std::vector<int>>& nonoverlapNetIndices) const {
    const int numGroups = nonoverlapNetIndices.size() - 1; // Last group is reserved for the rest
    if (numGroups <= 0) {
        std::cerr << "[ERROR] Number of groups must be greater than 0." << std::endl;
        return;
    }

    const int netSize = netIndices.size();
    std::vector<int> netCenterX(netSize);
    std::vector<int> divideX(numGroups - 1);

    for (int i = 0; i < netSize; ++i) {
        const GRNet& net = nets[netIndices[i]];
        netCenterX[i] = net.getBoundingBox().cx();
    }

    std::sort(netCenterX.begin(), netCenterX.end());

    for (int i = 0; i < numGroups - 1; ++i) {
        divideX[i] = netCenterX[(i + 1) * netSize / numGroups];
    }

    for (int i = 0; i < netSize; ++i) {
        const GRNet& net = nets[netIndices[i]];
        const auto& boundingBox = net.getBoundingBox();
        const int xlow = boundingBox.x.low, xhigh = boundingBox.x.high;

        bool assigned = false;
        for (int j = 0; j < numGroups; ++j) {
            if ((j == 0 && xhigh < divideX[j]) ||
                (j == numGroups - 1 && xlow > divideX[j - 1]) ||
                (j > 0 && xhigh < divideX[j] && xlow > divideX[j - 1])) {
                nonoverlapNetIndices[j].push_back(netIndices[i]);
                assigned = true;
                break;
            }
        }

        if (!assigned) {
            nonoverlapNetIndices[numGroups].push_back(netIndices[i]); // Add to the "rest" group
        }
    }
}

void GlobalRouter::sortNetIndices(vector<int>& netIndices) const {  // sort by half perimeter: 短的先繞
    vector<int> halfParameters(nets.size());
    // vector<int> maxEdgeLength(nets.size()); // added by Alan
    for (int netIndex : netIndices) {
        auto& net = nets[netIndex];
        halfParameters[netIndex] = net.getBoundingBox().hp();
        // maxEdgeLength[netIndex] = max(net.getBoundingBox().x.range(), net.getBoundingBox().y.range()); // added by Alan
    }
    sort(netIndices.begin(), netIndices.end(), [&](int lhs, int rhs) {
        return halfParameters[lhs] < halfParameters[rhs];
        // return maxEdgeLength[lhs] < maxEdgeLength[rhs]; // added by Alan
    });
}

// Analysis
void GlobalRouter::printStatistics() const {
    std::cout << "Routing Statistics" << std::endl;

    uint64_t wireLength = 0;
    int viaCount = 0;
    std::vector<std::vector<std::vector<int>>> wireUsage(
        gridGraph.getNumLayers(),
        std::vector<std::vector<int>>(gridGraph.getSize(0), std::vector<int>(gridGraph.getSize(1), 0))
    );

    for (const auto& net : nets) {
        GRTreeNode::preorder(net.getRoutingTree(), [&](std::shared_ptr<GRTreeNode> node) {
            for (const auto& child : node->children) {
                if (node->layerIdx == child->layerIdx) {
                    unsigned direction = gridGraph.getLayerDirection(node->layerIdx);
                    int l = std::min((*node)[direction], (*child)[direction]);
                    int h = std::max((*node)[direction], (*child)[direction]);
                    int r = (*node)[1 - direction];
                    for (int c = l; c < h; ++c) {
                        wireLength += gridGraph.getEdgeLength(direction, c);
                        int x = direction == 0 ? c : r;
                        int y = direction == 0 ? r : c;
                        wireUsage[node->layerIdx][x][y] += 1;
                    }
                } else {
                    viaCount += std::abs(node->layerIdx - child->layerIdx);
                }
            }
        });
    }

    CapacityT overflow = 0;
    CapacityT minResource = std::numeric_limits<CapacityT>::max();
    GRPoint bottleneck(-1, -1, -1);

    for (int layerIndex = parameters.min_routing_layer; layerIndex < gridGraph.getNumLayers(); ++layerIndex) {
        unsigned direction = gridGraph.getLayerDirection(layerIndex);
        for (int x = 0; x < gridGraph.getSize(0) - 1 + direction; ++x) {
            for (int y = 0; y < gridGraph.getSize(1) - direction; ++y) {
                CapacityT resource = gridGraph.getEdge(layerIndex, x, y).getResource();
                if (resource < minResource) {
                    minResource = resource;
                    bottleneck = {layerIndex, x, y};
                }
                CapacityT usage = wireUsage[layerIndex][x][y];
                CapacityT capacity = std::max(gridGraph.getEdge(layerIndex, x, y).capacity, 0.0);
                if (usage > capacity) {
                    overflow += usage - capacity;
                }
            }
        }
    }

    std::cout << "======================" << std::endl;
    std::cout << "Wire Length (Metric):  " << wireLength << std::endl;
    std::cout << "Total Via Count:       " << viaCount << std::endl;
    std::cout << "Total Wire Overflow:   " << static_cast<int>(overflow) << std::endl;
    std::cout << "Min Resource:          " << minResource << std::endl;
    std::cout << "Bottleneck:            " << bottleneck << std::endl;
    std::cout << "======================" << std::endl;
}

void GlobalRouter::writeExtractNetToFile(const std::vector<std::pair<Point, Point>>& extract_net, const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "[ERROR] Unable to open file: " << filename << std::endl;
        return;
    }
    for (const auto& pair : extract_net) {
        outFile << pair.first.x << " " << pair.first.y << " "
                << pair.second.x << " " << pair.second.y << "\n";
    }
    outFile.close();
    std::cout << "[INFO] Successfully wrote extracted net data to: " << filename << std::endl;
}

void GlobalRouter::write_partial_cap(const std::vector<std::vector<std::vector<double>>>& cap) const {
    std::stringstream ss;
    ss << gridGraph.nLayers << " " << gridGraph.ySize / 2 << " " << gridGraph.xSize / 2 << "\n";
    for (int layerIndex = 0; layerIndex < gridGraph.nLayers; ++layerIndex) {
        ss << layerIndex << "\n";
        for (int x = 0; x < gridGraph.xSize / 2; ++x) {
            for (int y = 0; y < gridGraph.ySize / 2; ++y) {
                ss << cap[layerIndex][x][y];
                if (y < gridGraph.ySize / 2 - 1) ss << " ";
            }
            ss << "\n";
        }
    }

    std::ofstream fout(cap_file_name);
    if (!fout.is_open()) {
        std::cerr << "[ERROR] Unable to open file: " << cap_file_name << std::endl;
        return;
    }
    fout << ss.str();
    fout.close();
    std::cout << "[INFO] Successfully wrote partial capacity data to: " << cap_file_name << std::endl;
}
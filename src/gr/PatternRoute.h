#pragma once
#include "../flute/flute.h"
#include "../global.h"
#include "GRNet.h"

extern "C" {
void readLUT();
Tree flute(int d, DTYPE x[], DTYPE y[], int acc);
}

class SteinerTreeNode : public utils::PointT<int> {
   public:
    vector<std::shared_ptr<SteinerTreeNode>> children;
    utils::IntervalT<int> fixedLayers;
    int z;  // Added by Alan: layer index

    SteinerTreeNode(utils::PointT<int> point)
        : utils::PointT<int>(point) {}
    SteinerTreeNode(utils::PointT<int> point, utils::IntervalT<int> _fixedLayers)
        : utils::PointT<int>(point), fixedLayers(_fixedLayers) {}
    SteinerTreeNode(utils::PointT<int> point, int _z) // Added by Alan
        : utils::PointT<int>(point), z(_z) {}
    SteinerTreeNode(utils::PointT<int> point, utils::IntervalT<int> _fixedLayers, int _z) // Added by Alan
        : utils::PointT<int>(point), fixedLayers(_fixedLayers), z(_z) {}

    static void preorder(std::shared_ptr<SteinerTreeNode> node, std::function<void(std::shared_ptr<SteinerTreeNode>)> visit);
    // static std::string getPythonString(std::shared_ptr<SteinerTreeNode> node);
};

class PatternRoutingNode : public utils::PointT<int> {
   public:
    const int index;

    vector<std::shared_ptr<PatternRoutingNode>> children;
    vector<vector<std::shared_ptr<PatternRoutingNode>>> paths;  // childIndex -> pathIndex -> path
    utils::IntervalT<int> fixedLayers;                          // layers that must be visited in order to connect all the pins
    vector<CostT> costs;                                        // layerIndex -> cost
    vector<vector<std::pair<int, int>>> bestPaths;              // best path for each child; layerIndex -> childIndex -> (pathIndex, layerIndex)
    bool optional;  // Alan: if this node is optional, it can be skipped if it is not necessary to connect all the pins

    PatternRoutingNode(utils::PointT<int> point, int _index, bool _optional = false)
        : utils::PointT<int>(point), index(_index), optional(_optional) {}
    PatternRoutingNode(utils::PointT<int> point, utils::IntervalT<int> _fixedLayers, int _index = 0)
        : utils::PointT<int>(point), fixedLayers(_fixedLayers), index(_index), optional(false) {}
    // static std::string getPythonString(std::shared_ptr<PatternRoutingNode> routingDag);
};

class PatternRoute {
   public:
    static void readFluteLUT() { readLUT(); };

    PatternRoute(GRNet& _net, const GridGraph& graph, const Parameters& param)
        : net(_net), gridGraph(graph), parameters(param), numDagNodes(0) {}
    void constructSteinerTree();
    void constructRoutingDAG();
    void constructDetours(GridGraphView<bool>& congestionView);
    void run();
    void setSteinerTree(std::shared_ptr<SteinerTreeNode> tree) { steinerTree = tree; }
    // added by Alan
    void getAllAccessPoints();
    void pruneRoutingTree(std::shared_ptr<GRTreeNode> &node);
    // for net extraction
    void extractNet(std::vector<std::pair<Point, Point> >& extracted_nets, int x_bound, int y_bound);

//    private:
    const Parameters& parameters;
    const GridGraph& gridGraph;
    GRNet& net;
    int numDagNodes;
    std::shared_ptr<SteinerTreeNode> steinerTree;
    std::shared_ptr<PatternRoutingNode> routingDag;
    // added by Alan
    vector<GRPoint> allAccessPoints;

    void constructPaths(std::shared_ptr<PatternRoutingNode>& start, std::shared_ptr<PatternRoutingNode>& end, int childIndex = -1);
    void calculateRoutingCosts(std::shared_ptr<PatternRoutingNode>& node);
    std::shared_ptr<GRTreeNode> getRoutingTree(std::shared_ptr<PatternRoutingNode>& node, int parentLayerIndex = -1);
};

#pragma once
#include "../global.h"
#include "../basic/design.h"
#include "GRTree.h"

class GRNet;
template <typename Type> class GridGraphView;

struct GraphEdge {
    GraphEdge(): capacity(0), demand(0) {}

    CapacityT capacity;
    CapacityT demand;
    CapacityT getResource() const { return capacity - demand; }
};

class GridGraph {
public:
    GridGraph(const Design& design, const Parameters& params);
    
    inline unsigned getNumLayers() const { return nLayers; }
    inline unsigned getSize(unsigned dimension) const { return (dimension ? ySize : xSize); }
    inline unsigned getLayerDirection(int layerIndex) const { return layerDirections[layerIndex]; }

    inline uint64_t hashCell(const GRPoint& point) const { // hash a point to a unique number
        return ((uint64_t)point.layerIdx * xSize + point.x) * ySize + point.y;
    };
    inline uint64_t hashCell(const int x, const int y) const { return (uint64_t)x * ySize + y; }
    // inline DBU getGridline(const unsigned dimension, const int index) const { return gridlines[dimension][index]; }
    // utils::BoxT<DBU> getCellBox(utils::PointT<int> point) const;
    // utils::BoxT<int> rangeSearchCells(const utils::BoxT<DBU>& box) const;
    inline GraphEdge getEdge(const int layerIndex, const int x, const int y) const {return graphEdges[layerIndex][x][y]; }

    // Costs
    DBU getEdgeLength(unsigned direction, unsigned edgeIndex) const;
    CostT getWireCost(const int layerIndex, const utils::PointT<int> u, const utils::PointT<int> v) const;
    CostT getViaCost(const int layerIndex, const utils::PointT<int> loc) const;
    inline CostT getUnitViaCost() const { return UnitViaCost; }
    
    // Misc
    void selectAccessPoints(GRNet& net, robin_hood::unordered_map<uint64_t, std::pair<utils::PointT<int>, utils::IntervalT<int>>>& selectedAccessPoints) const;
    
    // Methods for updating demands 
    void commitTree(const std::shared_ptr<GRTreeNode>& tree, const bool reverse = false);
    
    // Checks
    inline bool checkOverflow(const int layerIndex, const int x, const int y) const { return getEdge(layerIndex, x, y).getResource() < 0.0; }
    bool checkOverflow_stage(const int layerIndex, const int x, const int y, const int overflowThreshold) const;
    int checkOverflow(const int layerIndex, const utils::PointT<int> u, const utils::PointT<int> v, int overflowThreshold) const; // Check wire overflow
    int checkOverflow(const std::shared_ptr<GRTreeNode>& tree, int overflowThreshold) const; // Check routing tree overflow (Only wires are checked)
    std::string getPythonString(const std::shared_ptr<GRTreeNode>& routingTree) const;
   
    // 2D maps
    void extractCongestionView(GridGraphView<bool>& view) const; // 2D overflow look-up table
    void updateCongestionView(GridGraphView<bool>& view, const std::shared_ptr<GRTreeNode> routingTree) const;
    void extractWireCostView(GridGraphView<CostT>& view) const;
    void updateWireCostView(GridGraphView<CostT>& view, std::shared_ptr<GRTreeNode> routingTree) const;

    // For visualization
    void write(const std::string heatmap_file="heatmap.txt") const;
    void writeCapacity(const std::string heatmap_file="capacity.txt") const;
    
// private:
    const Parameters& parameters;

    unsigned nLayers;
    unsigned xSize;
    unsigned ySize;
    // vector<vector<DBU>> gridlines;
    vector<vector<DBU>> gridCenters;
    vector<unsigned> layerDirections;
    vector<DBU> layerMinLengths;

    // Unit costs
    CostT UnitLengthWireCost;
    CostT UnitViaCost;
    vector<CostT> OFWeight; // overflow weights

    DBU totalLength = 0;
    int totalNumVias = 0;
    vector<vector<vector<GraphEdge>>> graphEdges; // gridEdges[l][x][y] stores the edge {(l, x, y), (l, x+1, y)} or {(l, x, y), (l, x, y+1)}, depending on the routing direction of the layer

    // utils::IntervalT<int> rangeSearchGridlines(const unsigned dimension, const utils::IntervalT<DBU>& locInterval) const; // Find the gridlines within [locInterval.low, locInterval.high]
    // utils::IntervalT<int> rangeSearchRows(const unsigned dimension, const utils::IntervalT<DBU>& locInterval) const; // Find the rows/columns overlapping with [locInterval.low, locInterval.high]
    
    // Add by Alan
    // int searchXGridline(const int x) const;
    // int searchYGridline(const int y) const;

    inline CostT getUnitLengthWireCost() const { return UnitLengthWireCost; }

    inline double logistic(const CapacityT& input, bool s) const;
    CostT getWireCost(const int layerIndex, const utils::PointT<int> lower, const CapacityT demand = 1.0) const;

    // Methods for updating demands 
    void commit(const int layerIndex, const utils::PointT<int> lower, const CapacityT demand);
    void commitWire(const int layerIndex, const utils::PointT<int> lower, const bool reverse = false);
    void commitVia(const int layerIndex, const utils::PointT<int> loc, const bool reverse = false, bool isStackedVia = false);

    // for getEdgeLength()
    vector<int> hEdge;
    vector<int> vEdge;
};


template <typename Type>
class GridGraphView: public vector<vector<vector<Type>>> {
public:
    bool check(const utils::PointT<int>& u, const utils::PointT<int>& v) const {
        assert(u.x == v.x || u.y == v.y);
        if (u.y == v.y) {
            int l = min(u.x, v.x), h = max(u.x, v.x);
            for (int x = l; x < h; x++) {
                if ((*this)[0][x][u.y]) return true;
            }
        } else {
            int l = min(u.y, v.y), h = max(u.y, v.y);
            for (int y = l; y < h; y++) {
                if ((*this)[1][u.x][y]) return true;
            }
        }
        return false;
    }
    
    Type sum(const utils::PointT<int>& u, const utils::PointT<int>& v) const {
        assert(u.x == v.x || u.y == v.y);
        Type res = 0;
        if (u.y == v.y) {
            int l = min(u.x, v.x), h = max(u.x, v.x);
            for (int x = l; x < h; x++) {
                res += (*this)[0][x][u.y];
            }
        } else {
            int l = min(u.y, v.y), h = max(u.y, v.y);
            for (int y = l; y < h; y++) {
                res += (*this)[1][u.x][y];
            }
        }
        return res;
    }
};
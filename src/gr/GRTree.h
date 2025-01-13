#pragma once
#include "../global.h"

class GRPoint: public utils::PointT<int> {
public:
    int layerIdx;
    GRPoint(): layerIdx(0), utils::PointT<int>(0, 0) {}
    GRPoint(int l, int _x, int _y): layerIdx(l), utils::PointT<int>(_x, _y) {}
    // friend inline std::ostream& operator<<(std::ostream& os, const GRPoint& pt) {
    //     os << "(" << pt.layerIdx << ", " << pt.x << ", " << pt.y << ")";
    //     return os;
    // }
    
};

class GRTreeNode: public GRPoint {
public:
    vector<std::shared_ptr<GRTreeNode>> children;
    
    GRTreeNode(int l, int _x, int _y): GRPoint(l, _x, _y) {}
    GRTreeNode(const GRPoint& point): GRPoint(point) {}
    static void preorder(std::shared_ptr<GRTreeNode> node, std::function<void(std::shared_ptr<GRTreeNode>)> visit);
    // static void print(std::shared_ptr<GRTreeNode> node);
};
#include "GRNet.h"

GRNet::GRNet(const Net& baseNet, const Design& design, const GridGraph& gridGraph)
    : index(baseNet.id), name(baseNet.name) {
    pinAccessPoints.resize(baseNet.pin_ids.size());

    // construct pinAccessPoints
    for (int i = 0; i < baseNet.pin_ids.size(); i++) {  // i is pinIndex
        const Pin& pin = design.netlist.pins[baseNet.pin_ids[i]];
        pinAccessPoints[i].resize(pin.point_ids.size());
        for (int j = 0; j < pin.point_ids.size(); j++) {  // j is accessPointIndex
            const Point& point = design.netlist.points[pin.point_ids[j]];

            pinAccessPoints[i][j] = GRPoint(point.layer, point.x, point.y);
            // int x = gridGraph.searchXGridline(point.x);
            // int y = gridGraph.searchYGridline(point.y);
            // pinAccessPoints[i][j] = GRPoint(point.layer, x, y);
        }
    }

    // construct boundingBox
    boundingBox = utils::BoxT<int>(INT_MAX, INT_MAX, INT_MIN, INT_MIN);
    for (const auto& pinPoints : pinAccessPoints) {
        for (const auto& point : pinPoints) {
            boundingBox.lx() = std::min(boundingBox.lx(), point.x);
            boundingBox.hx() = std::max(boundingBox.hx(), point.x);
            boundingBox.ly() = std::min(boundingBox.ly(), point.y);
            boundingBox.hy() = std::max(boundingBox.hy(), point.y);
        }
    }
}

void GRNet::getGuides() {
    if (!routingTree)
        return;

    vector<vector<int>> guide;

    // function addGuideSegment, input is node, child, guide, add the guide segment to guide
    auto addGuideSegment = [&](vector<vector<int>>& guide, std::shared_ptr<GRTreeNode> node, std::shared_ptr<GRTreeNode> child) {
        vector<int> vec;
        vec.reserve(6);
        vec.emplace_back(min(node->x, child->x));
        vec.emplace_back(min(node->y, child->y));
        vec.emplace_back(min(node->layerIdx, child->layerIdx));
        vec.emplace_back(max(node->x, child->x));
        vec.emplace_back(max(node->y, child->y));
        vec.emplace_back(max(node->layerIdx, child->layerIdx));
        guide.push_back(vec);
    };

    GRTreeNode::preorder(routingTree, [&](std::shared_ptr<GRTreeNode> node) {
        for (const auto& child : node->children) {
            // assign zl, zh for node and child, zl is the lower layer, zh is the higher layer
            int zl = min(node->layerIdx, child->layerIdx);
            int zh = max(node->layerIdx, child->layerIdx);

            if (node->x == child->x && node->y == child->y && zl == zh) {
                // cout << "Warning: node and child are the same" << endl;
                continue;
            }
            if (node->x == child->x && node->y == child->y && zh != zl + 1) {
                // split the segment into multiple segments
                for (int z = zl; z < zh; z++) {
                    // create nodes for the split segments from z to z+1
                    std::shared_ptr<GRTreeNode> node1 = std::make_shared<GRTreeNode>(*node);
                    node1->layerIdx = z;
                    std::shared_ptr<GRTreeNode> node2 = std::make_shared<GRTreeNode>(*node);
                    node2->layerIdx = z + 1;
                    // add the guide segment
                    addGuideSegment(guide, node1, node2);
                }
            } else {
                addGuideSegment(guide, node, child);
            }
        }
    });
    // check if the guide has same elements
    unordered_set<string> guideSet;
    guide_string = name + "\n(\n";
    for (const auto& vec : guide) {
        string temp;

        // the format is (xl, yl, zl, xh, yh, zh), add "metal" to the layer(zl, zh)
        for (int i = 0; i < vec.size(); i++) {
            if (i % 3 == 2) {
                temp += "metal" + to_string(vec[i] + 1) + " ";
            } else {
                temp += to_string(vec[i] * 4200 + 2100) + " ";
            }
        }

        if (guideSet.find(temp) == guideSet.end()) {
            guideSet.insert(temp);
            guide_string += temp + "\n";
        }
    }
    guide_string += ")\n";
}

#include "../basic/design.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include "../basic/netlist.h"

bool Design::readCap(const std::string& filename) {
    FILE* inputFile = fopen(filename.c_str(), "r");
    if (!inputFile) {
        std::cerr << "[ERROR] Failed to open CAP file: " << filename << std::endl;
        return false;
    }

    if (fscanf(inputFile, "%d %d %d", &dimension.n_layers, &dimension.x_size, &dimension.y_size) != 3) {
        std::cerr << "[ERROR] Failed to read dimension information from CAP file." << std::endl;
        fclose(inputFile);
        return false;
    }

    if (fscanf(inputFile, "%lf %lf", &metrics.UnitLengthWireCost, &metrics.UnitViaCost) != 2) {
        std::cerr << "[ERROR] Failed to read wire and via costs." << std::endl;
        fclose(inputFile);
        return false;
    }
    parameters.UnitViaCost = metrics.UnitViaCost;

    metrics.OFWeight.resize(dimension.n_layers);
    for (int i = 0; i < dimension.n_layers; ++i) {
        if (fscanf(inputFile, "%lf", &metrics.OFWeight[i]) != 1) {
            std::cerr << "[ERROR] Failed to read overflow weights." << std::endl;
            fclose(inputFile);
            return false;
        }
    }

    dimension.hEdge.resize(dimension.x_size - 1);
    for (int& hEdge : dimension.hEdge) {
        if (fscanf(inputFile, "%d", &hEdge) != 1) {
            std::cerr << "[ERROR] Failed to read horizontal edges." << std::endl;
            fclose(inputFile);
            return false;
        }
    }

    dimension.vEdge.resize(dimension.y_size - 1);
    for (int& vEdge : dimension.vEdge) {
        if (fscanf(inputFile, "%d", &vEdge) != 1) {
            std::cerr << "[ERROR] Failed to read vertical edges." << std::endl;
            fclose(inputFile);
            return false;
        }
    }

    char name[1000];
    for (int i = 0; i < dimension.n_layers; ++i) {
        Layer layer;
        layer.id = i;

        if (fscanf(inputFile, "%s %d %lf", name, &layer.direction, &layer.minLength) != 3) {
            std::cerr << "[ERROR] Failed to read layer information." << std::endl;
            fclose(inputFile);
            return false;
        }

        layer.capacity.resize(dimension.y_size, std::vector<double>(dimension.x_size, 0.0));
        for (int y = 0; y < dimension.y_size; ++y) {
            for (int x = 0; x < dimension.x_size; ++x) {
                if (fscanf(inputFile, "%lf", &layer.capacity[y][x]) != 1) {
                    std::cerr << "[ERROR] Failed to read layer capacity." << std::endl;
                    fclose(inputFile);
                    return false;
                }
            }
        }
        layers.push_back(std::move(layer));
    }

    fclose(inputFile);

    // print out the read information
    std::cout << "=====================================" << std::endl;
    std::cout << "[INFO] Number of layers: " << dimension.n_layers << std::endl;
    std::cout << "[INFO] Grid dimension: " << dimension.x_size << " x " << dimension.y_size << std::endl;
    std::cout << "[INFO] Unit Length Wire Cost: " << metrics.UnitLengthWireCost << std::endl;
    std::cout << "[INFO] Unit Via Cost: " << metrics.UnitViaCost << std::endl;
    std::cout << "[INFO] Overflow Weights: ";
    for (int i = 0; i < dimension.n_layers; ++i) {
        std::cout << metrics.OFWeight[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "=====================================" << std::endl;

    return true;
}

bool Design::readNet(const std::string& filename) {
    FILE* inputFile = fopen(filename.c_str(), "r");
    if (!inputFile) {
        std::cerr << "[ERROR] Failed to open NET file: " << filename << std::endl;
        return false;
    }

    std::vector<Net> nets;
    std::vector<Pin> pins;
    std::vector<Point> points;

    char line[1000];
    int net_id = 0, pin_id = 0, point_id = 0;

    while (fgets(line, sizeof(line), inputFile)) {
        if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';

        std::string netName(line);
        Net net(net_id++, netName);
        std::vector<int> pin_ids;

        if (!fgets(line, sizeof(line), inputFile)) break; // Skip "(\n"
        while (fgets(line, sizeof(line), inputFile)) {
            std::string pinLine(line);
            pinLine.erase(std::remove(pinLine.begin(), pinLine.end(), ' '), pinLine.end());

            if (pinLine == ")\n" || pinLine == ")") break;

            Pin pin(pin_id++, net.id);
            std::vector<int> point_ids;

            char* token = strtok(line, ", \t\n");
            pin.name = token ? token : "";

            token = strtok(nullptr, ", \t\n");
            pin.slack = token ? std::atof(token) : 0.0;

            token = strtok(nullptr, ", \t\n");
            replaceChars(token);

            while (token) {
                int layer = std::atoi(token);
                token = strtok(nullptr, " \t\n");
                if (!token) break;
                int x = std::atoi(token);
                token = strtok(nullptr, " \t\n");
                if (!token) break;
                int y = std::atoi(token);
                token = strtok(nullptr, " \t\n");

                points.emplace_back(point_id++, net.id, layer, x, y);
                point_ids.push_back(point_id - 1);
            }

            pin.point_ids = std::move(point_ids);
            pins.push_back(std::move(pin));
            pin_ids.push_back(pins.back().id);
        }

        net.pin_ids = std::move(pin_ids);
        nets.push_back(std::move(net));
    }

    netlist.n_nets = nets.size();
    netlist.n_pins = pins.size();
    netlist.n_points = points.size();
    netlist.nets = std::move(nets);
    netlist.pins = std::move(pins);
    netlist.points = std::move(points);

    std::cout << "=====================================" << std::endl;
    std::cout << "[INFO] Number of nets: " << netlist.n_nets << std::endl;
    std::cout << "[INFO] Number of pins: " << netlist.n_pins << std::endl;
    std::cout << "[INFO] Number of points: " << netlist.n_points << std::endl;
    std::cout << "=====================================" << std::endl;

    fclose(inputFile);
    return true;
}

void Design::replaceChars(char* str) {
    while (str && *str) {
        if (*str == '[' || *str == ']' || *str == '(' || *str == ')') {
            *str = ' ';
        }
        ++str;
    }
}
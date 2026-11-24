#pragma once
#include "point.h"
#include <string>
#include <vector>

struct TargetData {
    int id{};
    std::string type;
    Point2D position;
    double altitude{};
    double speedKmh{};
    double headingDeg{};
};

std::vector<TargetData> loadTargets(const std::string &path);

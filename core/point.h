#pragma once

struct Point2D {
    double x{};
    double y{};
};

double distance(const Point2D &a, const Point2D &b);
double bearing(const Point2D &a, const Point2D &b);
#pragma once

constexpr double kPi {3.14159265358979323846};

struct Point2D {
    double x{};
    double y{};
};

double distance(const Point2D &a, const Point2D &b);
double bearing(const Point2D &a, const Point2D &b);

double degreesToRadians(double degrees);
double radiansToDegrees(double radians);
double normalizeHeading(double headingDeg);
Point2D move(const Point2D &from, double speedKmh, double headingDeg, double dtHours);
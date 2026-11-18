#include "point.h"
#include <cmath>

constexpr double kPi {3.14159265358979323846};

double distance(const Point2D &a, const Point2D &b) {
    const double dx {b.x - a.x};
    const double dy {b.y - a.y};
    return std::sqrt(dx * dx + dy * dy);
}

double bearing(const Point2D &a, const Point2D &b) {
    const double dx {b.x - a.x};
    const double dy {b.y - a.y};
    double azimuth {std::atan2(dx, dy)};
    azimuth = azimuth * 180 / kPi;

    if (azimuth < 0) {
        azimuth += 360;
    }

    return azimuth;
}
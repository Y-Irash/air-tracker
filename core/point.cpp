#include "point.h"
#include <cmath>

double distance(const Point2D &a, const Point2D &b) {
    const double dx {b.x - a.x};
    const double dy {b.y - a.y};
    return std::sqrt(dx * dx + dy * dy);
}

double bearing(const Point2D &a, const Point2D &b) {
    const double dx {b.x - a.x};
    const double dy {b.y - a.y};
    return normalizeHeading(radiansToDegrees(std::atan2(dx, dy)));
}

double degreesToRadians(double degrees) {
    return degrees * kPi / 180;
}

double radiansToDegrees(double radians) {
    return radians * 180 / kPi;
}

double normalizeHeading(double headingDeg) {
    headingDeg = std::fmod(headingDeg, 360);

    if (headingDeg < 0) {
        headingDeg += 360;
    } else if (headingDeg >= 360) {
        headingDeg -= 360;
    }

    return headingDeg;
}

Point2D move(const Point2D &from, double speedKmh, double headingDeg, double dtHours) {
    const double s = speedKmh * dtHours;
    const double headingNorm = normalizeHeading(headingDeg);
    const double headingRad = degreesToRadians(headingNorm);
    const Point2D to {from.x + s * std::sin(headingRad), from.y + s * std::cos(headingRad)};
    return to;
}


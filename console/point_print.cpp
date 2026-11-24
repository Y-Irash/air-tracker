#include "point_print.h"
#include <ostream>

std::ostream &operator<<(std::ostream &out, const Point2D &p) {
    out << '(' << p.x << ", " << p.y << ')';
    return out;
}

std::ostream &operator<<(std::ostream &out, const Target &target) {
    out << "id " << target.id()
        << "  " << target.type()
        << "  позиция " << target.position()
        << "  H " << target.altitudeKm() << " км"
        << "  V " << target.speedKmh() << " км/ч"
        << "  курс " << target.headingDeg() << "°";
    return out;
}

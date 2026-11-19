#include "point_print.h"
#include <ostream>

std::ostream &operator<<(std::ostream &out, const Point2D &p) {
    out << '(' << p.x << ", " << p.y << ')';
    return out;
}

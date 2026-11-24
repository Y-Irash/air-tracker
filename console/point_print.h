#pragma once
#include <iosfwd>

#include "core/point.h"
#include "core/target.h"

// Слой представления консоли: как объекты ядра выглядят при печати.
std::ostream &operator<<(std::ostream &out, const Point2D &p);
std::ostream &operator<<(std::ostream &out, const Target &target);

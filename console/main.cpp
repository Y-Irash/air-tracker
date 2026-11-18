#include <iostream>
#include "../core/point.h"
#include "console_utf8.h"

int main()
{
    enableUtf8Console();

    Point2D radar{0, 0};

    Point2D north_point {0, 10};
    Point2D south_point {0, -10};
    Point2D west_point {-10, 0};
    Point2D east_point {10, 0};

    std::cout << "Расстояние до северной точки: " << distance(radar, north_point) << "\n";
    std::cout << "Расстояние до южной точки: " << distance(radar, south_point) << "\n";
    std::cout << "Расстояние до западной точки: " << distance(radar, west_point) << "\n";
    std::cout << "Расстояние до восточной точки: " << distance(radar, east_point) << "\n";

    std::cout << "Азимут до северной точки: " << bearing(radar, north_point) << "\n";
    std::cout << "Азимут до южной точки: " << bearing(radar, south_point) << "\n";
    std::cout << "Азимут до западной точки: " << bearing(radar, west_point) << "\n";
    std::cout << "Азимут до восточной точки: " << bearing(radar, east_point) << "\n";

    return 0;
}

#include <iostream>
#include "../core/point.h"
#include "console_utf8.h"
#include "point_print.h"

int main()
{
    enableUtf8Console();

    const Point2D radar {0, 0};

    const Point2D northPoint {0, 10};
    const Point2D southPoint {0, -10};
    const Point2D westPoint {-10, 0};
    const Point2D eastPoint {10, 0};

    std::cout << "Расстояние до северной точки: " << distance(radar, northPoint) << "\n";
    std::cout << "Расстояние до южной точки: " << distance(radar, southPoint) << "\n";
    std::cout << "Расстояние до западной точки: " << distance(radar, westPoint) << "\n";
    std::cout << "Расстояние до восточной точки: " << distance(radar, eastPoint) << "\n";

    std::cout << "Азимут до северной точки: " << bearing(radar, northPoint) << "\n";
    std::cout << "Азимут до южной точки: " << bearing(radar, southPoint) << "\n";
    std::cout << "Азимут до западной точки: " << bearing(radar, westPoint) << "\n";
    std::cout << "Азимут до восточной точки: " << bearing(radar, eastPoint) << "\n";

    // Движение: скорость 600 км/ч, разные курсы и интервалы времени.
    // В каждой строке печатаются входные данные вместе с результатом,
    // иначе по выводу не понять, какой строке какой курс соответствует.
    const Point2D start {0, 0};

    std::cout << "Движение из (0, 0) со скоростью 600 км/ч:\n";
    std::cout << "  курс 90,  1 ч   : " << move(start, 600, 90, 1.0) << "\n";
    std::cout << "  курс 0,   1 ч   : " << move(start, 600, 0, 1.0) << "\n";
    std::cout << "  курс 180, 0.5 ч : " << move(start, 600, 180, 0.5) << "\n";
    std::cout << "  курс 270, 2 ч   : " << move(start, 600, 270, 2.0) << "\n";

    // 370 и 10 должны дать одинаковый результат: sin/cos периодичны,
    // а move ещё и приводит курс к [0, 360) через normalizeHeading.
    std::cout << "  курс 370, 1 ч   : " << move(start, 600, 370, 1.0) << "\n";
    std::cout << "  курс 10,  1 ч   : " << move(start, 600, 10, 1.0) << "\n";

    std::cout << "Стартовая точка после всех вызовов: " << start << "\n";

    return 0;
}

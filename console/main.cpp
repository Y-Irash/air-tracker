#include <algorithm>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "../core/io.h"
#include "../core/point.h"
#include "console_utf8.h"
#include "point_print.h"

namespace {

void printTarget(const TargetData &target, const Point2D &radar)
{
    std::cout << "  id " << std::setw(2) << target.id
              << "  " << std::setw(10) << std::left << target.type << std::right
              << "  позиция " << target.position
              << "  H " << target.altitude << " км"
              << "  V " << target.speedKmh << " км/ч"
              << "  курс " << target.headingDeg << "°"
              << "  до радара " << distance(radar, target.position) << " км\n";
}

void demoGeometry(const Point2D &radar)
{
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
}

void demoMovement()
{
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
}

void demoCsvTargets(const std::string &path, const Point2D &radar)
{
    const std::vector<TargetData> targets = loadTargets(path);

    std::cout << "Цели из " << path << ": " << targets.size() << "\n";
    if (targets.empty()) {
        std::cout << "  (пусто: файла нет или в нём только заголовок)\n";
        return;
    }

    std::cout << "Все цели:\n";
    for (const TargetData &target : targets) {
        printTarget(target, radar);
    }

    std::vector<TargetData> byDistance = targets;
    std::sort(byDistance.begin(), byDistance.end(),
              [&radar](const TargetData &a, const TargetData &b) {
                  return distance(radar, a.position) < distance(radar, b.position);
              });

    std::cout << "По возрастанию расстояния до радара:\n";
    for (const TargetData &target : byDistance) {
        std::cout << "  " << std::setw(2) << target.id
                  << "  " << distance(radar, target.position) << " км\n";
    }

    std::vector<TargetData> nearTargets;
    std::copy_if(byDistance.begin(), byDistance.end(), std::back_inserter(nearTargets),
                 [&radar](const TargetData &target) {
                     return distance(radar, target.position) < 50.0;
                 });

    std::cout << "Ближе 50 км: " << nearTargets.size() << "\n";
    for (const TargetData &target : nearTargets) {
        printTarget(target, radar);
    }
}

} // namespace

int main(int argc, char *argv[])
{
    enableUtf8Console();

    const Point2D radar {0, 0};
    const std::string path = (argc > 1) ? argv[1] : "data/sample.csv";

    demoGeometry(radar);
    demoMovement();

    try {
        demoCsvTargets(path, radar);
    } catch (const std::exception &e) {
        std::cerr << "Ошибка чтения " << path << ": " << e.what() << "\n";
        return 1;
    }

    return 0;
}

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "core/conflict.h"
#include "core/io.h"
#include "core/point.h"
#include "core/target.h"
#include "core/tracker.h"
#include "console_utf8.h"
#include "point_print.h"

namespace {

const Point2D kRadar {0, 0};

bool nearlyEqualForDemo(const TargetData &a, const TargetData &b)
{
    const double eps = 1e-9;
    return a.id == b.id
        && a.type == b.type
        && std::fabs(a.position.x - b.position.x) < eps
        && std::fabs(a.position.y - b.position.y) < eps
        && std::fabs(a.altitude - b.altitude) < eps
        && std::fabs(a.speedKmh - b.speedKmh) < eps
        && std::fabs(a.headingDeg - b.headingDeg) < eps;
}

// --- задачи 01 и 02: геометрия и движение -----------------------------------

void demoGeometry()
{
    std::cout << "== Геометрия ==\n";
    const Point2D points[] = {{0, 10}, {10, 0}, {0, -10}, {-10, 0}};
    const char *names[] = {"север", "восток", "юг", "запад"};

    for (int i = 0; i < 4; ++i) {
        std::cout << "  " << std::setw(7) << std::left << names[i] << std::right
                  << "  расстояние " << distance(kRadar, points[i]) << " км"
                  << "  азимут " << bearing(kRadar, points[i]) << "°\n";
    }
}

void demoMovement()
{
    std::cout << "== Движение (600 км/ч) ==\n";
    const Point2D start {0, 0};
    std::cout << "  курс 90,  1 ч   : " << move(start, 600, 90, 1.0) << "\n";
    std::cout << "  курс 0,   1 ч   : " << move(start, 600, 0, 1.0) << "\n";
    std::cout << "  курс 180, 0.5 ч : " << move(start, 600, 180, 0.5) << "\n";
    std::cout << "  курс 270, 2 ч   : " << move(start, 600, 270, 2.0) << "\n";
    std::cout << "  курс 370 (=10)  : " << move(start, 600, 370, 1.0) << "\n";
    std::cout << "  стартовая точка не изменилась: " << start << "\n";
}

// --- задача 03 и 04: цели, сортировка, фильтр, прогон во времени ------------

void printTracker(const Tracker &tracker)
{
    for (const Target &target : tracker.targets()) {
        std::cout << "  " << target << "\n";
    }
}

std::vector<Target> sortedByDistance(const Tracker &tracker)
{
    std::vector<Target> sorted = tracker.targets();
    std::sort(sorted.begin(), sorted.end(), [](const Target &a, const Target &b) {
        return distance(kRadar, a.position()) < distance(kRadar, b.position());
    });
    return sorted;
}

std::vector<Target> closerThan(const std::vector<Target> &targets, double km)
{
    std::vector<Target> closeTargets;
    std::copy_if(targets.begin(), targets.end(), std::back_inserter(closeTargets),
                 [km](const Target &target) {
                     return distance(kRadar, target.position()) < km;
                 });
    return closeTargets;
}

void demoTargets(const std::string &path)
{
    std::cout << "== Цели из " << path << " ==\n";

    const std::vector<TargetData> data = loadTargets(path);
    Tracker tracker;
    for (const Target &target : toTargets(data)) {
        tracker.add(target);
    }

    std::cout << "загружено целей: " << tracker.size() << "\n";
    printTracker(tracker);

    std::cout << "-- по возрастанию расстояния до радара --\n";
    for (const Target &target : sortedByDistance(tracker)) {
        std::cout << "  id " << std::setw(2) << target.id()
                  << "  " << distance(kRadar, target.position()) << " км\n";
    }

    const std::vector<Target> closeTargets = closerThan(sortedByDistance(tracker), 50.0);
    std::cout << "-- ближе 50 км: " << closeTargets.size() << " --\n";
    for (const Target &target : closeTargets) {
        std::cout << "  " << target << "\n";
    }
}

void demoSimulation(const std::string &path, int steps, int printEvery)
{
    std::cout << "== Прогон " << steps << " шагов по 0.2 с ==\n";

    Tracker tracker;
    for (const Target &target : toTargets(loadTargets(path))) {
        tracker.add(target);
    }

    const double dtHours = 0.2 / 3600.0;

    for (int step = 1; step <= steps; ++step) {
        tracker.updateAll(dtHours);

        if (step % printEvery == 0 || step == steps) {
            std::cout << "  шаг " << std::setw(2) << step << ":\n";
            for (const Target &target : tracker.targets()) {
                std::cout << "    id " << std::setw(2) << target.id()
                          << "  " << target.position()
                          << "  азимут от радара " << bearing(kRadar, target.position()) << "°\n";
            }

            const std::vector<Conflict> conflicts = findConflicts(tracker, 10.0);
            if (!conflicts.empty()) {
                for (const Conflict &conflict : conflicts) {
                    std::cout << "    !! сближение: цели " << conflict.idA << " и " << conflict.idB
                              << " — " << conflict.distanceKm << " км\n";
                }
            }
        }
    }
}

// --- задача 05: сохранение и загрузка --------------------------------------

void demoRoundTrip(const std::string &path, const std::string &outPath)
{
    std::cout << "== Сохранение и повторная загрузка ==\n";

    const std::vector<TargetData> original = loadTargets(path);
    saveTargets(original, outPath);
    const std::vector<TargetData> restored = loadTargets(outPath);

    std::cout << "  было целей: " << original.size()
              << ", после сохранения и загрузки: " << restored.size() << "\n";
    if (original.size() == restored.size() && !original.empty()) {
        const bool sameFirst = nearlyEqualForDemo(original.front(), restored.front());
        std::cout << "  первая цель совпадает: " << (sameFirst ? "да" : "нет") << "\n";
    }
    std::cout << "  файл записан: " << outPath << "\n";
}

} // namespace

int main(int argc, char *argv[])
{
    enableUtf8Console();

    // путь можно передать аргументом; по умолчанию — данные рядом с exe
    const std::string dataPath = (argc > 1) ? argv[1] : std::string("data/sample.csv");

    try {
        demoGeometry();
        demoMovement();
        demoTargets(dataPath);
        demoSimulation(dataPath, 60, 10);
        demoRoundTrip(dataPath, "targets_out.csv");
    } catch (const std::exception &e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

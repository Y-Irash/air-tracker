// Тесты ядра: проверяют математику, классы и чтение файла. Без фреймворков.
// Запуск: core_tests.exe, код возврата 0 — всё хорошо, иначе номер провала.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <exception>
#include <string>
#include <vector>

#include "conflict.h"
#include "io.h"
#include "point.h"
#include "target.h"
#include "tracker.h"

namespace {

int g_fails = 0;

bool nearlyEqual(double a, double b, double eps = 1e-9)
{
    return std::fabs(a - b) < eps;
}

void check(const std::string &name, bool ok)
{
    std::printf("%-46s %s\n", name.c_str(), ok ? "OK" : "FAIL");
    if (!ok) {
        ++g_fails;
    }
}

void checkPoint(const std::string &name, Point2D got, Point2D want)
{
    check(name, nearlyEqual(got.x, want.x) && nearlyEqual(got.y, want.y));
}

void testGeometry()
{
    check("distance (0,0)-(3,4) = 5", nearlyEqual(distance({0, 0}, {3, 4}), 5.0));
    check("distance точки совпадают = 0", nearlyEqual(distance({7, 7}, {7, 7}), 0.0));

    check("bearing north = 0", nearlyEqual(bearing({0, 0}, {0, 10}), 0.0));
    check("bearing east = 90", nearlyEqual(bearing({0, 0}, {10, 0}), 90.0));
    check("bearing south = 180", nearlyEqual(bearing({0, 0}, {0, -10}), 180.0));
    check("bearing west = 270", nearlyEqual(bearing({0, 0}, {-10, 0}), 270.0));
    check("bearing north-east = 45", nearlyEqual(bearing({0, 0}, {10, 10}), 45.0));

    check("normalizeHeading(370) = 10", nearlyEqual(normalizeHeading(370), 10.0));
    check("normalizeHeading(-10) = 350", nearlyEqual(normalizeHeading(-10), 350.0));
    check("normalizeHeading(720) = 0", nearlyEqual(normalizeHeading(720), 0.0));
    check("normalizeHeading(360) = 0", nearlyEqual(normalizeHeading(360), 0.0));

    checkPoint("move 600 km/h, 90, 1 h", move({0, 0}, 600, 90, 1.0), {600, 0});
    checkPoint("move 600 km/h, 0, 1 h", move({0, 0}, 600, 0, 1.0), {0, 600});
    checkPoint("move 600 km/h, 180, 0.5 h", move({0, 0}, 600, 180, 0.5), {0, -300});
}

void testTarget()
{
    Target target(1, "aircraft", {0, 0}, 600, 90, 10);
    check("Target: id и тип", target.id() == 1 && target.type() == "aircraft");
    check("Target: курс нормализован", nearlyEqual(target.headingDeg(), 90.0));

    target.update(1.0);
    checkPoint("Target.update(1 ч) = (600, 0)", target.position(), {600, 0});

    target.update(0.5);
    checkPoint("Target.update(0.5 ч) = (900, 0)", target.position(), {900, 0});

    Target turning(2, "drone", {0, 0}, 100, 370, 0.5);
    check("Target: курс 370 приведён к 10", nearlyEqual(turning.headingDeg(), 10.0));

    Tracker tracker;
    tracker.add(Target(1, "aircraft", {10, 0}, 600, 90, 5));
    tracker.add(Target(2, "drone", {0, 20}, 120, 180, 1));
    tracker.add(Target(3, "helicopter", {-30, 0}, 250, 0, 2));
    check("Tracker.add: size = 3", tracker.size() == 3);

    check("Tracker.remove(2) = true", tracker.remove(2));
    check("Tracker.remove: size = 2", tracker.size() == 2);
    check("Tracker.remove(99) = false", !tracker.remove(99));

    check("Tracker.findById(999) = nullptr", tracker.findById(999) == nullptr);

    Target *found = tracker.findById(1);
    check("Tracker.findById(1) найден", found != nullptr && found->id() == 1);

    const Tracker &constRef = tracker;
    const Target *constFound = constRef.findById(1);
    check("Tracker.findById const: найден", constFound != nullptr && constFound->id() == 1);

    tracker.updateAll(1.0);
    const Target *moved = tracker.findById(1);
    checkPoint("Tracker.updateAll сдвинул цель 1", moved->position(), {610, 0});
}

void testFiles()
{
    const std::vector<TargetData> sample = loadTargets("data/sample.csv");
    check("loadTargets(sample.csv): 10 целей", sample.size() == 10);
    check("loadTargets: первая цель id = 1", !sample.empty() && sample.front().id == 1);
    check("loadTargets: курс 370 нормализован", !sample.empty() && nearlyEqual(sample[4].headingDeg, 10.0));

    const std::vector<Target> targets = toTargets(sample);
    check("toTargets: размер совпадает", targets.size() == sample.size());

    bool badFileThrows = false;
    try {
        loadTargets("data/bad.csv");
    } catch (const std::exception &e) {
        const std::string what = e.what();
        badFileThrows = what.find("line 4") != std::string::npos;
    }
    check("loadTargets(bad.csv): исключение про line 4", badFileThrows);

    bool missingThrows = false;
    try {
        loadTargets("data/нет-такого.csv");
    } catch (const std::exception &) {
        missingThrows = true;
    }
    check("loadTargets(нет файла): исключение", missingThrows);

    const std::string outPath = "tests_out.csv";
    saveTargets(sample, outPath);
    const std::vector<TargetData> restored = loadTargets(outPath);
    check("save/load: размеры совпадают", restored.size() == sample.size());
    check("save/load: первая цель совпадает",
          !restored.empty() && restored.front().id == sample.front().id
              && nearlyEqual(restored.front().position.x, sample.front().position.x)
              && nearlyEqual(restored.front().headingDeg, sample.front().headingDeg));
}

void testConflicts()
{
    Tracker tracker;
    for (const Target &target : toTargets(loadTargets("data/dense.csv"))) {
        tracker.add(target);
    }

    const std::vector<Conflict> conflicts = findConflicts(tracker, 10.0);
    check("findConflicts(dense.csv, 10): три пары", conflicts.size() == 3);

    bool has5 = false;
    bool has8 = false;
    bool has99 = false;
    bool has101 = false;
    for (const Conflict &conflict : conflicts) {
        const int a = std::min(conflict.idA, conflict.idB);
        const int b = std::max(conflict.idA, conflict.idB);
        if (a == 1 && b == 2) has5 = nearlyEqual(conflict.distanceKm, 5.0);
        if (a == 3 && b == 4) has8 = nearlyEqual(conflict.distanceKm, 8.0);
        if (a == 8 && b == 9) has99 = nearlyEqual(conflict.distanceKm, 9.9);
        if (a == 10 && b == 11) has101 = true;
    }
    check("findConflicts: пара (1,2) на 5.0 км", has5);
    check("findConflicts: пара (3,4) на 8.0 км", has8);
    check("findConflicts: пара (8,9) на 9.9 км", has99);
    check("findConflicts: пара (10,11) на 10.1 не попала", !has101);
}

} // namespace

int main()
{
    std::printf("=== tests: core ===\n");
    testGeometry();
    testTarget();
    testFiles();
    testConflicts();

    if (g_fails == 0) {
        std::printf("\nвсе проверки прошли\n");
        return 0;
    }

    std::printf("\nпровалов: %d\n", g_fails);
    return 1;
}

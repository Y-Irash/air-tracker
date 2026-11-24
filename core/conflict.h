#pragma once

#include <vector>

#include "tracker.h"

// Пара целей, сблизившихся ближе порога.
struct Conflict {
    int idA{};
    int idB{};
    double distanceKm{};
};

// Все пары целей ближе thresholdKm (порог строгий: 9.9 — да, 10.1 — нет).
// Каждая пара встречается один раз: (1,2) есть, (2,1) и (1,1) — нет.
std::vector<Conflict> findConflicts(const Tracker &tracker, double thresholdKm);

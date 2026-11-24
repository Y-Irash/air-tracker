#include "conflict.h"

#include <cstddef>

std::vector<Conflict> findConflicts(const Tracker &tracker, double thresholdKm)
{
    std::vector<Conflict> conflicts;
    const std::vector<Target> &targets = tracker.targets();

    for (std::size_t i = 0; i < targets.size(); ++i) {
        // внутренний цикл начинается с i + 1: пара считается один раз
        for (std::size_t j = i + 1; j < targets.size(); ++j) {
            const double d = distance(targets[i].position(), targets[j].position());
            if (d < thresholdKm) {
                conflicts.push_back(Conflict{targets[i].id(), targets[j].id(), d});
            }
        }
    }

    return conflicts;
}

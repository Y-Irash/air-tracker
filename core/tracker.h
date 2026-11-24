#pragma once

#include <cstddef>
#include <vector>

#include "target.h"

// Владелец списка целей: добавление, удаление, поиск, обновление всех.
class Tracker {
public:
    void add(const Target &target);

    // Возвращает true, если цель была и её удалили.
    bool remove(int id);

    // Найденная цель или nullptr, если такой нет.
    Target *findById(int id);
    const Target *findById(int id) const;

    // Сдвинуть все цели на интервал времени (часы).
    void updateAll(double dtHours);

    std::size_t size() const;

    // Только чтение: снаружи нельзя ни добавить цель в обход add(), ни удалить.
    const std::vector<Target> &targets() const;

private:
    std::vector<Target> m_targets;
};

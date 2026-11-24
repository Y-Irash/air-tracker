#include "tracker.h"

void Tracker::add(const Target &target)
{
    m_targets.push_back(target);
}

bool Tracker::remove(int id)
{
    for (auto it = m_targets.begin(); it != m_targets.end(); ++it) {
        if (it->id() == id) {
            m_targets.erase(it);
            return true;
        }
    }
    return false;
}

Target *Tracker::findById(int id)
{
    for (Target &target : m_targets) {
        if (target.id() == id) {
            return &target;
        }
    }
    return nullptr;
}

const Target *Tracker::findById(int id) const
{
    for (const Target &target : m_targets) {
        if (target.id() == id) {
            return &target;
        }
    }
    return nullptr;
}

void Tracker::updateAll(double dtHours)
{
    for (Target &target : m_targets) {
        target.update(dtHours);
    }
}

std::size_t Tracker::size() const
{
    return m_targets.size();
}

const std::vector<Target> &Tracker::targets() const
{
    return m_targets;
}

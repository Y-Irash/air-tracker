#include "target.h"

#include <utility>

Target::Target(int id, std::string type, Point2D position,
               double speedKmh, double headingDeg, double altitudeKm)
    : m_id(id)
    , m_type(std::move(type))
    , m_position(position)
    , m_speedKmh(speedKmh)
    , m_headingDeg(normalizeHeading(headingDeg))
    , m_altitudeKm(altitudeKm)
{
}

int Target::id() const { return m_id; }

const std::string &Target::type() const { return m_type; }

Point2D Target::position() const { return m_position; }

double Target::speedKmh() const { return m_speedKmh; }

double Target::headingDeg() const { return m_headingDeg; }

double Target::altitudeKm() const { return m_altitudeKm; }

void Target::update(double dtHours)
{
    m_position = move(m_position, m_speedKmh, m_headingDeg, dtHours);
    m_headingDeg = normalizeHeading(m_headingDeg);
}

std::string Target::toString() const
{
    // Форматирование простыми средствами: ядро не знает ни про cout, ни про Qt.
    std::string text;
    text += "id " + std::to_string(m_id);
    text += "  " + m_type;
    text += "  (" + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ")";
    text += "  H " + std::to_string(m_altitudeKm) + " км";
    text += "  V " + std::to_string(m_speedKmh) + " км/ч";
    text += "  курс " + std::to_string(m_headingDeg) + "°";
    return text;
}

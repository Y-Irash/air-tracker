#pragma once

#include <iosfwd>
#include <string>

#include "point.h"

// Живая цель: хранит состояние и умеет его обновлять по времени.
// Позицию и курс снаружи поменять нельзя — только через update().
class Target {
public:
    Target(int id, std::string type, Point2D position,
           double speedKmh, double headingDeg, double altitudeKm);
    Target() = delete;

    int id() const;
    const std::string &type() const;
    Point2D position() const;
    double speedKmh() const;
    double headingDeg() const;
    double altitudeKm() const;

    // Сдвинуть цель на интервал времени (в часах) по её курсу и скорости.
    void update(double dtHours);

    // Одна строка для таблицы/печати. Ядро не печатает — только отдаёт текст.
    std::string toString() const;

private:
    int m_id;
    std::string m_type;
    Point2D m_position;
    double m_speedKmh;
    double m_headingDeg;
    double m_altitudeKm;
};

#pragma once

#include <string>
#include <vector>

#include "point.h"
#include "target.h"

// Запись цели из файла: ровно то, что лежит в строке CSV, без поведения.
struct TargetData {
    int id{};
    std::string type;
    Point2D position;
    double altitude{};
    double speedKmh{};
    double headingDeg{};
};

// Читает цели из CSV (id,type,x,y,altitude,speed,heading).
// Файл не открылся — std::runtime_error с именем файла.
// Битая строка — std::runtime_error с номером строки. Пустой файл (только заголовок) — пустой список.
std::vector<TargetData> loadTargets(const std::string &path);

// Записывает цели в CSV в том же формате (заголовок + строка на цель).
void saveTargets(const std::vector<TargetData> &targets, const std::string &path);

// Превращает прочитанные записи в живые цели — чтобы консоль и окно делали это одинаково.
std::vector<Target> toTargets(const std::vector<TargetData> &data);

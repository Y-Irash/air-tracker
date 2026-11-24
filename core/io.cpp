#include "io.h"

#include <fstream>
#include <stdexcept>

namespace {

std::vector<std::string> splitFields(const std::string &line)
{
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (true) {
        const std::size_t comma = line.find(',', start);
        if (comma == std::string::npos) {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, comma - start));
        start = comma + 1;
    }
    return fields;
}

[[noreturn]] void fail(std::size_t lineNumber, const std::string &what)
{
    throw std::runtime_error("line " + std::to_string(lineNumber) + ": " + what);
}

int toInt(const std::string &field, std::size_t lineNumber)
{
    try {
        return std::stoi(field);
    } catch (const std::exception &) {
        fail(lineNumber, "ожидалось целое число, получено '" + field + "'");
    }
}

double toDouble(const std::string &field, std::size_t lineNumber)
{
    try {
        return std::stod(field);
    } catch (const std::exception &) {
        fail(lineNumber, "ожидалось число, получено '" + field + "'");
    }
}

} // namespace

std::vector<TargetData> loadTargets(const std::string &path)
{
    std::vector<TargetData> targets;

    std::ifstream in(path);
    if (!in) {
        return targets;
    }

    std::string line;
    std::size_t lineNumber = 0;
    bool headerSkipped = false;

    while (std::getline(in, line)) {
        ++lineNumber;

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        if (!headerSkipped) {
            headerSkipped = true;
            continue;
        }

        const std::vector<std::string> fields = splitFields(line);
        if (fields.size() < 7) {
            fail(lineNumber, "ожидалось 7 полей, найдено " + std::to_string(fields.size()));
        }

        TargetData target;
        target.id = toInt(fields[0], lineNumber);
        target.type = fields[1];
        target.position.x = toDouble(fields[2], lineNumber);
        target.position.y = toDouble(fields[3], lineNumber);
        target.altitude = toDouble(fields[4], lineNumber);
        target.speedKmh = toDouble(fields[5], lineNumber);
        target.headingDeg = normalizeHeading(toDouble(fields[6], lineNumber));

        targets.push_back(target);
    }

    return targets;
}

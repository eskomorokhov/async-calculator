#pragma once
#include <string>
#include <stdexcept>

#include "shunting_yard_calc.hpp"

template<typename TUnit=signed long>
class TCalculator {
public:
    std::string process(const std::string& line) const;
};

template<typename TUnit>
std::string TCalculator<TUnit>::process(const std::string& line) const {
    std::string out;
    if (shunting_yard_calc(line, out)) {
        return out;
    } else {
        throw std::runtime_error("Cannot parse string " + line);
    }
}

using Calculator=TCalculator<>;
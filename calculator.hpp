#pragma once
#include <string>
#include <stdexcept>

#include "shunting_yard_calc.hpp"


namespace Application
{

template<typename TUnit=signed long>
class TCalculator {
public:
    std::string process(const std::string& line) const;
};

template<typename TUnit>
std::string TCalculator<TUnit>::process(const std::string& line) const {
    std::string outcome;
    std::string error;
    if (shunting_yard_calc(line, outcome, error)) {
        return outcome;
    } else {
        throw std::runtime_error("parse error:" + error);
    }
}

using Calculator=TCalculator<>;

}	// namespace Application
#include <string>

template<typename TUnit=unsigned long>
class TCalculator {
public:
    std::string process(const std::string& line);
};

template<typename TUnit>
std::string TCalculator<TUnit>::process(const std::string &line) {

    return line;
}

using Calculator=TCalculator<>;
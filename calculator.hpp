#include <sstream>
#include <string>

template<typename TUnit=signed long>
class TCalculator {
public:
    std::string process(const std::string& line);
};

template<typename TUnit>
std::string TCalculator<TUnit>::process(const std::string &line) {
    TUnit result;
    std::istringstream ss(line);
    if (ss >> result) {
        return std::to_string(result);
    }
    return "";
}

using Calculator=TCalculator<>;
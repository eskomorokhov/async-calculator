//
// main <-> IO <-> calculator
//

#include <iostream>

#include <ctime>
#include <cstring>

#include "calculator.hpp"
#include "server.hpp"
#include "shunting_yard_calc.hpp"

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " ip port\n";
        return 1;
    }
    Calculator c;
    TCPServer server(argv[1], std::atoi(argv[2]), 2,
                     [&c](const std::string& expression) -> std::string {
                         auto time_start = std::time(0);
                         std::cerr << "Start at " << time_start << "\n";
                         auto res = c.process(expression) ;
                         std::cerr << "calc duration: " << std::time(0) - time_start << " sec res:" << res << "\n";
                         return res;
                     });
    server.run();
    return 0;
}

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
    if (argc > 1 && argc < 3) {
        std::cerr << "Usage: " << argv[0] << " ip port workers_number\n";
        return 1;
    }
    std::string ip = argc > 1 ? argv[1] : "127.0.0.1";
    int port = argc > 2 ? std::atoi(argv[2]) : 9090;
    int workers_number = argc > 3 ? std::atoi(argv[3]) : 2;
    Calculator c;
    TCPServer server(ip.c_str(), port, workers_number,
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

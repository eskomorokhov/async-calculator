//
// main <-> IO <-> calculator
//

#include <iostream>

#include <ostream>
#include <istream>

#include <calculator.hpp>

#include <server.hpp>
#include <ctime>
#include "shunting_yard_calc.hpp"

int main() {
    Calculator c;
    //TCPServer server("127.0.0.1", 9090, 2,
    //                 [&c](const std::string& expression) -> std::string { return c.process(expression); });
    TCPServer server("127.0.0.1", 9090, 2,
                     [&c](const std::string& expression) -> std::string {
                         auto time_start = std::time(0);
                         std::cerr << "Got expression: " << time_start << "\n";
                         auto res = c.process(expression) ;
                         std::cerr << "calc duration: " << std::time(0) - time_start << " sec res:" << res << "\n";
                         return res;
                     });
    server.run();
    return 0;
}

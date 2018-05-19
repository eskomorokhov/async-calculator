#include <cstring>
#include <iostream>
#include <random>

int main(int argc, const char*argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " size_bytes max_int";
        std::cerr << "\nGenerates expression contained sequence of literals with operators (*-+) "
            "\nwhere "
            "\nsize_bytes - required size of expression"
            "\nmax_int - maximum value of literals to avoid overflow";
        return 1;
    }
    std::ios::sync_with_stdio(false);
    const unsigned size = std::atoi(argv[1]);
    // need to prevent overflow during expressions
    const unsigned max_int = std::atoi(argv[2]);
    const unsigned length_literal = std::strlen(argv[2]);
    const unsigned avg_length_literal = length_literal /2 ? length_literal/2: 1;
    const int commands_needed = size / (avg_length_literal + 1);
    std::cout << 1;
    char operations[] = {'+','-','*'};
    for (int i = 0; i < commands_needed; ++i) {
        std::cout << operations[rand() % sizeof operations] << rand() % max_int;
    }

    return 0;
}

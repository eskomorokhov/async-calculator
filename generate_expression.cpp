#include <cstring>
#include <iostream>
#include <random>

int main(int argc, const char*argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " size_bytes max_int\n";
        return 1;
    }
    std::ios::sync_with_stdio(false);
    unsigned size = std::atoi(argv[1]);
    // need to prevent overflow during expressions
    unsigned max_int = std::atoi(argv[2]);
    unsigned len_number = std::strlen(argv[2]);
    unsigned avg_len_number = len_number /2 ? len_number/2: 1;
    int commands_needed = size / (avg_len_number + 1);
    std::cout << 1;
    char ops[] = {'+','-','*' };
    for (int i = 0; i < commands_needed; ++i) {
        
        std::cout << ops[rand()%sizeof ops] << rand() % max_int;
    }

    return 0;
}

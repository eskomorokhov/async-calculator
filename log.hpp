#pragma once

void log_debug() {
//    std::cerr << "\n";
}

template <typename TArg, typename... Args> 
void log_debug(TArg /*arg*/, Args&&... args) {
//    std::cerr << arg;
    log_debug(std::forward<Args>(args)...);
}

void log_info() {
    std::cerr << "\n";
}

template <typename TArg, typename... Args> 
void log_info(TArg arg, Args&&... args) {
    std::cerr << arg;
    log_info(std::forward<Args>(args)...);
}
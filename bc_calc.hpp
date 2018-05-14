#pragma once
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>

inline void close_pipe(std::FILE* fp) { pclose(fp); }

inline bool calc_using_system(const std::string& expression, std::string& outcome) {
    if (expression.empty()) {
        return false;
    }

    const auto buf_max = 1024;
    char buf[buf_max];
    std::string cmd = "echo '";
    cmd += expression;
    cmd += "' | bc -q 2>&1 ";
    std::unique_ptr<std::FILE, decltype(&close_pipe)> fp(popen(cmd.c_str(), "r"), &close_pipe);
    if (!fp) {
        return false;
    }

    bool first = true;
    while (std::fgets(buf, sizeof buf, fp.get()) != NULL) {
        if (first) {
            first = false;
        } else {
            outcome += "\n";
        }
        outcome += buf;
    }
    if (outcome.back() == '\n') {
        outcome.pop_back();
    }
    if (std::strstr(outcome.c_str(), "error") != nullptr) {
        return false;
    }
    return true;
}


#pragma once
#include <list>
#include <map>
#include <stack>
#include <vector>
#include <string>

inline bool shunting_yard_calc(const std::string& expression, std::string& outcome) {
    static const std::map<char, int> operators_precedence = {{'*', 10}, {'/', 10}, {'+', 8}, {'-', 9}, {'n',11}};
    std::stack<char> ops;
    std::list<std::string> rpn;
    const char* p = expression.c_str();
    const char* p_end = expression.c_str() + expression.size();
    bool prev_op = true;
    for(; p != p_end; ++p) {
        if (std::isdigit(*p) ) {
            int v = 0;
            do {
                v *=10;
                v += (*p) -'0';
                ++p;
            } while (std::isdigit(*p));
            --p;
            rpn.push_back(std::to_string(v));
            prev_op = false;
        } else if (std::isspace(*p)){
            //skip
        } else {
            auto cmd = *p;
            if(prev_op && cmd == '-') {
                cmd = 'n';
            }
            {
                if (ops.empty() || operators_precedence.at(ops.top()) < operators_precedence.at(cmd)) {
                    ops.push(cmd);
                    prev_op = true;
                } else {
                    std::string op;
                    op += ops.top();
                    rpn.push_back(op);
                    ops.pop();
                    --p;
                }
            }
            //std::copy(rpn.begin(), rpn.end(), std::ostream_iterator<std::string>(std::cerr));
            //std::cerr << "->";
        }
    }
    if (ops.empty() && rpn.empty()) {
        return false;
    }
    while (!ops.empty()) {
        std::string op;
        op += ops.top();
        rpn.push_back(op);
        ops.pop();
    }
    //std::copy(rpn.begin(), rpn.end(), std::ostream_iterator<std::string>(std::cerr));
    //std::cerr << ":";

    for (auto it = rpn.begin()++; it != rpn.end(); ++it) {
        if (std::isdigit((*it)[0])) {
        } else {
            if ((*it)[0] == 'n') {
                if (std::distance(rpn.begin(), it) < 1) {
                    return false;
                }
                auto it_literal = it;
                it_literal--;
                int res = -atoi(it_literal->c_str());
                *it = std::to_string(res);
                rpn.erase(it_literal, it);
            } else {
                if (std::distance(rpn.begin(), it) < 2) {
                    return false;
                }
                auto it_literal = it;
                std::advance(it_literal, -2);
                auto &a = *it_literal;
                ++it_literal;
                auto &b = *it_literal;
                int res = 0;
                switch ((*it)[0]) {
                    case '+':
                        res = atoi(a.c_str()) + atoi(b.c_str());
                        break;
                    case '-':
                        res = atoi(a.c_str()) - atoi(b.c_str());
                        break;
                    case '*':
                        res = atoi(a.c_str()) * atoi(b.c_str());
                        break;
                    case '/':
                        res = atoi(a.c_str()) / atoi(b.c_str());
                        break;
                };
                *it = std::to_string(res);
                it_literal--;
                rpn.erase(it_literal, it);
            }
        }
    }
    outcome = rpn.front();
    return true;
}



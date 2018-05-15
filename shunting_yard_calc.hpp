#pragma once
#include <list>
#include <map>
#include <stack>
#include <vector>
#include <string>

template<typename TUnit=long>
inline bool shunting_yard_calc(const std::string& expression, std::string& outcome) {
    struct Item{
        bool is_literal;
        union {
            TUnit literal;
            char op;
        } value;
    };

    //std::cerr<< "exp:" << expression << "\n";
    static const std::map<char, int> operators_precedence = {{'*', 10}, {'/', 10}, {'+', 8}, {'-', 9}, {'n',11}, {'(',1}, {')',1}};
    std::deque<Item> ops;
    std::deque<Item> rpn;
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
            Item literal;
            literal.is_literal = true;
            literal.value.literal = v;
            rpn.push_back(literal);
            prev_op = false;
        } else if (std::isspace(*p)){
            //skip
        } else if (*p == '(') {
            Item op_item;
            op_item.is_literal = false;
            op_item.value.op = *p;
            ops.push_back(op_item);
            prev_op = false;
        } else if (*p == ')') {
            while (!ops.empty() && ops.back().value.op != '(') {
                rpn.push_back(ops.back());
                ops.pop_back();
            }
            if (!ops.empty() && ops.back().value.op == '(') {
                ops.pop_back();
            }
        } else {
            auto cmd = *p;
            if (prev_op && cmd == '-') {
                cmd = 'n';
            }

            {
                if (ops.empty() || operators_precedence.at(ops.back().value.op) < operators_precedence.at(cmd)) {
                    Item op;
                    op.is_literal = false;
                    op.value.op = cmd;
                    ops.push_back(op);
                    prev_op = true;
                } else {
                    rpn.push_back(ops.back());
                    ops.pop_back();
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
        rpn.push_back(ops.back());
        ops.pop_back();
    }
    //std::copy(rpn.begin(), rpn.end(), std::ostream_iterator<std::string>(std::cerr));
    //std::cerr << ":";

    for (auto it = rpn.begin()++; it != rpn.end(); ++it) {
        if (it->is_literal) {
        } else {
            if (it->value.op == 'n') {
                if (std::distance(rpn.begin(), it) < 1) {
                    return false;
                }
                auto it_literal = it;
                it_literal--;
                if (!it_literal->is_literal) {
                    return false;
                }
                *it = *it_literal;
                it->value.literal = -it_literal->value.literal;
                rpn.erase(it_literal, it);
            } else {
                if (std::distance(rpn.begin(), it) < 2) {
                    return false;
                }
                auto it_literal = it;
                std::advance(it_literal, -2);
                const auto &a = *it_literal;
                ++it_literal;
                const auto &b = *it_literal;
                if (!a.is_literal || !b.is_literal) {
                    return false;
                }
                const auto& a_val = a.value.literal;
                const auto& b_val = b.value.literal;
                TUnit res = 0;
                switch (it->value.op) {
                    case '+':
                        res = a_val + b_val;
                        break;
                    case '-':
                        res = a_val - b_val;
                        break;
                    case '*':
                        res = a_val * b_val;
                        break;
                    case '/':
                        if (b_val == 0) {
                            return false;
                        }
                        res = a_val / b_val;
                        break;
                };
                *it = a;
                it->value.literal = res;
                it_literal--;
                rpn.erase(it_literal, it);
            }
        }
    }
    const auto& res_item = rpn.front();
    if (!res_item.is_literal) {
        return false;
    }
    outcome = std::to_string(res_item.value.literal);
    return true;
}

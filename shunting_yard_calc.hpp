#pragma once
#include <iterator>
#include <list>
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <iostream>

//! Infix expression evaluation 
//! Shunting yard algorithm https://en.wikipedia.org/wiki/Shunting-yard_algorithm
//! \param expression [in] string with expression for evaluation
//! \param outcome [out] string with result if success, undefined else
//! \param error [out] string with error description if fail, undefined else
template<typename TUnit=long>
inline bool shunting_yard_calc(
            const std::string& expression,
            std::string& outcome,
            std::string& error
    ) {
    // Item represents tokens of expression in internal representation
    // Contains source literals and operations, intermediate results 
    struct Item {
        bool is_literal;
        union {
            TUnit literal;
            char operation_character;
        } value;
    };

    std::cerr<< "exp:" << expression << "\n";
    static const std::map<char, int> operators_precedence = {{'*', 10}, {'/', 10}, {'+', 8}, {'-', 9}, {'n',11}, {'(',1}, {')',1}};
    std::deque<Item> ops;   //! Pending queue of operations, first step of shunting-yard algorithm
    std::deque<Item> rpn;   //! Reverse polish notation https://en.wikipedia.org/wiki/Polish_notation 
    const char* current_character = expression.c_str();
    const char* end_character = expression.c_str() + expression.size();
    bool previous_token_is_operation = true;
    unsigned parenthesis_depth = 0;
    for(; current_character != end_character; ++current_character) {
        if (std::isdigit(*current_character) ) {
            int v = 0;
            do {
                v *=10;
                v += (*current_character) - '0';
                ++current_character;
            } while (std::isdigit(*current_character));
            --current_character;
            if (!previous_token_is_operation) {
                error = "an unexpected literal, missed an operation before " + std::to_string(v);
                return false;
            }
            Item literal;
            literal.is_literal = true;
            literal.value.literal = v;
            rpn.push_back(literal);
            previous_token_is_operation = false;
        } else if (std::isspace(*current_character)){
            //skip
        } else if (*current_character == '(') {
            if (!previous_token_is_operation) {
                error = "starts parenthesis without a conjuntion operation";
                return false;
            }
            Item op_item;
            op_item.is_literal = false;
            op_item.value.operation_character = *current_character;
            ops.push_back(op_item);
            previous_token_is_operation = true;
            parenthesis_depth++;
        } else if (*current_character == ')') {
            if (previous_token_is_operation) {
                error = "an unexpected parentheses";
                return false;
            }
            while (!ops.empty() && ops.back().value.operation_character != '(') {
                rpn.push_back(ops.back());
                ops.pop_back();
            }
            if (!ops.empty() && ops.back().value.operation_character == '(') {
                ops.pop_back();
            } else {
                error = "missing left parentheses";
                return false;
            }
            previous_token_is_operation = false;
            parenthesis_depth--;
        } else {
            auto cmd = *current_character;
            if (previous_token_is_operation && cmd == '-') {
                cmd = 'n';
            }
            if (previous_token_is_operation && cmd != 'n') {
                error = "an unexpected operation, missing literal or parentheses";
                return false;
            }
            {
                if (ops.empty() || operators_precedence.at(ops.back().value.operation_character) < 
                        operators_precedence.at(cmd)) {
                    Item operation_character;
                    operation_character.is_literal = false;
                    operation_character.value.operation_character = cmd;
                    ops.push_back(operation_character);
                    previous_token_is_operation = true;
                } else {
                    rpn.push_back(ops.back());
                    ops.pop_back();
                    --current_character;
                }
            }
            //std::copy(rpn.begin(), rpn.end(), std::ostream_iterator<std::string>(std::cerr));
            //std::cerr << "->";
        }
    }
    if (parenthesis_depth > 0) {
        error = "missing right parentheses";
        return false;
    }
    if (ops.empty() && rpn.empty()) {
        return false;
    }
    while (!ops.empty()) {
        rpn.push_back(ops.back());
        ops.pop_back();
    }
    //for (const auto&item: rpn) {
    //    (item.is_literal ? (std::cerr <<item.value.literal) : (std::cerr << item.value.operation_character)) << " ";
    //}
    //std::cerr << ":";

    for (auto it = rpn.begin()++; it != rpn.end(); ++it) {
        if (!it->is_literal) {
            if (it->value.operation_character == 'n') {
                if (std::distance(rpn.begin(), it) < 1) {
                    error = "missed an operand for unary operation -";
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
                    error = "missed operand(s) for operation " + std::string(1, it->value.operation_character);
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
                const auto& left_operand = a.value.literal;
                const auto& right_operand = b.value.literal;
                TUnit res = 0;
                switch (it->value.operation_character) {
                    case '+':
                        res = left_operand + right_operand;
                        break;
                    case '-':
                        res = left_operand - right_operand;
                        break;
                    case '*':
                        res = left_operand * right_operand;
                        break;
                    case '/':
                        if (right_operand == 0) {
                            return false;
                        }
                        res = left_operand / right_operand;
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

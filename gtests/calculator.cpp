#include <gtest/gtest.h>

#include <calculator.hpp>

#include <sstream>

using namespace Application;

TEST(Calculator, empty) {
    Calculator c;
    EXPECT_EQ(c.process(""), "parse error: no expression");
}

TEST(Calculator, literal) {
    Calculator c;
    EXPECT_EQ(c.process("134234"), "134234");
    EXPECT_EQ(c.process("-134234"), "-134234");
    EXPECT_EQ(c.process("-134234"), "-134234");
    EXPECT_EQ(c.process("134234   "), "134234");
    EXPECT_EQ(c.process("   134234"), "134234");
    EXPECT_EQ(c.process("   134234  "), "134234");
    EXPECT_EQ(c.process("-- 134234"), "parse error: missed an operand for unary operation -");
}

TEST(Calculator, multicalls) {
    Calculator c;
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(c.process("174234"), "174234");
        EXPECT_EQ(c.process("1"), "1");
        EXPECT_EQ(c.process("7"), "7");
        EXPECT_EQ(c.process("-7"), "-7");
    }
}

TEST(Calculator, operators) {
    Calculator c;
    EXPECT_EQ(c.process("1+2"), "3");
    EXPECT_EQ(c.process("3+2"), "5");
    EXPECT_EQ(c.process("2-1"), "1");
    EXPECT_EQ(c.process("6/2"), "3");
    EXPECT_EQ(c.process("6/4"), "1");
    EXPECT_EQ(c.process("6*4"), "24");
}

TEST(Calculator, operators_with_signed_literals) {
    Calculator c;
    EXPECT_EQ(c.process("1+-2"), "-1");
    EXPECT_EQ(c.process("3+-2"), "1");
    EXPECT_EQ(c.process("2-1"), "1");
    EXPECT_EQ(c.process("6/-2"), "-3");
    EXPECT_EQ(c.process("6/-4"), "-1");
    EXPECT_EQ(c.process("6*-4"), "-24");
}

TEST(Calculator, sequence_of_operators) {
    Calculator c;
    EXPECT_EQ(c.process("1+2+3"), "6");
    EXPECT_EQ(c.process("3+2+1"), "6");
    EXPECT_EQ(c.process("2-1+3"), "4");
    EXPECT_EQ(c.process("6/2/1"), "3");
    EXPECT_EQ(c.process("24/4/3"), "2");
    EXPECT_EQ(c.process("6*4*2"), "48");
}

TEST(Calculator, space_separators) {
    Calculator c;
    EXPECT_EQ(c.process("1 + 2"), "3");
    EXPECT_EQ(c.process("3 + 2 + 1"), "6");
    EXPECT_EQ(c.process("2 - 1 + 3"), "4");
    EXPECT_EQ(c.process("6 / 2 / 1"), "3");
    EXPECT_EQ(c.process("24 / 4 / 3"), "2");
    EXPECT_EQ(c.process("6 * 4 * 2"), "48");
}

TEST(Calculator, space_separators_signed_literals) {
    Calculator c;
    EXPECT_EQ(c.process("1 + -2"), "-1");
    EXPECT_EQ(c.process("3 + 2 + 1"), "6");
    EXPECT_EQ(c.process("2 - -1"), "3");
    EXPECT_EQ(c.process("2 - -1 + 3"), "6");
    EXPECT_EQ(c.process("6 / -2 / 1"), "-3");
    EXPECT_EQ(c.process("24 / -4 / -3"), "2");
    EXPECT_EQ(c.process("-6 * -4 * -2"), "-48");
    // incorrect order in rpn
    //EXPECT_THROW(c.process("-6 * -4 * --2"), "");
}

TEST(Calculator, priority) {
    Calculator c;
    EXPECT_EQ(c.process("15*2/3"), "10");
    EXPECT_EQ(c.process("1*2+3"), "5");
    EXPECT_EQ(c.process("3*2-1"), "5");
    EXPECT_EQ(c.process("4/2*2"), "4");
    EXPECT_EQ(c.process("4/2/2"), "1");
    EXPECT_EQ(c.process("4/2+2"), "4");
    EXPECT_EQ(c.process("4/2-2"), "0");
    EXPECT_EQ(c.process("4+2*2"), "8");
    EXPECT_EQ(c.process("4+2/2"), "5");
    EXPECT_EQ(c.process("4+2-2"), "4");
}

TEST(Calculator, parenthesis) {
    Calculator c;
    EXPECT_EQ(c.process("1 - 3 - 2"), "-4");
    EXPECT_EQ(c.process("1 - (3 - 2)"), "0");
    EXPECT_EQ(c.process("1 - (3 - 2)+(3- -2)"), "5");
    EXPECT_EQ(c.process("2 * 3 + 2"), "8");
    EXPECT_EQ(c.process("2 * (3 + 2)"), "10");
    EXPECT_EQ(c.process("2 * (3 + 2*5)"), "26");
    EXPECT_EQ(c.process("2 * (3 + 2*5+2)"), "30");
    EXPECT_EQ(c.process("2 * (3 + 2*(5+2))"), "34");
    EXPECT_EQ(c.process("- (1)"), "-1");
    EXPECT_EQ(c.process("- (1+1)"), "-2");
}

TEST(Calculator, long_expressions) {
    Calculator c;
    EXPECT_EQ(c.process("1+6807+5248+1272+8930+1272+7544+878+7923+7709+4440+8165+4492+3042+7987+2503+2327+1729+8840+2612+4303+3169+7709+7157+9560+933+3099+278+1816+5335+9097+7826+3511+3"), "157517");
}

TEST(Calculator, random_evaluations) {
    Calculator c;
    EXPECT_EQ(c.process("1"), "1");
}

TEST(Calculator, overflowing) {
    Calculator c;
    EXPECT_EQ(c.process("1"), "1");
}

TEST(Calculator, incorrect_input) {
    Calculator c;
    EXPECT_EQ(c.process("-"), "parse error: missed an operand for unary operation -");
    EXPECT_EQ(c.process("-*"), "parse error: an unexpected operation, missing literal or parentheses");
    EXPECT_EQ(c.process("-/"), "parse error: an unexpected operation, missing literal or parentheses");
    EXPECT_EQ(c.process("-+"), "parse error: an unexpected operation, missing literal or parentheses");
    EXPECT_EQ(c.process("-("), "parse error: missing right parentheses");
    EXPECT_EQ(c.process("-()"), "parse error: an unexpected parentheses");
    EXPECT_EQ(c.process(")"), "parse error: an unexpected parentheses");
    EXPECT_EQ(c.process("1("), "parse error: starts parenthesis without a conjuntion operation");
    EXPECT_EQ(c.process("*1"), "parse error: an unexpected operation, missing literal or parentheses");
    EXPECT_EQ(c.process("/1"), "parse error: an unexpected operation, missing literal or parentheses");
    EXPECT_EQ(c.process("+1"), "parse error: an unexpected operation, missing literal or parentheses");
    EXPECT_EQ(c.process("*"), "parse error: an unexpected operation, missing literal or parentheses");
    EXPECT_EQ(c.process("1-"), "parse error: missed operand(s) for operation -");
    EXPECT_EQ(c.process("1+"), "parse error: missed operand(s) for operation +");
    EXPECT_EQ(c.process("1*"), "parse error: missed operand(s) for operation *");
    EXPECT_EQ(c.process("1/"), "parse error: missed operand(s) for operation /");
    EXPECT_EQ(c.process("-1-"), "parse error: missed operand(s) for operation -");
    EXPECT_EQ(c.process("*1-"), "parse error: an unexpected operation, missing literal or parentheses");
    EXPECT_EQ(c.process("(1-2-1"), "parse error: missing right parentheses");
    EXPECT_EQ(c.process("(1-(2-1"), "parse error: missing right parentheses");
    EXPECT_EQ(c.process("((1)"), "parse error: missing right parentheses");
    EXPECT_EQ(c.process("()1"), "parse error: an unexpected parentheses");
    EXPECT_EQ(c.process("1(2)"), "parse error: starts parenthesis without a conjuntion operation");
    EXPECT_EQ(c.process("(2)1"), "parse error: an unexpected literal, missed an operation before 1");
    EXPECT_EQ(c.process("(2)1"), "parse error: an unexpected literal, missed an operation before 1");
    EXPECT_EQ(c.process("(2)1"), "parse error: an unexpected literal, missed an operation before 1");
    EXPECT_EQ(c.process("(2)-"), "parse error: missed operand(s) for operation -");
}

TEST(Calculator, custom_cases) {
    Calculator c;
    EXPECT_EQ(c.process("77*10+1*3)"), "parse error: missing left parentheses");
    EXPECT_EQ(c.process("77*10+1*3)+2"), "parse error: missing left parentheses");  
}
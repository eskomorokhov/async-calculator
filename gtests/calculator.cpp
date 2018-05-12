#include <gtest/gtest.h>

#include <calculator.hpp>

#include <sstream>

TEST(Calculator, empty) {
    Calculator c;
    EXPECT_THROW(c.process(""), std::runtime_error);
}


TEST(Calculator, literal) {
    Calculator c;
    EXPECT_EQ(c.process("134234"), "134234");
    EXPECT_EQ(c.process("-134234"), "-134234");
    EXPECT_EQ(c.process("-134234"), "-134234");
    EXPECT_EQ(c.process("134234   "), "134234");
    EXPECT_EQ(c.process("   134234"), "134234");
    EXPECT_EQ(c.process("   134234  "), "134234");
    EXPECT_THROW(c.process("- 134234"), std::runtime_error);
}

TEST(Calculator, multicalls) {
    Calculator c;
    for (int i = 0; i < 1000; ++i) {
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
    EXPECT_EQ(c.process("2-+1"), "1");
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
    EXPECT_EQ(c.process("2 - -1 + 3"), "6");
    EXPECT_EQ(c.process("6 / -2 / 1"), "-3");
    EXPECT_EQ(c.process("24 / -4 / -3"), "2");
    EXPECT_EQ(c.process("-6 * -4 * -2"), "-48");
    EXPECT_THROW(c.process("-6 * -4 * --2"), std::runtime_error);
}

TEST(Calculator, priority) {
    Calculator c;
    EXPECT_EQ(c.process("1"), "1");
}

TEST(Calculator, parentheses) {
    Calculator c;
    EXPECT_EQ(c.process("1"), "1");
}

TEST(Calculator, random_evaluations) {
    Calculator c;
    EXPECT_EQ(c.process("1"), "1");
}

TEST(Calculator, overflowing) {
    Calculator c;
    EXPECT_EQ(c.process("1"), "1");
}

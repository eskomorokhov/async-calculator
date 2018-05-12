#include <gtest/gtest.h>

#include <calculator.hpp>

#include <sstream>

TEST(Calculator, empty) {
    Calculator c;
    EXPECT_EQ(c.process(""), "");
}


TEST(Calculator, literal) {
    Calculator c;
    EXPECT_EQ(c.process("134234"), "134234");
    EXPECT_EQ(c.process("   134234  "), "134234");
}

TEST(Calculator, multicalls) {
    Calculator c;
    EXPECT_EQ(c.process(""), "");
}

TEST(Calculator, operators) {
    Calculator c;
    EXPECT_EQ(c.process(""), "");
}

TEST(Calculator, priority) {
    Calculator c;
    EXPECT_EQ(c.process(""), "");
}

TEST(Calculator, parentheses) {
    Calculator c;
    EXPECT_EQ(c.process(""), "");
}

TEST(Calculator, random_evaluations) {
    Calculator c;
    EXPECT_EQ(c.process(""), "");
}

TEST(Calculator, overflowing) {
    Calculator c;
    EXPECT_EQ(c.process(""), "");
}

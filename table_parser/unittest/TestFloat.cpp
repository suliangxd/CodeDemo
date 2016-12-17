#include <gtest/gtest.h>
#include <cmath>
#include <cstring>

#include "../src/table_parser.cpp"

#define GENERATE_LEGAL_TEST(NAME, VALUE)                                 \
    TEST(TestFloat, NAME) {                                              \
        float f;                                                         \
        ASSERT_TRUE(tp::parse_float_callback(#VALUE, strlen(#VALUE), &f, \
                                             sizeof(float), NULL));      \
        EXPECT_FLOAT_EQ(VALUE, f);                                       \
        SUCCEED();                                                       \
    }

GENERATE_LEGAL_TEST(TestTrivial, 3.1415927)
GENERATE_LEGAL_TEST(TestPositive, +2.718281828)
GENERATE_LEGAL_TEST(TestNegative, -0.1428571)
GENERATE_LEGAL_TEST(TestPoint, 1.)
GENERATE_LEGAL_TEST(TestZero, 0.000)
GENERATE_LEGAL_TEST(TestExp, 1e11)
GENERATE_LEGAL_TEST(TestPosExp, 1e+8)
GENERATE_LEGAL_TEST(TestNegExp, 1e-7)
GENERATE_LEGAL_TEST(TestZeroExp, 5e0)
GENERATE_LEGAL_TEST(TestPointExp, 1.e4)
GENERATE_LEGAL_TEST(TestLongExp, 1e00000000000000000000000010)
GENERATE_LEGAL_TEST(TestMax, 3.40e+38)
GENERATE_LEGAL_TEST(TestMin, -3.40e+38)
GENERATE_LEGAL_TEST(TestCapitalExp, 1.5E23)
GENERATE_LEGAL_TEST(TestLong1, 123456789012345678901234567890123456789.)
GENERATE_LEGAL_TEST(TestLong2, 0.0000000000000000000000000000000000001)

TEST(TestFloat, TestInf) {
    float f;
    ASSERT_TRUE(tp::parse_float_callback("1e99", 4, &f, sizeof(float), NULL));
    EXPECT_NE(0, isinf(f));
    EXPECT_GT(f, 0);
}

TEST(TestFloat, TestNegInf) {
    float f;
    ASSERT_TRUE(tp::parse_float_callback("-1e99", 5, &f, sizeof(float), NULL));
    EXPECT_NE(0, isinf(f));
    EXPECT_LT(f, 0);
}

#define GENERATE_ILLEGAL_TEST(NAME, VALUE)                              \
    TEST(TestFloat, NAME) {                                             \
        float f;                                                        \
        ASSERT_FALSE(tp::parse_float_callback(VALUE, strlen(VALUE), &f, \
                                              sizeof(float), NULL));    \
    }

GENERATE_ILLEGAL_TEST(TestJunk, "ooooooooorz")
GENERATE_ILLEGAL_TEST(TestEmpty, "")
GENERATE_ILLEGAL_TEST(TestExtra, "123.45e33orz")
GENERATE_ILLEGAL_TEST(TestIllegalSym, "*5e11")
GENERATE_ILLEGAL_TEST(TestIllegalExpSym, "5e*11")
GENERATE_ILLEGAL_TEST(TestDuplicateSym, "--5e11")
GENERATE_ILLEGAL_TEST(TestDuplicateExp, "5ee11")

TEST(TestFloat, TestSizeTooLarge) {
    double f;
    ASSERT_FALSE(tp::parse_float_callback("0", 1, &f, sizeof(f), NULL));
}

TEST(TestFloat, TestSizeTooSmall) {
    char f;
    ASSERT_FALSE(tp::parse_float_callback("0", 1, &f, sizeof(f), NULL));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

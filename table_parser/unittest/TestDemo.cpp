//
// Created by Liang on 2016/12/14.
//
#include <gtest/gtest.h>
#include <cstring>
#include <iostream>

#include "table_parser.h"

using namespace std;

struct sub_data {
    bool a;
};

struct my_data {
    unsigned count_a;
    float a[5];
    char b[32];
    int c;
    sub_data d;
};

bool ParseSubDataCallback(const char* s, unsigned len, void* data,
                          unsigned size, void* context) {
    sub_data* p = reinterpret_cast<sub_data*>(data);

    if (size != sizeof(sub_data)) return false;

    if (::strncmp(s, "true", len) == 0)
        p->a = true;
    else if (::strncmp(s, "false", len) == 0)
        p->a = false;
    else
        return false;
    return true;
}

tp::ColumnDescriptor my_data_desc[] = {
    {tp::KFLOAT, true, 5, sizeof(float), offsetof(my_data, a),
     offsetof(my_data, count_a), nullptr, nullptr},
    {tp::KSTRING, false, 0, sizeof(my_data::b), offsetof(my_data, b), 0,
     nullptr, nullptr},
    {tp::KINT, false, 0, sizeof(int), offsetof(my_data, c), 0, nullptr,
     nullptr},
    {tp::KCLASS, false, 0, sizeof(sub_data), offsetof(my_data, d), 0,
     ParseSubDataCallback, nullptr},
    {tp::KNONE, false, 0, 0, 0, 0, nullptr, nullptr}};

TEST(DemoTest, HandleLegalInput) {
    const char* input =
        "3:-1.5,2.23,1\thello world!\t47\ttrue\n"
        "2:3.1415927,2.7182818\tooooorz\t42\tfalse\n";
    static const my_data output[] = {
        {3, {-1.5, 2.23, 1.0}, "hello world!", 47, {true}},
        {2, {3.1415927, 2.7182818}, "ooooorz", 42, {false}}};

    vector<my_data> results;
    vector<string> errors;
    unsigned count = tp::parse_all(input, my_data_desc, results, errors);

    EXPECT_EQ(2, count);
    ASSERT_EQ(2, results.size());

    for (unsigned i = 0; i < results.size(); ++i) {
        // Make a closure for ASSERT_XXX
        [&]() {
            ASSERT_EQ(output[i].count_a, results[i].count_a);

            for (unsigned j = 0; j < results[i].count_a; ++j)
                EXPECT_FLOAT_EQ(output[i].a[j], results[i].a[j]);
        }();
        EXPECT_STREQ(output[i].b, results[i].b);
        EXPECT_EQ(output[i].c, results[i].c);
        EXPECT_EQ(output[i].d.a, results[i].d.a);
    }

    SUCCEED();
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

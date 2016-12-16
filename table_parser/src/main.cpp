//
// Created by Liang on 2016/12/14.
//
#include <iostream>
#include <cstring>


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

bool ParseSubDataCallback(const char* s, size_t len, void* data, size_t size, void* context) {
    sub_data* p = reinterpret_cast<sub_data*>(data);

    if (size != sizeof(sub_data))
        return false;

    if (::strncmp(s, "true", len) == 0)
        p->a = true;
    else if (::strncmp(s, "false", len) == 0)
        p->a = false;
    else
        return false;
    return true;
}

tp::ColumnDescriptor my_data_desc[] = {
        {tp::KFLOAT, true, 5, sizeof(float), offsetof(my_data, a), offsetof(my_data, count_a),
                                                                              nullptr, nullptr },
        {tp::KSTRING, false, 0, sizeof(my_data::b), offsetof(my_data, b), 0, nullptr, nullptr },
        {tp::KINT, false, 0, sizeof(int), offsetof(my_data, c), 0, nullptr, nullptr },
        {tp::KCLASS, false, 0, sizeof(sub_data), offsetof(my_data, d), 0, ParseSubDataCallback,
                                                                                       nullptr },
        {tp::KNONE, false, 0, 0, 0, 0, nullptr, nullptr }
};

const char* my_data_text = "3:-1.5,2.23,1\thello world!\t+100011111111111\ttrue\n3:0,0.1,1e2\ttest\t12345\tfale";

// const char* my_data_text = "3:-1.5,0.1,1e2\thello world!\t-100\ttrue\n3:0,0.1,1e2\ttest\t12345\tfalse";

int main() {
    vector<my_data> results;
    vector<string> errors;
    size_t count = tp::parse_all(my_data_text, my_data_desc, results, errors);

    for (int i = 0; i < errors.size(); ++i)
        cout << errors[i] << endl;
    for (int i = 0; i < results.size(); ++i) {
        for (int j = 0; j < results[i].count_a; ++j)
            cout << results[i].a[j] << " ";
        cout << results[i].b << " ";
        cout << results[i].c << " ";
        cout << results[i].d.a;
        cout << endl;
    }

    return 0;
}

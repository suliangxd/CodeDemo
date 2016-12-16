//
// Created by Liang on 2016/12/12.
//

#ifndef TABLEPARSER_TABLE_PARSER_H
#define TABLEPARSER_TABLE_PARSER_H

#include <cassert>
#include <cstddef>
#include <cstdint>

#include <string>
#include <vector>

namespace tp {

/**
 * @brief 数据类型
 *
 * 定义了数据类型的基本构成类型
 */
enum DataType { KNONE = 0, KINT, KFLOAT, KSTRING, KCLASS };

/**
 * @brief 描述解析结果
 */
enum ParseResult { KOK = 0, KERROR, KEOF };

/**
 * @brief 自定义解析回调函数
 * @param[in] s 被解析字符串开始位置, 注意该字符串不一定以'\0'结尾
 * @param[in] len 字符串s的长度
 * @param[in,out] data 反序列化目的内存
 * @param[in] size 目的内存大小
 * @param[in] context 回调函数上下文
 * @return 解析是否成功
 *
 * 用于解析用户自定义结构的回调函数
 */
typedef bool (*parser_callback)(const char* s, unsigned len, void* data,
                                unsigned size, void* context);

/**
 * @brief 列结构描述符
 *
 * 描述一个表的各列的数据类型
 */
struct ColumnDescriptor {
    DataType type;          /// @brief 列中数据类型
    bool is_array;          /// @brief 是否为数组
    unsigned array_max;     /// @brief 数组元素的最大个数
    unsigned element_size;  /// @brief 元素的大小

    unsigned offset;  /// @brief 该数据域在结构体中的位置
    unsigned array_counter_offset;  ///@brief 数组计数偏移

    parser_callback callback;  ///@brief 自定义解析回调函数
    void* context;             ///@brief 回调函数上下文
};

class TableParser {
   public:
    TableParser(const char* src, const ColumnDescriptor desc[]);

    TableParser(const TableParser& org);

    TableParser& operator=(const TableParser& rhs);

    /**
     * @brief 解析函数
     *
     */
    ParseResult parse(void* p, unsigned size);

    const char* last_error() const;

    ~TableParser(){};

   private:
    ParseResult parse_element(unsigned idx, const char* s, unsigned len,
                              void* data);

   private:
    const char* _src;
    const ColumnDescriptor* _desc;
    unsigned _line;
    char _err[128];
};

template <typename T>
unsigned parse_all(const char* src, const ColumnDescriptor desc[],
                   std::vector<T>& out, std::vector<std::string>& err) {
    unsigned ret = 0;

    TableParser tb_parser(src, desc);
    while (true) {
        T object;
        ParseResult result = tb_parser.parse(&object, sizeof(T));
        if (result == KERROR) {
            err.push_back(tb_parser.last_error());
        } else if (result == KEOF) {
            break;
        } else {
            assert(result == KOK);
            err.push_back(tb_parser.last_error());
            out.push_back(object);
            ++ret;
        }
    }

    return ret;
}
}
#endif  // TABLEPARSER_TABLE_PARSER_H

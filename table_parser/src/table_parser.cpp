//
// Created by Liang on 2016/12/12.
//
#include "table_parser.h"

#include <cmath>
#include <cstdlib>

#define UNUSED(p) static_cast<void>(p)

namespace tp {

    // 支持针对10进制带符号32位整数的解析
    static bool parse_int_callback(const char *s, unsigned len, void *data, unsigned size,
                                   void *context) {
        UNUSED(context);

        if (size != sizeof(int)) {
            return false;
        }

        if (len == 0) {
            return false;
        }

        // 符号位
        bool neg = false;
        if (s[0] == '+') {
            ++s;
            --len;
        } else if (s[0] == '-') {
            ++s;
            --len;
            neg = true;
        }

        // 数字
        int ret = 0;
        for (unsigned i = 0; i < len; ++i) {
            char c = s[i];
            if (c >= '0' && c <= '9') {
                ret = ret * 10 + (c - '0');
            } else {
                return false;
            }
        }

        if (neg) {
            ret = -ret;
        }

        *reinterpret_cast<int *>(data) = ret;
        return true;
    }

    // 支持符合浮点数表达的解析
    // 参见json.org的语法描述
    static bool parse_float_callback(const char *s, unsigned len, void *data, unsigned size,
                                     void *context) {
        UNUSED(context);

        if (size != sizeof(float)) {
            return false;
        }

        if (len == 0) {
            return false;
        }

        // 符号位
        bool neg = false;
        if (s[0] == '+') {
            ++s;
            --len;
        } else if (s[0] == '-') {
            ++s;
            --len;
            neg = true;
        }

        int int_part = 0;
        float frac_part = 0;
        float frac_pos = 0.1;
        bool neg_power = false;
        int power = 0;

        unsigned state = 0;
        for (unsigned i = 0; i < len; ++i) {
            char c = s[i];
            switch (state) {
            case 0:
                if (c == '0') {
                    state = 2;
                } else {
                    state = 1;
                    if (c >= '1' && c <= '9') {
                        int_part = c - '1' + 1;
                    } else {
                        return false;
                    }
                }
                break;
            case 1:  // 终止态
                if (c >= '0' && c <= '9') {
                    int_part = int_part * 10 + c - '0';
                } else if (c == '.') {
                    state = 3;
                } else if (c == 'e' || c == 'E') {
                    state = 4;
                } else {
                    return false;
                }
                break;
            case 2:  // 终止态
                if (c == '.') {
                    state = 3;
                } else if (c == 'e' || c == 'E') {
                    state = 4;
                } else {
                    return false;
                }
                break;
            case 3:  // 终止态，允许0.或1.这样的小数出现
                if (c >= '0' && c <= '9') {
                    frac_part = frac_part + (c - '0') * frac_pos;
                    frac_pos *= 0.1;
                } else if (c == 'e' || c == 'E') {
                    state = 4;
                }
                break;
            case 4:
                if (c == '+') {
                    state = 5;
                } else if (c == '-') {
                    neg_power = true;
                    state = 5;
                } else {
                    --i;
                    state = 5;
                }
                break;
            case 5:
                if (c == '0') {
                    state = 7;
                } else if (c >= '1' && c <= '9') {
                    state = 6;
                    power = c - '1' + 1;
                } else {
                    return false;
                }
                break;
            case 6:  // 终止态
                if (c >= '0' && c <= '9') {
                    power = power * 10 + c - '0';
                } else {
                    return false;
                }
                break;
            case 7:  // 无效状态
                return false;
            default:
                assert(false);
            }
        }

        if (!(state == 1 || state == 2 || state == 3 || state == 6)) {
            return false;
        }

        float ret = (int_part + frac_part) *
                    std::pow(10., static_cast<float>(neg_power ? -power : power));
        if (neg) {
            ret = -ret;
        }

        *reinterpret_cast<float *>(data) = ret;
        return true;
    }

    // 支持字符串格式
    static bool parse_string_callback(const char *s, unsigned len, void *data, unsigned size,
                                      void *context) {
        UNUSED(context);

        if (size < len + 1) {
            return false;
        }

        memcpy(data, s, len);
        reinterpret_cast<char *>(data)[len] = '\0';
        return true;
    }

    TableParser::TableParser(const char *src, const ColumnDescriptor desc[])
            : _src(src), _desc(desc), _line(1) {
        // std::strcpy(_err, "ok");
        std::snprintf(_err, sizeof(_err), "%s", "ok");
    }

    TableParser::TableParser(const TableParser &org) {
        _src = org._src;
        _desc = org._desc;
        _line = org._line;
        // std::strcpy(_err, org._err);
        std::snprintf(_err, sizeof(_err), "%s", org._err);
    }

    TableParser &TableParser::operator=(const TableParser &rhs) {
        _src = rhs._src;
        _desc = rhs._desc;
        _line = rhs._line;
        // std::strcpy(_err, rhs._err);
        std::snprintf(_err, sizeof(_err), "%s", rhs._err);
        return *this;
    }

    /**
     * 输入数据每行均满足
     *   line := element elements '\n' | '\n'
     *   element := int | float | array | string | struct
     *   elements := '\t' element elements | ε
     *   array := int ':' element array_elements | '0' ':'
     *   array_elements := ',' element array_elements | ε
     * 上述规则表明
     *   针对非array字段，element构成元素中不得出现'\t'
     *   针对array字段，element构成元素中不得出现','和'\t'
     */
    ParseResult TableParser::parse(void *p, unsigned size) {
        char c = *_src;
        if (c == '\0') {
            return KEOF;
        }

        unsigned idx = 0;
        ColumnDescriptor desc = _desc[idx];
        ParseResult ret = KOK;
        bool parse_end = false;
        while (!(c == '\n' || c == '\0') && !parse_end) {
            if (desc.type == KNONE) {
                break;
            }

            // 计算空间占用是否越界
            bool memory_out_boundary = false;
            if (desc.is_array) {
                if (desc.offset + desc.array_max * desc.element_size > size) {
                    memory_out_boundary = true;
                } else if (desc.array_counter_offset + sizeof(unsigned) > size) {
                    memory_out_boundary = true;
                }
            } else {
                if (desc.offset + desc.element_size > size) {
                    memory_out_boundary = true;
                }
            }

            if (memory_out_boundary) {
                ret = KERROR;
                std::snprintf(_err, sizeof(_err),
                              "line %u: element %u memory out of boundary.", _line, idx);
                parse_end = true;
            }

            // 解析一个元素
            if (!parse_end && desc.is_array) {
                char *end_of_count = nullptr;
                unsigned count = std::strtoul(_src, &end_of_count, 10);

                if (end_of_count == _src) {
                    ret = KERROR;
                    std::snprintf(_err, sizeof(_err),
                                  "[ERROR] line %u: array size required near element %u.", _line,
                                  idx);
                    parse_end = true;
                } else if (*end_of_count != ':') {
                    ret = KERROR;
                    std::snprintf(_err, sizeof(_err),
                                  "[ERROR] line %u: unexpected character near element %u.", _line, idx);
                    parse_end = true;
                }

                if (count > desc.array_max) {
                    ret = KERROR;
                    std::snprintf(_err, sizeof(_err),
                                  "[ERROR] line %u: array size out of range near element %u.", _line, idx);
                    parse_end = true;
                }

                if (!parse_end) {
                    // 设置数组大小描述内存
                    *reinterpret_cast<unsigned *>((void *) ((char *) p + desc.array_counter_offset))
                            = count;

                    // 依次解析数组元素
                    _src = end_of_count + 1;
                    for (unsigned i = 0; !parse_end && i < count; ++i) {
                        const char *start = _src;
                        const char *end = start;

                        while (!(*end == '\0' || *end == ',' || *end == '\t' || *end == '\n')) {
                            ++end;
                        }

                        if (i != count - 1) {
                            switch (*end) {
                            case '\t':
                                ret = KERROR;
                                std::snprintf(_err, sizeof(_err),
                                              "[ERROR] line %u: unexpected column "
                                                      "splitter near element %u.",
                                              _line, idx);
                                parse_end = true;
                                break;
                            case '\0':
                                ret = KERROR;
                                std::snprintf(_err, sizeof(_err),
                                              "[ERROR] line %u: unexpected eof near element %u.", _line,
                                              idx);
                                parse_end = true;
                                break;
                            case '\n':
                                ret = KERROR;
                                std::snprintf(_err, sizeof(_err),
                                              "[ERROR] line %u: unexpected new line near element %u.",
                                              _line, idx);
                                parse_end = true;
                                break;
                            default:
                                assert(*end == ',');
                                break;
                            }
                        }

                        if (*end == ',') {
                            _src = end + 1;

                            if (i == count - 1) {
                                ret = KERROR;
                                std::snprintf(_err, sizeof(_err),
                                              "[ERROR] line %u: more array element found near element %u.",
                                              _line, idx);
                                parse_end = true;
                            }
                        }
                        else if (*end == '\t') {
                            _src = end + 1;
                        }
                        else {
                            _src = end;
                        }

                        ret = parse_element(idx, start, end - start,
                                           (void *) ((char *) p + desc.offset +
                                                     desc.element_size * i));
                        if (ret != KOK) {
                            parse_end = true;
                        }
                    }
                }
            } else if (!parse_end) {
                const char *start = _src;
                const char *end = start;

                while (!(*end == '\0' || *end == '\t' || *end == '\n')) {
                    ++end;
                }

                if (*end == '\t') {
                    _src = end + 1;
                } else {
                    _src = end;
                }

                ret = parse_element(idx, start, end - start, (void *) ((char *) p + desc.offset));
                if (ret != KOK) {
                    parse_end = true;
                }
            }
            if (!parse_end) {
                desc = _desc[++idx];
                c = *_src;
            }
        }

        // PARSE_END:
        c = *_src;
        if (ret == KOK) {
            if ((c == '\n' || c == '\0') && desc.type != KNONE) {
                // 输入行已读完，但是元素没有全部被解析
                std::snprintf(_err, sizeof(_err),
                              "[ERROR] line %u: element %u required in input.", _line, idx);
                ret = KERROR;
            } else if (!(c == '\n' || c == '\0') && desc.type == KNONE) {
                // 输入行未读完，但是元素全部被解析
                std::snprintf(_err, sizeof(_err),
                              "[ERROR] line %u: more element found in input after element %u.",
                              _line, idx);
                ret = KERROR;
            } else {
                assert((c == '\n' || c == '\0') && desc.type == KNONE);
            }
        } else {
            assert(ret == KERROR);
        }

        if (ret == KOK) {
            std::snprintf(_err, sizeof(_err), "[OK] line %u: parse success", _line);
        }

        while (!(c == '\n' || c == '\0')) {
            c = *(++_src);
        }
        if (c == '\n') {
            ++_line;
            ++_src;
        }

        return ret;
    }

    const char *TableParser::last_error() const {
        return _err;
    }

    ParseResult TableParser::parse_element(unsigned idx, const char *s, unsigned len, void *data) {
        const ColumnDescriptor &desc = _desc[idx];
        parser_callback cb;

        switch (desc.type) {
        case KINT:
            cb = parse_int_callback;
            break;
        case KFLOAT:
            cb = parse_float_callback;
            break;
        case KSTRING:
            cb = parse_string_callback;
            break;
        case KCLASS:
            if (!desc.callback) {
                std::snprintf(_err, sizeof(_err),
                              "[ERROR] line %u: user-defined callback required at element %u.",
                              _line, idx);
                return KERROR;
            }
            cb = desc.callback;
            break;
        default:
            std::snprintf(_err, sizeof(_err),
                          "[ERROR] line %u: unknown element type at element %u.", _line, idx);
            return KERROR;
        }

        if (cb(s, len, data, desc.element_size, desc.context)) {
            return KOK;
        }

        std::snprintf(_err, sizeof(_err),
                      "[ERROR] line %u: element %u parse failed.", _line, idx);
        return KERROR;
    }
}
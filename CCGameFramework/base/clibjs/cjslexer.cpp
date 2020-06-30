//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include <array>
#include <tuple>
#include <cmath>
#include <sstream>
#include <codecvt>
#include <locale>
#include <cassert>
#include "cjslexer.h"

namespace clib {

    using namespace types;

    cjslexer::cjslexer() {
        std::array<std::tuple<js_lexer_t, std::string>, KEYWORD_END - KEYWORD_START + 1> keyword_string_list = {
                std::make_tuple(K_NEW, "new"),
                std::make_tuple(K_VAR, "var"),
                std::make_tuple(K_LET, "let"),
                std::make_tuple(K_FUNCTION, "function"),
                std::make_tuple(K_IF, "if"),
                std::make_tuple(K_ELSE, "else"),
                std::make_tuple(K_FOR, "for"),
                std::make_tuple(K_WHILE, "while"),
                std::make_tuple(K_IN, "in"),
                std::make_tuple(K_DO, "do"),
                std::make_tuple(K_BREAK, "break"),
                std::make_tuple(K_CONTINUE, "continue"),
                std::make_tuple(K_RETURN, "return"),
                std::make_tuple(K_SWITCH, "switch"),
                std::make_tuple(K_DEFAULT, "default"),
                std::make_tuple(K_CASE, "case"),
                std::make_tuple(K_NULL, "null"),
                std::make_tuple(K_UNDEFINED, "undefined"),
                std::make_tuple(K_TRUE, "true"),
                std::make_tuple(K_FALSE, "false"),
                std::make_tuple(K_INSTANCEOF, "instanceof"),
                std::make_tuple(K_TYPEOF, "typeof"),
                std::make_tuple(K_VOID, "void"),
                std::make_tuple(K_DELETE, "delete"),
                std::make_tuple(K_CLASS, "class"),
                std::make_tuple(K_THIS, "this"),
                std::make_tuple(K_SUPER, "super"),
                std::make_tuple(K_WITH, "with"),
                std::make_tuple(K_TRY, "try"),
                std::make_tuple(K_THROW, "throw"),
                std::make_tuple(K_CATCH, "catch"),
                std::make_tuple(K_FINALLY, "finally"),
                std::make_tuple(K_DEBUGGER, "debugger"),
        };
        auto key_N = KEYWORD_END - KEYWORD_START - 1;
        for (auto i = 0; i < key_N; i++) {
            const auto &k = keyword_string_list[i];
            mapKeyword.insert({std::string(std::get<1>(k)), std::get<0>(k)});
        }
    }

    // 十六进制字符转十进制
    static int hex2dec(char c) {
        if (c >= '0' && c <= '9') {
            return c - '0';
        } else if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        } else {
            return -1;
        }
    }

    // 八进制字符转十进制
    static int oct2dec(char c) {
        if (c >= '0' && c <= '7') {
            return c - '0';
        } else {
            return -1;
        }
    }

    // 二进制字符转十进制
    static int bin2dec(char c) {
        if (c == '0') {
            return 0;
        } else if (c == '1') {
            return 1;
        } else {
            return -1;
        }
    }

    // 单字符转义
    static int escape(char c) {
        if (c >= '0' && c <= '9') {
            return c - '0';
        } else {
            switch (c) { // like \r, \n, ...
                case 'b':
                    return '\b';
                case 'f':
                    return '\f';
                case 'n':
                    return '\n';
                case 'r':
                    return '\r';
                case 't':
                    return '\t';
                case 'v':
                    return '\v';
                case '\'':
                    return '\'';
                case '\"':
                    return '\"';
                case '\\':
                    return '\\';
                default:
                    return -1;
            }
        }
    }

    void cjslexer::input(const std::string &s, std::string &error_string) {
        decltype(units) us;
        text = s;
        auto len = (int) text.length();
        text.push_back(0);
        text.push_back(0);
        text.push_back(0); // 防止溢出
        auto i = 0;
        auto line = 1;
        auto column = 1;
#if 0
        auto ii = 0;
        auto iL = 0;
        auto iC = 0;
#endif
        std::vector<char> buf(256);
        for (i = 0; i < len;) {
            auto c = text[i];
#if 0
            if (ii > 0)
                fprintf(stdout, "P [%04d-%04d] Line: %04d, Column: %03d '%s'\n", ii, i, iL, iC,
                        text.substr((size_t) ii, (size_t) (i - ii)).c_str());
            ii = i;
            iL = line;
            iC = column;
#endif
            if (isalpha(c) || c == '_' || c == '$') { // 变量名或关键字
                auto j = 0;
                for (j = i + 1; isalnum(text[j]) || text[j] == '_' || text[j] == '$'; j++);
                auto keyword = text.substr((size_t) i, (size_t) (j - i));
                auto kw = mapKeyword.find(keyword);
                auto u = alloc_unit(line, column, i, j);
                if (kw != mapKeyword.end()) { // 哈希查找关键字
                    u.t = kw->second;
                } else { // 变量名
                    u.t = ID;
                    u.len = j - i;
                    u.idx = alloc(u.len + 1);
                    std::copy(text.begin() + i, text.begin() + j, data.begin() + u.idx);
                    data[u.idx + u.len] = 0;
                }
                us.push_back(u);
                column += j - i;
                i = j;
                continue;
            } else if (isdigit(c) || (c == '.' && isdigit(text[i + 1])) ||
                       (c == '-' && (isdigit(text[i + 1]) || text[i + 1] == '.'))) { // 数字
                // 判断是否可以负数
                if (c != '-' || allow_expr(us)) {
                    // 假定这里的数字规则是以0-9开头
                    // 正则：^((?:\d+(\.)?\d*)(?:[eE][+-]?\d+)?)$
                    // 正则：^0[Xx][0-9A-Fa-f]+$
                    // 其他功能：e科学记数法
                    // 注意：这里不考虑负数，因为估计到歧义（可能是减法呢？）
                    bool neg = false;
                    auto d = 0.0;
                    auto c1 = text[i + 1];
                    auto j = i;
                    if (c == '-') {
                        c = c1;
                        c1 = text[i + 2];
                        j++;
                        neg = true;
                    }
                    if (c == '0') {
                        if (c1 == 'x' || c1 == 'X') {
                            auto cc = 0;
                            // 十六进制
                            for (j = i + 2; (cc = hex2dec(text[j])) != -1; j++) {
                                d *= 16.0;
                                d += cc;
                            }
                            if (j == i + 2) {
                                snprintf(buf.data(), buf.size(), "Line: %d, Column: %d, Error: invalid number '%.2s'", line, column,
                                         &text[i]);
                                break;
                            }
                            auto u = alloc_unit(line, column, i, j);
                            u.t = NUMBER;
                            u.len = sizeof(d);
                            u.idx = alloc(u.len);
                            *((double *) &data[u.idx]) = d;
                            us.push_back(u);
                            column += j - i;
                            i = j;
                            continue;
                        } else if (c1 == 'b' || c1 == 'B') {
                            auto cc = 0;
                            // 二进制
                            for (j = i + 2; (cc = bin2dec(text[j])) != -1; j++) {
                                d *= 2;
                                d += cc;
                            }
                            if (j == i + 2) {
                                snprintf(buf.data(), buf.size(), "Line: %d, Column: %d, Error: invalid number '%.2s'", line, column,
                                         &text[i]);
                                break;
                            }
                            auto u = alloc_unit(line, column, i, j);
                            u.t = NUMBER;
                            u.len = sizeof(d);
                            u.idx = alloc(u.len);
                            *((double *) &data[u.idx]) = d;
                            us.push_back(u);
                            column += j - i;
                            i = j;
                            continue;
                        } else {
                            j = i + 1;
                            if (c1 == 'o' || c1 == 'O') { // 八进制
                                c1 = text[j];
                                j++;
                            }
                            if (c1 >= '0' && c1 <= '7') { // 八进制
                                auto cc = 0;
                                // 八进制
                                for (; (cc = oct2dec(text[j])) != -1; j++) {
                                    d *= 8.0;
                                    d += cc;
                                }
                                if (!(oct2dec(text[j]) && isdigit(text[j]))) {
                                    if (j == i + 2) {
                                        snprintf(buf.data(), buf.size(), "Line: %d, Column: %d, Error: invalid number '%.2s'\n", line,
                                                 column,
                                                 &text[i]);
                                        break;
                                    }
                                    auto u = alloc_unit(line, column, i, j);
                                    u.t = NUMBER;
                                    u.len = sizeof(d);
                                    u.idx = alloc(u.len);
                                    *((double *) &data[u.idx]) = d;
                                    us.push_back(u);
                                    column += j - i;
                                    i = j;
                                    continue;
                                } else {
                                    d = 0.0; // 0778 失败
                                }
                            }
                        }
                    }
                    // 判断整数部分
                    for (; isdigit(text[j]); j++) { // 解析整数部分
                        d *= 10.0;
                        d += text[j] - '0';
                    }
                    if (text[j] == '.') { // 解析小数部分
                        auto l = ++j;
                        for (; isdigit(text[j]); j++) {
                            d *= 10.0;
                            d += text[j] - '0';
                        }
                        l = j - l;
                        if (l > 0) {
                            d *= pow(10.0, -l);
                        }
                    }
                    if (text[j] == 'e' || text[j] == 'E') { // 科学计数法
                        auto ne = false;
                        auto e = 0.0;
                        if (!isdigit(text[++j])) {
                            if (text[j] == '-') { // 1e-1
                                ne = true;
                                j++;
                            } else if (text[j] == '+') {
                                j++;
                            } else if (!isdigit(text[j])) { // 1e+1
                                snprintf(buf.data(), buf.size(), "Line: %d, Column: %d, Error: invalid number '%s'", line, column,
                                         text.substr((size_t) i, (size_t) (j - i)).c_str());
                                break;
                            }
                        }
                        auto l = j;
                        for (; isdigit(text[j]); j++) { // 解析指数部分
                            e *= 10;
                            e += text[j] - '0';
                        }
                        if (l == j) {
                            snprintf(buf.data(), buf.size(), "Line: %d, Column: %d, Error: invalid number '%s'", line, column,
                                     text.substr((size_t) i, (size_t) (j - i)).c_str());
                            break;
                        }
                        d *= pow(10.0, ne ? -e : e);
                    }
                    auto u = alloc_unit(line, column, i, j);
                    u.t = NUMBER;
                    u.len = sizeof(d);
                    u.idx = alloc(u.len);
                    *((double *) &data[u.idx]) = neg ? -d : d;
                    us.push_back(u);
                    column += j - i;
                    i = j;
                    continue;
                }
            } else if (isspace(c)) { // 空白字符
                auto j = 0;
                if (c == ' ' || c == '\t') {
                    // 查找连续的空格或Tab
                    for (j = i + 1; text[j] == ' ' || text[j] == '\t'; j++);
                    auto u = alloc_unit(line, column, i, j);
                    u.t = SPACE;
                    us.push_back(u);
                    column += j - i;
                    i = j;
                    continue;
                } else if (c == '\r' || c == '\n') {
                    // 查找连续的'\n'或'\r\n'
                    auto l = 0;
                    for (j = i; j < len;) {
                        if (text[j] == '\r') {
                            if (text[j + 1] == '\n') {
                                l++;
                                j += 2;
                            } else {
                                l++;
                                j++;
                            }
                        } else if (text[j] == '\n') {
                            l++;
                            j++;
                        } else {
                            break;
                        }
                    }
                    auto u = alloc_unit(line, column, i, j);
                    u.t = NEWLINE;
                    us.push_back(u);
                    line += l;
                    column = 1;
                    i = j;
                    continue;
                } else {
                    snprintf(buf.data(), buf.size(), "Line: %d, Column: %d, Error: invalid space", line, column);
                    break;
                }
            } else if (c == '\'' || c == '\"') { // 字符串
                auto j = i;
                // 寻找非'\"'的第一个'"'
                for (j++; j < len; j++) {
                    if (text[j] == '\\') {
                        if (text[j + 1] == '\\' || text[j + 1] == c) {
                            j++;
                            if (text[j + 1] == c) {
                                j++;
                                break;
                            }
                            continue;
                        }
                        continue;
                    }
                    if (text[j] == c)break;
                }
                auto k = j;
                if (k == len) { // " EOF
                    snprintf(buf.data(), buf.size(), "Line: %d, Column: %d, Error: invalid string, missing %c", line, column, c);
                    break;
                }
                std::stringstream ss;
                ss << c;
                auto status = 1; // 状态机
                auto count = 0;
                auto digit = 0;
                uint32_t cc = 0;
                auto err = false;
                for (j = i + 1; j <= k;) {
                    switch (status) {
                        case 1: { // 处理字符
                            if (text[j] == '\\') {
                                status = 2;
                            } else { // '?'
                                ss << text[j];
                            }
                            j++;
                        }
                            break;
                        case 2: { // 处理转义
                            if (text[j] == 'x') {
                                status = 3;
                                j++;
                                count = 2;
                                digit = 16;
                            } else if (text[j] == 'u') {
                                status = 3;
                                j++;
                                count = 4;
                                digit = 16;
                            } else {
                                auto esc = escape(text[j]);
                                if (esc != -1) {
                                    if (esc >= 0 && esc < 8) { //八进制
                                        status = 3;
                                        count = 3;
                                        digit = 8;
                                    } else {
                                        ss << (char) esc;
                                        j++;
                                        status = 1;
                                    }
                                } else {
                                    status = 0; // 失败
                                }
                            }
                        }
                            break;
                        case 3: { // 处理 '\x??'，'\111' 或 '\u????' 前一位十六进制数字
                            auto esc = digit == 16 ? hex2dec(text[j]) : oct2dec(text[j]);
                            if (esc != -1) {
                                cc = (uint32_t) esc;
                                status = 4;
                                j++;
                                count--;
                            } else {
                                status = 0; // 失败
                            }
                        }
                            break;
                        case 4: { // 处理 '\x??'，'\111' 或 '\u????' 后面的十六进制数字
                            auto esc = digit == 16 ? hex2dec(text[j]) : oct2dec(text[j]);
                            auto fin = false;
                            if (esc != -1) {
                                if (digit == 16) {
                                    cc *= 16;
                                    cc += (uint32_t) esc;
                                    j++;
                                    count--;
                                    if (count == 0) {
                                        fin = true;
                                    }
                                } else {
                                    auto oldc = cc;
                                    cc *= 8;
                                    cc += (uint32_t) esc;
                                    if (cc > 0x100) {
                                        cc = oldc;
                                        fin = true;
                                    }
                                    if (!fin) {
                                        j++;
                                        count--;
                                        if (count == 0) {
                                            fin = true;
                                        }
                                    }
                                }
                            } else {
                                fin = true;
                            }
                            if (fin) {
                                status = 1;
                                if (cc < 0x100) {
                                    ss << (char) cc;
                                } else {
                                    std::u32string w;
                                    w.push_back((char32_t) cc);
                                    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
                                    try {
                                        auto ret = cv.to_bytes(w);
                                        ss << ret;
                                    } catch (const std::exception &e) {
                                        snprintf(buf.data(), buf.size(), "Line: %d, Column: %d, Error: '%s' during conversion '%s'\n",
                                                 line,
                                                 column,
                                                 e.what(), text.substr((size_t) i, (size_t) (j - i)).c_str());
                                        err = true;
                                    }
                                }
                            }
                        }
                            break;
                        default: // 失败
                            snprintf(buf.data(), buf.size(), "Line: %d, Column: %d, Error: invalid string '%s'", line, column,
                                     text.substr((size_t) i, (size_t) (j - i)).c_str());
                            err = true;
                            break;
                    }
                    if (err)break;
                }
                if (err)break;
                if (status == 1) { // 为初态/终态
                    j = k + 1;
                    auto st = ss.str();
                    auto u = alloc_unit(line, column, i, j);
                    u.t = STRING;
                    u.len = (int) st.length();
                    u.idx = alloc(u.len + 1);
                    std::copy(st.begin(), st.end(), data.begin() + u.idx);
                    data[u.idx + u.len] = 0;
                    us.push_back(u);
                    column += j - i;
                    i = j;
                    continue;
                } else {
                    snprintf(buf.data(), buf.size(), "Line: %d, Column: %d, Error: invalid string", line, column);
                    break;
                }
            } else if (c == '/') { // 注释
                auto c1 = text[i + 1];
                if (c1 == '/' || c1 == '*') { // 注释
                    auto j = i + 1;
                    if (c1 == '/') { // '//'
                        // 寻找第一个换行符
                        for (++j; text[j] != '\n' && text[j] != '\r' && text[j] != '\0'; j++);
                        auto u = alloc_unit(line, column, i, j);
                        u.t = COMMENT;
                        us.push_back(u);
                        column += j - i;
                        i = j;
                    } else { // '/*  */'
                        // 寻找第一个 '*/'
                        char prev = 0;
                        auto col = column;
                        auto newline = 0;
                        for (++j;;) {
                            if (prev == '*' && text[j] == '/')
                                break;
                            if (text[j] == '\0')
                                break;
                            prev = text[j++];
                            if (prev == '\n') {
                                ++newline;
                                col = 1;
                            } else {
                                col++;
                            }
                        }
                        j++;
                        auto u = alloc_unit(line, column, i, j);
                        u.t = COMMENT;
                        us.push_back(u);
                        column += j - i;
                        line += newline;
                        i = j;
                    }
                    continue;
                } else {
                    // 判断正则表达式？
                    // 寻找满足的条件，如四则、return之后
                    if (allow_expr(us)) {
                        auto j = i;
                        auto squ = false;
                        for (++j; j < len; j++) {
                            if (text[j] == '\\') {
                                j++;
                                continue;
                            }
                            if (text[j] == '/' && !squ)break;
                            if (text[j] == '[') squ = true;
                            else if (text[j] == ']') squ = false;
                        }
                        j++;
                        // postfix: /i /g /gi /ig /m
                        for (auto k = 0; k < 3; k++) {
                            if (text[j] == 'g' || text[j] == 'i' || text[j] == 'm') {
                                j++;
                            }
                        }
                        auto u = alloc_unit(line, column, i, j);
                        u.t = REGEX;
                        auto re = text.substr((size_t) i, (size_t) (j - i));
                        u.len = (int) re.length();
                        u.idx = alloc(u.len + 1);
                        std::copy(re.begin(), re.end(), data.begin() + u.idx);
                        data[u.idx + u.len] = 0;
                        us.push_back(u);
                        column += j - i;
                        i = j;
                        continue;
                    }
                }
            }
            {
                auto T = NONE;
                auto j = i;
                auto c1 = text[i + 1];
                auto c2 = text[i + 2];
                switch (c) {
                    case '=':
                        if (c1 == '=') {
                            if (c2 == '=') {
                                T = T_FEQUAL;
                                j += 3;
                            } else {
                                T = T_EQUAL;
                                j += 2;
                            }
                        } else if (c1 == '>') {
                            T = T_ARROW;
                            j += 2;
                        } else {
                            T = T_ASSIGN;
                            j++;
                        }
                        break;
                    case '>':
                        if (c1 == '>') {
                            if (c2 == '>') {
                                auto c3 = text[i + 3];
                                if (c3 == '=') {
                                    T = T_ASSIGN_URSHIFT;
                                    j += 4;
                                } else {
                                    T = T_URSHIFT;
                                    j += 3;
                                }
                            } else if (c2 == '=') {
                                T = T_ASSIGN_RSHIFT;
                                j += 3;
                            } else {
                                T = T_RSHIFT;
                                j += 2;
                            }
                        } else if (c1 == '=') {
                            T = T_GREATER_EQUAL;
                            j += 2;
                        } else {
                            T = T_GREATER;
                            j++;
                        }
                        break;
                    case '<':
                        if (c1 == '<') {
                            if (c2 == '=') {
                                T = T_ASSIGN_LSHIFT;
                                j += 3;
                            } else {
                                T = T_LSHIFT;
                                j += 2;
                            }
                        } else if (c1 == '=') {
                            T = T_LESS_EQUAL;
                            j += 2;
                        } else {
                            T = T_LESS;
                            j++;
                        }
                        break;
                    case '+':
                        if (c1 == '=') {
                            T = T_ASSIGN_ADD;
                            j += 2;
                        } else if (c1 == '+') {
                            T = T_INC;
                            j += 2;
                        } else {
                            T = T_ADD;
                            j++;
                        }
                        break;
                    case '-':
                        if (c1 == '=') {
                            T = T_ASSIGN_SUB;
                            j += 2;
                        } else if (c1 == '-') {
                            T = T_DEC;
                            j += 2;
                        } else {
                            T = T_SUB;
                            j++;
                        }
                        break;
                    case '*':
                        if (c1 == '=') {
                            T = T_ASSIGN_MUL;
                            j += 2;
                        } else if (c1 == '*') {
                            if (c2 == '=') {
                                T = T_ASSIGN_POWER;
                                j += 3;
                            } else {
                                T = T_POWER;
                                j += 2;
                            }
                        } else {
                            T = T_MUL;
                            j++;
                        }
                        break;
                    case '/':
                        if (c1 == '=') {
                            T = T_ASSIGN_DIV;
                            j += 2;
                        } else {
                            T = T_DIV;
                            j++;
                        }
                        break;
                    case '%':
                        if (c1 == '=') {
                            T = T_ASSIGN_MOD;
                            j += 2;
                        } else {
                            T = T_MOD;
                            j++;
                        }
                        break;
                    case '^': {
                        if (c1 == '=') {
                            T = T_ASSIGN_XOR;
                            j += 2;
                        } else {
                            T = T_BIT_XOR;
                            j++;
                        }
                    }
                        break;
                    case '~': {
                        T = T_BIT_NOT;
                        j++;
                    }
                        break;
                    case '!':
                        if (c1 == '=') {
                            if (c2 == '=') {
                                T = T_FNOT_EQUAL;
                                j += 3;
                            } else {
                                T = T_NOT_EQUAL;
                                j += 2;
                            }
                        } else {
                            T = T_LOG_NOT;
                            j++;
                        }
                        break;
                    case '&':
                        if (c1 == '&') {
                            T = T_LOG_AND;
                            j += 2;
                        } else if (c1 == '=') {
                            T = T_ASSIGN_AND;
                            j += 2;
                        } else {
                            T = T_BIT_AND;
                            j++;
                        }
                        break;
                    case '|':
                        if (c1 == '|') {
                            T = T_LOG_OR;
                            j += 2;
                        } else if (c1 == '=') {
                            T = T_ASSIGN_OR;
                            j += 2;
                        } else {
                            T = T_BIT_OR;
                            j++;
                        }
                        break;
                    case '.': {
                        if (c1 == '.' && c2 == '.') {
                            T = T_ELLIPSIS;
                            j += 3;
                        } else {
                            T = T_DOT;
                            j++;
                        }
                    }
                        break;
                    case ',': {
                        T = T_COMMA;
                        j++;
                    }
                        break;
                    case ';': {
                        T = T_SEMI;
                        j++;
                    }
                        break;
                    case ':': {
                        T = T_COLON;
                        j++;
                    }
                        break;
                    case '?': {
                        if (c1 == '?') {
                            T = T_COALESCE;
                            j += 2;
                        } else {
                            T = T_QUERY;
                            j++;
                        }
                    }
                        break;
                    case '(': {
                        T = T_LPARAN;
                        j++;
                    }
                        break;
                    case ')': {
                        T = T_RPARAN;
                        j++;
                    }
                        break;
                    case '[': {
                        T = T_LSQUARE;
                        j++;
                    }
                        break;
                    case ']': {
                        T = T_RSQUARE;
                        j++;
                    }
                        break;
                    case '{': {
                        T = T_LBRACE;
                        j++;
                    }
                        break;
                    case '}': {
                        T = T_RBRACE;
                        j++;
                    }
                        break;
                    case '#': {
                        T = T_SHARP;
                        j++;
                    }
                        break;
                    default:
                        break;
                }
                if (T != NONE) {
                    auto u = alloc_unit(line, column, i, j);
                    u.t = T;
                    us.push_back(u);
                    column += j - i;
                    i = j;
                    continue;
                } else {
                    snprintf(buf.data(), buf.size(), "Line: %d, Column: %d, Error: invalid operator '%c'", line, column, c);
                    break;
                }
            }
        }
        {
            std::string _error_string = buf.data();
            if (!_error_string.empty()) {
                error_string = _error_string;
                return;
            }
            auto id = 0;
            auto no_l = true;
            for (auto &U : us) {
                switch (U.t) {
                    case NEWLINE:
                        no_l = false;
                    case SPACE:
                    case COMMENT:
                        break;
                    default:
                        U.id = id++;
                        units.push_back(U);
                        no_line.push_back(no_l);
                        if (!no_l)no_l = true;
                        break;
                }
            }
            auto u = alloc_unit(line, column, i, i);
            u.t = END;
            u.id = id;
            units.push_back(u);
            no_line.push_back(false);
        }
    }

#define js_mem_align(d, a) (((d) + (a - 1)) & ~(a - 1))

    int cjslexer::alloc(int size) {
        auto idx = (int) data.size();
        data.resize(data.size() + js_mem_align(size, 4));
        return idx;
    }

#undef js_mem_align

    bool cjslexer::allow_expr(const std::vector<lexer_unit> &u) {
        if (u.empty())
            return true;
        bool can = false;
        for (auto U = u.rbegin(); U != u.rend(); U++) {
            if (U->t == NEWLINE || U->t == COMMENT || U->t == SPACE)
                continue;
            switch (U->t) {
                case K_RETURN:
                case K_CASE:
                case T_ADD:
                case T_SUB:
                case T_MUL:
                case T_DIV:
                case T_MOD:
                case T_POWER:
                case T_INC:
                case T_DEC:
                case T_ASSIGN:
                case T_ASSIGN_ADD:
                case T_ASSIGN_SUB:
                case T_ASSIGN_MUL:
                case T_ASSIGN_DIV:
                case T_ASSIGN_MOD:
                case T_ASSIGN_LSHIFT:
                case T_ASSIGN_RSHIFT:
                case T_ASSIGN_URSHIFT:
                case T_ASSIGN_AND:
                case T_ASSIGN_OR:
                case T_ASSIGN_XOR:
                case T_ASSIGN_POWER:
                case T_LESS:
                case T_LESS_EQUAL:
                case T_GREATER:
                case T_GREATER_EQUAL:
                case T_EQUAL:
                case T_FEQUAL:
                case T_NOT_EQUAL:
                case T_FNOT_EQUAL:
                case T_LOG_NOT:
                case T_LOG_AND:
                case T_LOG_OR:
                case T_BIT_NOT:
                case T_BIT_AND:
                case T_BIT_OR:
                case T_BIT_XOR:
                case T_COMMA:
                case T_SEMI:
                case T_COLON:
                case T_QUERY:
                case T_LSHIFT:
                case T_RSHIFT:
                case T_URSHIFT:
                case T_LPARAN:
                case T_LSQUARE:
                case T_LBRACE:
                    can = true;
                    break;
                default:
                    break;
            }
            break;
        }
        return can;
    }

    lexer_unit cjslexer::alloc_unit(int line, int column, int start, int end) {
        lexer_unit u;
        u.line = line;
        u.column = column;
        u.start = start;
        u.end = end;
        return u;
    }

    void cjslexer::dump() const {
        for (size_t i = 0; i < units.size(); i++) {
            fprintf(stdout, "%s\n", get_unit_desc((int) i).c_str());
        }
    }

    const lexer_unit &cjslexer::get_unit(int idx) const {
        if (idx < 0 || idx >= (int) units.size()) {
            return units.back();
        }
        return units[idx];
    }

    const char *cjslexer::get_data(int idx) const {
        if (idx < 0 || idx >= (int) data.size()) {
            return nullptr;
        }
        return &data[idx];
    }

    bool cjslexer::valid_rule(int idx, js_lexer_t rule) const {
        if (idx < 0 || idx >= (int) units.size()) {
            return false;
        }
        switch (rule) {
            case RULE_NO_LINE:
                return no_line[idx];
            case RULE_LINE:
                return !no_line[idx];
            case RULE_RBRACE:
                return units[idx].t == T_RBRACE;
            case RULE_EOF:
                return idx + 1 == (int) units.size();
            default:
                break;
        }
        assert(!"invalid rule type");
        return false;
    }

    int cjslexer::get_unit_size() const {
        return max((int) (units.size()) - 1, 0);
    }

    std::string cjslexer::get_unit_desc(int idx) const {
        auto U = get_unit(idx);
        auto type = "";
        auto isprint = true;
        if (U.t == ID) {
            type = "ID";
        } else if (U.t == NUMBER) {
            type = "NUMBER";
        } else if (U.t > KEYWORD_START && U.t < KEYWORD_END) {
            type = "KEYWORD";
        } else if (U.t > OPERATOR_START && U.t < OPERATOR_END) {
            type = "OPERATOR";
        } else if (U.t == STRING) {
            type = "STRING";
        } else if (U.t == SPACE) {
            type = "SPACE";
            isprint = false;
        } else if (U.t == NEWLINE) {
            type = "NEWLINE";
            isprint = false;
        } else if (U.t == COMMENT) {
            type = "COMMENT";
        } else if (U.t == REGEX) {
            type = "REGEX";
        } else if (U.t == END) {
            type = "END";
            isprint = false;
        }
        char buf[256];
        snprintf(buf, sizeof(buf), "D [%04d-%04d] Line: %04d, Column: %03d |%-10s| %.20s",
                 U.start, U.end, U.line, U.column,
                 type, isprint ? text.substr((size_t) U.start, (size_t) (U.end - U.start)).c_str() : "");
        return buf;
    }

    int cjslexer::get_index() const {
        return index;
    }

    void cjslexer::inc_index() {
        index++;
    }

    const lexer_unit &cjslexer::get_current_unit() const {
        return get_unit(index);
    }
}
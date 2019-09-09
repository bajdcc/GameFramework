//
// Project: CMiniLang
// Author: bajdcc
//

#ifndef CMINILANG_TYPES_H
#define CMINILANG_TYPES_H

#include <string>
#include <unordered_map>

using string_t = std::string;
template<class K, class V> using map_t = std::unordered_map<K, V>;

namespace clib {
#if __APPLE__ && __MACH__
    using int8 = int8_t;
    using uint8 = uint8_t;
    using int16 = int16_t;
    using uint16 = uint16_t;
    using int32 = int32_t;
    using uint32 = uint32_t;
    using int64 = int64_t;
    using uint64 = uint64_t;
#else
    using int8 = signed __int8;
    using uint8 = unsigned __int8;
    using int16 = signed __int16;
    using uint16 = unsigned __int16;
    using int32 = signed __int32;
    using uint32 = unsigned __int32;
    using int64 = signed __int64;
    using uint64 = unsigned __int64;
#endif

    using sint = signed int;
    using uint = unsigned int;
    using slong = long long;
    using ulong = unsigned long long;

    using byte = uint8;
    using decimal = double; // 浮点类型

    enum lexer_t {
        l_none,
        l_error,
        l_char,
        l_uchar,
        l_short,
        l_ushort,
        l_int,
        l_uint,
        l_long,
        l_ulong,
        l_float,
        l_double,
        l_operator,
        l_keyword,
        l_identifier,
        l_string,
        l_comment,
        l_space,
        l_newline,
        l_end,
    };

    enum keyword_t {
        k__start,
        k_auto,
        k_bool,
        k_break,
        k_case,
        k_char,
        k_const,
        k_continue,
        k_default,
        k_do,
        k_double,
        k_else,
        k_enum,
        k_extern,
        k_false,
        k_float,
        k_for,
        k_goto,
        k_if,
        k_int,
        k_long,
        k_register,
        k_return,
        k_short,
        k_signed,
        k_sizeof,
        k_static,
        k_struct,
        k_switch,
        k_true,
        k_typedef,
        k_union,
        k_unsigned,
        k_void,
        k_volatile,
        k_while,
        k_interrupt,
        k_pragma,
        k__end
    };

    enum operator_t {
        op__start,
        op_assign,
        op_equal,
        op_plus,
        op_plus_assign,
        op_minus,
        op_minus_assign,
        op_times,
        op_times_assign,
        op_divide,
        op_div_assign,
        op_bit_and,
        op_and_assign,
        op_bit_or,
        op_or_assign,
        op_bit_xor,
        op_xor_assign,
        op_mod,
        op_mod_assign,
        op_less_than,
        op_less_than_or_equal,
        op_greater_than,
        op_greater_than_or_equal,
        op_logical_not,
        op_not_equal,
        op_escape,
        op_query,
        op_bit_not,
        op_lparan,
        op_rparan,
        op_lbrace,
        op_rbrace,
        op_lsquare,
        op_rsquare,
        op_comma,
        op_dot,
        op_semi,
        op_colon,
        op_plus_plus,
        op_minus_minus,
        op_logical_and,
        op_logical_or,
        op_pointer,
        op_left_shift,
        op_right_shift,
        op_left_shift_assign,
        op_right_shift_assign,
        op_ellipsis,
        op__end,
    };

    enum error_t {
        e__start,
        e_invalid_char,
        e_invalid_operator,
        e_invalid_digit,
        e_invalid_string,
        e__end
    };

    enum ins_t {
        NOP, LEA, IMM, IMX, JMP, JZ, JNZ, ENT, LOAD, SAVE, INTR, CAST, ADJ, CALL, LEV,
        PUSH, POP, OR, XOR, AND, EQ, CASE, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD, NEG, NOT, LNT,
        EXIT,
    };

    template<lexer_t>
    struct base_t {
        using type = void*;
    };
    template<class T>
    struct base_lexer_t {
        static const lexer_t type = l_none;
    };

#define DEFINE_BASE_TYPE(t, obj) \
template<> \
struct base_t<t> \
{ \
    using type = obj; \
    static const int size = sizeof(obj); \
};

    DEFINE_BASE_TYPE(l_char, char)
        DEFINE_BASE_TYPE(l_uchar, unsigned char)
        DEFINE_BASE_TYPE(l_short, short)
        DEFINE_BASE_TYPE(l_ushort, unsigned short)
        DEFINE_BASE_TYPE(l_int, int)
        DEFINE_BASE_TYPE(l_uint, unsigned int)
        DEFINE_BASE_TYPE(l_long, slong)
        DEFINE_BASE_TYPE(l_ulong, ulong)
        DEFINE_BASE_TYPE(l_float, float)
        DEFINE_BASE_TYPE(l_double, double)
        DEFINE_BASE_TYPE(l_keyword, keyword_t)
        DEFINE_BASE_TYPE(l_operator, operator_t)
        DEFINE_BASE_TYPE(l_identifier, string_t)
        DEFINE_BASE_TYPE(l_string, string_t)
        DEFINE_BASE_TYPE(l_comment, string_t)
        DEFINE_BASE_TYPE(l_space, uint)
        DEFINE_BASE_TYPE(l_newline, uint)
        DEFINE_BASE_TYPE(l_error, error_t)
#undef DEFINE_BASE_TYPE

#define DEFINE_CONV_TYPE(t, obj) \
template<> \
struct base_lexer_t<obj> \
{ \
    static const lexer_t type = t; \
};

        DEFINE_CONV_TYPE(l_char, char)
        DEFINE_CONV_TYPE(l_uchar, unsigned char)
        DEFINE_CONV_TYPE(l_short, short)
        DEFINE_CONV_TYPE(l_ushort, unsigned short)
        DEFINE_CONV_TYPE(l_int, int)
        DEFINE_CONV_TYPE(l_uint, unsigned int)
        DEFINE_CONV_TYPE(l_long, slong)
        DEFINE_CONV_TYPE(l_ulong, ulong)
        DEFINE_CONV_TYPE(l_float, float)
        DEFINE_CONV_TYPE(l_double, double)
        DEFINE_CONV_TYPE(l_string, string_t)
        DEFINE_CONV_TYPE(l_error, error_t)
#undef DEFINE_CONV_TYPE

        const string_t& lexer_typestr(lexer_t);
    const string_t& lexer_keywordstr(keyword_t);
    int lexer_prior(lexer_t);
    int lexer_sizeof(lexer_t);
    const string_t& lexer_opstr(operator_t);
    const string_t& lexer_opnamestr(operator_t);
    const string_t& lexer_errstr(error_t);
    int lexer_operatorpred(operator_t);
    ins_t lexer_op2ins(operator_t);

#define LEX_T(t) base_t<l_##t>::type
#define LEX_CONV_T(t) base_lexer_t<t>::type
#define LEX_SIZEOF(t) sizeof(LEX_T(t))
#define LEX_SIZE(t) lexer_sizeof(t)
#define LEX_STRING(t) lexer_typestr(t)
#define LEX_PRIOR(t) lexer_prior(t)

#define KEYWORD_STRING(t) lexer_keywordstr(t)
#define OPERATOR_STRING(t) lexer_opnamestr(t)
#define OP_STRING(t) lexer_opstr(t)
#define ERROR_STRING(t) lexer_errstr(t)

#define OPERATOR_PRED(t) lexer_operatorpred(t)
#define OP_INS(t) lexer_op2ins(t)

    const string_t& ins_str(ins_t);
    int ins_ops(ins_t t);
#define INS_STRING(t) ins_str(t)
#define INS_OPNUM(t) ins_ops(t)

    enum coll_t {
        c_program,
        c_primaryExpression,
        c_constant,
        c_postfixExpression,
        c_argumentExpressionList,
        c_unaryExpression,
        c_unaryOperator,
        c_castExpression,
        c_multiplicativeExpression,
        c_additiveExpression,
        c_shiftExpression,
        c_relationalExpression,
        c_equalityExpression,
        c_andExpression,
        c_exclusiveOrExpression,
        c_inclusiveOrExpression,
        c_logicalAndExpression,
        c_logicalOrExpression,
        c_conditionalExpression,
        c_assignmentExpression,
        c_assignmentOperator,
        c_expression,
        c_constantExpression,
        c_declaration,
        c_declarationSpecifiers,
        c_declarationSpecifiers2,
        c_declarationSpecifier,
        c_initDeclaratorList,
        c_initDeclarator,
        c_storageClassSpecifier,
        c_typeSpecifier,
        c_structOrUnionSpecifier,
        c_structOrUnion,
        c_structDeclarationList,
        c_structDeclaration,
        c_specifierQualifierList,
        c_structDeclaratorList,
        c_structDeclarator,
        c_enumSpecifier,
        c_enumeratorList,
        c_enumerator,
        c_enumerationConstant,
        c_typeQualifier,
        c_declarator,
        c_directDeclarator,
        c_pointer,
        c_typeQualifierList,
        c_parameterTypeList,
        c_parameterList,
        c_parameterDeclaration,
        c_identifierList,
        c_typeName,
        c_abstractDeclarator,
        c_directAbstractDeclarator,
        c_typedefName,
        c_initializer,
        c_initializerList,
        c_designation,
        c_designatorList,
        c_designator,
        c_statement,
        c_labeledStatement,
        c_compoundStatement,
        c_blockItemList,
        c_blockItem,
        c_expressionStatement,
        c_selectionStatement,
        c_iterationStatement,
        c_forCondition,
        c_forDeclaration,
        c_forExpression,
        c_jumpStatement,
        c_pragmaStatement,
        c_compilationUnit,
        c_translationUnit,
        c_externalDeclaration,
        c_functionDefinition,
        c_declarationList,
    };

    const string_t& coll_str(coll_t);

#define COLL_STRING(t) coll_str(t)
}

#endif //CMINILANG_TYPES_H

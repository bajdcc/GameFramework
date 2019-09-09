//
// Project: CMiniLang
// Author: bajdcc
//

#include "stdafx.h"
#include <cassert>
#include <cstring>
#include "types.h"

namespace clib {
    std::tuple<lexer_t, string_t, int, int> lexer_string_list[] = {
        std::make_tuple(l_none, "none", 0, 0),
        std::make_tuple(l_error, "error", 0, 0),
        std::make_tuple(l_char, "char", 1, LEX_SIZEOF(char)),
        std::make_tuple(l_uchar, "uchar", 2, LEX_SIZEOF(uchar)),
        std::make_tuple(l_short, "short", 3, LEX_SIZEOF(short)),
        std::make_tuple(l_ushort, "ushort", 4, LEX_SIZEOF(ushort)),
        std::make_tuple(l_int, "int", 5, LEX_SIZEOF(int)),
        std::make_tuple(l_uint, "uint", 6, LEX_SIZEOF(uint)),
        std::make_tuple(l_long, "long", 7, LEX_SIZEOF(long)),
        std::make_tuple(l_ulong, "ulong", 8, LEX_SIZEOF(ulong)),
        std::make_tuple(l_float, "float", 9, LEX_SIZEOF(float)),
        std::make_tuple(l_double, "double", 10, LEX_SIZEOF(double)),
        std::make_tuple(l_operator, "operator", 0, 0),
        std::make_tuple(l_keyword, "keyword", 0, 0),
        std::make_tuple(l_identifier, "identifier", 0, 0),
        std::make_tuple(l_string, "string", 0, sizeof(void*)),
        std::make_tuple(l_comment, "comment", 0, 0),
        std::make_tuple(l_space, "space", 0, 0),
        std::make_tuple(l_newline, "newline", 0, 0),
        std::make_tuple(l_end, "END", 0, 0),
    };

    const string_t& lexer_typestr(lexer_t type) {
        assert(type >= l_none && type < l_end);
        return std::get<1>(lexer_string_list[type]);
    }

    int lexer_prior(lexer_t type) {
        assert(type >= l_none && type < l_end);
        return std::get<2>(lexer_string_list[type]);
    }

    int lexer_sizeof(lexer_t type) {
        assert(type >= l_none && type < l_end);
        return std::get<3>(lexer_string_list[type]);
    }

    std::tuple<keyword_t, string_t> keyword_string_list[] = {
        std::make_tuple(k__start, "@START"),
        std::make_tuple(k_auto, "auto"),
        std::make_tuple(k_bool, "bool"),
        std::make_tuple(k_break, "break"),
        std::make_tuple(k_case, "case"),
        std::make_tuple(k_char, "char"),
        std::make_tuple(k_const, "const"),
        std::make_tuple(k_continue, "continue"),
        std::make_tuple(k_default, "default"),
        std::make_tuple(k_do, "do"),
        std::make_tuple(k_double, "double"),
        std::make_tuple(k_else, "else"),
        std::make_tuple(k_enum, "enum"),
        std::make_tuple(k_extern, "extern"),
        std::make_tuple(k_false, "false"),
        std::make_tuple(k_float, "float"),
        std::make_tuple(k_for, "for"),
        std::make_tuple(k_goto, "goto"),
        std::make_tuple(k_if, "if"),
        std::make_tuple(k_int, "int"),
        std::make_tuple(k_long, "long"),
        std::make_tuple(k_register, "register"),
        std::make_tuple(k_return, "return"),
        std::make_tuple(k_short, "short"),
        std::make_tuple(k_signed, "signed"),
        std::make_tuple(k_sizeof, "sizeof"),
        std::make_tuple(k_static, "static"),
        std::make_tuple(k_struct, "struct"),
        std::make_tuple(k_switch, "switch"),
        std::make_tuple(k_true, "true"),
        std::make_tuple(k_typedef, "typedef"),
        std::make_tuple(k_union, "union"),
        std::make_tuple(k_unsigned, "unsigned"),
        std::make_tuple(k_void, "void"),
        std::make_tuple(k_volatile, "volatile"),
        std::make_tuple(k_while, "while"),
        std::make_tuple(k_interrupt, "interrupt"),
        std::make_tuple(k_pragma, "pragma"),
        std::make_tuple(k__end, "@END"),
    };

    const string_t& lexer_keywordstr(keyword_t type) {
        assert(type > k__start && type < k__end);
        return std::get<1>(keyword_string_list[type - k__start]);
    }

    std::tuple<operator_t, string_t, string_t, ins_t, int> operator_string_list[] = {
        std::make_tuple(op__start, "@START", "@START", NOP, 9999),
        std::make_tuple(op_assign, "=", "assign", NOP, 1401),
        std::make_tuple(op_equal, "==", "equal", EQ, 701),
        std::make_tuple(op_plus, "+", "plus", ADD, 401),
        std::make_tuple(op_plus_assign, "+=", "plus_assign", ADD, 1405),
        std::make_tuple(op_minus, "-", "minus", SUB, 402),
        std::make_tuple(op_minus_assign, "-=", "minus_assign", SUB, 1406),
        std::make_tuple(op_times, "*", "times", MUL, 302),
        std::make_tuple(op_times_assign, "*=", "times_assign", MUL, 1403),
        std::make_tuple(op_divide, "/", "divide", DIV, 301),
        std::make_tuple(op_div_assign, "/=", "div_assign", DIV, 1402),
        std::make_tuple(op_bit_and, "&", "bit_and", AND, 801),
        std::make_tuple(op_and_assign, "&=", "and_assign", AND, 1409),
        std::make_tuple(op_bit_or, "|", "bit_or", OR, 1001),
        std::make_tuple(op_or_assign, "|=", "or_assign", OR, 1411),
        std::make_tuple(op_bit_xor, "^", "bit_xor", XOR, 901),
        std::make_tuple(op_xor_assign, "^=", "xor_assign", XOR, 1410),
        std::make_tuple(op_mod, "%", "mod", MOD, 303),
        std::make_tuple(op_mod_assign, "%=", "mod_assign", MOD, 1404),
        std::make_tuple(op_less_than, "<", "less_than", LT, 603),
        std::make_tuple(op_less_than_or_equal, "<=", "less_than_or_equal", LE, 604),
        std::make_tuple(op_greater_than, ">", "greater_than", GT, 601),
        std::make_tuple(op_greater_than_or_equal, ">=", "greater_than_or_equal", GE, 602),
        std::make_tuple(op_logical_not, "!", "logical_not", NOP, 207),
        std::make_tuple(op_not_equal, "!=", "not_equal", NE, 702),
        std::make_tuple(op_escape, "\\", "escape", NOP, 9000),
        std::make_tuple(op_query, "?", "query", NOP, 1301),
        std::make_tuple(op_bit_not, "~", "bit_not", NOP, 208),
        std::make_tuple(op_lparan, "(", "lparan", NOP, 102),
        std::make_tuple(op_rparan, ")", "rparan", NOP, 102),
        std::make_tuple(op_lbrace, "{", "lbrace", NOP, 9000),
        std::make_tuple(op_rbrace, "}", "rbrace", NOP, 9000),
        std::make_tuple(op_lsquare, "[", "lsquare", NOP, 101),
        std::make_tuple(op_rsquare, "]", "rsquare", NOP, 101),
        std::make_tuple(op_comma, ",", "comma", NOP, 1501),
        std::make_tuple(op_dot, ".", "dot", NOP, 103),
        std::make_tuple(op_semi, ";", "semi", NOP, 9000),
        std::make_tuple(op_colon, ":", "colon", NOP, 1302),
        std::make_tuple(op_plus_plus, "++", "plus_plus", ADD, 203),
        std::make_tuple(op_minus_minus, "--", "minus_minus", SUB, 204),
        std::make_tuple(op_logical_and, "&&", "logical_and", JZ, 1101),
        std::make_tuple(op_logical_or, "||", "logical_or", JNZ, 1201),
        std::make_tuple(op_pointer, "->", "pointer", NOP, 104),
        std::make_tuple(op_left_shift, "<<", "left_shift", SHL, 501),
        std::make_tuple(op_right_shift, ">>", "right_shift", SHR, 502),
        std::make_tuple(op_left_shift_assign, "<<=", "left_shift_assign", SHL, 1407),
        std::make_tuple(op_right_shift_assign, ">>=", "right_shift_assign", SHR, 1408),
        std::make_tuple(op_ellipsis, "...", "ellipsis", NOP, 9000),
        std::make_tuple(op__end, "@END", "@END", NOP, 9999),
    };

    const string_t& lexer_opstr(operator_t type) {
        assert(type > op__start && type <= op__end);
        return std::get<1>(operator_string_list[type]);
    }

    const string_t& lexer_opnamestr(operator_t type) {
        assert(type > op__start && type <= op__end);
        return std::get<2>(operator_string_list[type]);
    }

    string_t err_string_list[] = {
        "@START",
        "#E !char!",
        "#E !operator!",
        "#E !digit!",
        "#E !string!",
        "@END",
    };

    const string_t& lexer_errstr(error_t type) {
        assert(type > e__start && type < e__end);
        return err_string_list[type];
    }

    int lexer_operatorpred(operator_t type) {
        assert(type > op__start && type < op__end);
        return std::get<4>(operator_string_list[type]);
    }

    ins_t lexer_op2ins(operator_t type) {
        assert(type > op__start && type < op__end);
        return std::get<3>(operator_string_list[type]);
    }

    std::tuple<coll_t, string_t> coll_string_list[] = {
        std::make_tuple(c_program, "program"),
        std::make_tuple(c_primaryExpression, "primaryExpression"),
        std::make_tuple(c_constant, "constant"),
        std::make_tuple(c_postfixExpression, "postfixExpression"),
        std::make_tuple(c_argumentExpressionList, "argumentExpressionList"),
        std::make_tuple(c_unaryExpression, "unaryExpression"),
        std::make_tuple(c_unaryOperator, "unaryOperator"),
        std::make_tuple(c_castExpression, "castExpression"),
        std::make_tuple(c_multiplicativeExpression, "multiplicativeExpression"),
        std::make_tuple(c_additiveExpression, "additiveExpression"),
        std::make_tuple(c_shiftExpression, "shiftExpression"),
        std::make_tuple(c_relationalExpression, "relationalExpression"),
        std::make_tuple(c_equalityExpression, "equalityExpression"),
        std::make_tuple(c_andExpression, "andExpression"),
        std::make_tuple(c_exclusiveOrExpression, "exclusiveOrExpression"),
        std::make_tuple(c_inclusiveOrExpression, "inclusiveOrExpression"),
        std::make_tuple(c_logicalAndExpression, "logicalAndExpression"),
        std::make_tuple(c_logicalOrExpression, "logicalOrExpression"),
        std::make_tuple(c_conditionalExpression, "conditionalExpression"),
        std::make_tuple(c_assignmentExpression, "assignmentExpression"),
        std::make_tuple(c_assignmentOperator, "assignmentOperator"),
        std::make_tuple(c_expression, "expression"),
        std::make_tuple(c_constantExpression, "constantExpression"),
        std::make_tuple(c_declaration, "declaration"),
        std::make_tuple(c_declarationSpecifiers, "declarationSpecifiers"),
        std::make_tuple(c_declarationSpecifiers2, "declarationSpecifiers2"),
        std::make_tuple(c_declarationSpecifier, "declarationSpecifier"),
        std::make_tuple(c_initDeclaratorList, "initDeclaratorList"),
        std::make_tuple(c_initDeclarator, "initDeclarator"),
        std::make_tuple(c_storageClassSpecifier, "storageClassSpecifier"),
        std::make_tuple(c_typeSpecifier, "typeSpecifier"),
        std::make_tuple(c_structOrUnionSpecifier, "structOrUnionSpecifier"),
        std::make_tuple(c_structOrUnion, "structOrUnion"),
        std::make_tuple(c_structDeclarationList, "structDeclarationList"),
        std::make_tuple(c_structDeclaration, "structDeclaration"),
        std::make_tuple(c_specifierQualifierList, "specifierQualifierList"),
        std::make_tuple(c_structDeclaratorList, "structDeclaratorList"),
        std::make_tuple(c_structDeclarator, "structDeclarator"),
        std::make_tuple(c_enumSpecifier, "enumSpecifier"),
        std::make_tuple(c_enumeratorList, "enumeratorList"),
        std::make_tuple(c_enumerator, "enumerator"),
        std::make_tuple(c_enumerationConstant, "enumerationConstant"),
        std::make_tuple(c_typeQualifier, "typeQualifier"),
        std::make_tuple(c_declarator, "declarator"),
        std::make_tuple(c_directDeclarator, "directDeclarator"),
        std::make_tuple(c_pointer, "pointer"),
        std::make_tuple(c_typeQualifierList, "typeQualifierList"),
        std::make_tuple(c_parameterTypeList, "parameterTypeList"),
        std::make_tuple(c_parameterList, "parameterList"),
        std::make_tuple(c_parameterDeclaration, "parameterDeclaration"),
        std::make_tuple(c_identifierList, "identifierList"),
        std::make_tuple(c_typeName, "typeName"),
        std::make_tuple(c_abstractDeclarator, "abstractDeclarator"),
        std::make_tuple(c_directAbstractDeclarator, "directAbstractDeclarator"),
        std::make_tuple(c_typedefName, "typedefName"),
        std::make_tuple(c_initializer, "initializer"),
        std::make_tuple(c_initializerList, "initializerList"),
        std::make_tuple(c_designation, "designation"),
        std::make_tuple(c_designatorList, "designatorList"),
        std::make_tuple(c_designator, "designator"),
        std::make_tuple(c_statement, "statement"),
        std::make_tuple(c_labeledStatement, "labeledStatement"),
        std::make_tuple(c_compoundStatement, "compoundStatement"),
        std::make_tuple(c_blockItemList, "blockItemList"),
        std::make_tuple(c_blockItem, "blockItem"),
        std::make_tuple(c_expressionStatement, "expressionStatement"),
        std::make_tuple(c_selectionStatement, "selectionStatement"),
        std::make_tuple(c_iterationStatement, "iterationStatement"),
        std::make_tuple(c_forCondition, "forCondition"),
        std::make_tuple(c_forDeclaration, "forDeclaration"),
        std::make_tuple(c_forExpression, "forExpression"),
        std::make_tuple(c_jumpStatement, "jumpStatement"),
        std::make_tuple(c_compilationUnit, "compilationUnit"),
        std::make_tuple(c_translationUnit, "translationUnit"),
        std::make_tuple(c_externalDeclaration, "externalDeclaration"),
        std::make_tuple(c_functionDefinition, "functionDefinition"),
        std::make_tuple(c_declarationList, "declarationList"),
    };

    const string_t& coll_str(coll_t t) {
        assert(t >= c_program && t <= c_declarationList);
        return std::get<1>(coll_string_list[t]);
    }

    std::tuple<ins_t, string_t, int> ins_string_list[] = {
        std::make_tuple(NOP, "NOP", 0),
        std::make_tuple(LEA, "LEA", 1),
        std::make_tuple(IMM, "IMM", 1),
        std::make_tuple(IMX, "IMX", 2),
        std::make_tuple(JMP, "JMP", 1),
        std::make_tuple(JZ, "JZ", 1),
        std::make_tuple(JNZ, "JNZ", 1),
        std::make_tuple(ENT, "ENT", 1),
        std::make_tuple(LOAD, "LOAD", 1),
        std::make_tuple(SAVE, "SAVE", 1),
        std::make_tuple(INTR, "INTR", 1),
        std::make_tuple(CAST, "CAST", 1),
        std::make_tuple(ADJ, "ADJ", 1),
        std::make_tuple(CALL, "CALL", 0),
        std::make_tuple(LEV, "LEV", 0),
        std::make_tuple(PUSH, "PUSH", 1),
        std::make_tuple(POP, "POP", 1),
        std::make_tuple(OR, "OR", 1),
        std::make_tuple(XOR, "XOR", 1),
        std::make_tuple(AND, "AND", 1),
        std::make_tuple(EQ, "EQ", 1),
        std::make_tuple(CASE, "CASE", 0),
        std::make_tuple(NE, "NE", 1),
        std::make_tuple(LT, "LT", 1),
        std::make_tuple(GT, "GT", 1),
        std::make_tuple(LE, "LE", 1),
        std::make_tuple(GE, "GE", 1),
        std::make_tuple(SHL, "SHL", 1),
        std::make_tuple(SHR, "SHR", 1),
        std::make_tuple(ADD, "ADD", 1),
        std::make_tuple(SUB, "SUB", 1),
        std::make_tuple(MUL, "MUL", 1),
        std::make_tuple(DIV, "DIV", 1),
        std::make_tuple(MOD, "MOD", 1),
        std::make_tuple(NEG, "NEG", 1),
        std::make_tuple(NOT, "NOT", 1),
        std::make_tuple(LNT, "LNT", 1),
        std::make_tuple(EXIT, "EXIT", 0),
    };

    const string_t& ins_str(ins_t t) {
        assert(t >= NOP && t <= EXIT);
        return std::get<1>(ins_string_list[t]);
    }

    int ins_ops(ins_t t) {
        assert(t >= NOP && t <= EXIT);
        return std::get<2>(ins_string_list[t]);
    }
}

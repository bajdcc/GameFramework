//
// Project: CMiniLang
// Author: bajdcc
//

#include "stdafx.h"
#include <iomanip>
#include <iostream>
#include <algorithm>
#include "cexception.h"
#include "cparser.h"
#include "clexer.h"
#include "cast.h"
#include "cunit.h"

#define REPORT_ERROR 0
#define REPORT_ERROR_FILE "parsing.log"

#define TRACE_PARSING 0
#define DUMP_PDA 0
#define DUMP_PDA_FILE "PDA.log"
#define DEBUG_AST 0
#define CHECK_AST 0

namespace clib {

    ast_node* cparser::parse(const string_t& str, csemantic* s) {
        semantic = s;
        lexer = std::make_unique<clexer>(str);
        ast = std::make_unique<cast>();
        // 清空词法分析结果
        lexer->reset();
        // 清空AST
        ast->reset();
        // 产生式
        if (unit.get_pda().empty())
            gen();
        // 语法分析（递归下降）
        program();
        return ast->get_root();
    }

    ast_node* cparser::root() const {
        return ast->get_root();
    }

    void cparser::clear_ast() {
        ast.reset();
    }

    void cparser::next() {
        lexer_t token;
        do {
            token = lexer->next();
            if (token == l_error) {
                auto err = lexer->recent_error();
                ATLTRACE("[%04d:%03d] %-12s - %s\n",
                    err.line,
                    err.column,
                    ERROR_STRING(err.err).c_str(),
                    err.str.c_str());
            }
        } while (token == l_newline || token == l_space || token == l_error || token == l_comment);
#if 0
        if (token != l_end) {
            ATLTRACE("[%04d:%03d] %-12s - %s\n",
                lexer->get_last_line(),
                lexer->get_last_column(),
                LEX_STRING(lexer->get_type()).c_str(),
                lexer->current().c_str());
        }
#endif
    }

    void cparser::gen() {
        // REFER: antlr/grammars-v4
        // URL: https://github.com/antlr/grammars-v4/blob/master/c/C.g4
#define DEF_KEYWORD(name) auto &_##name##_ = unit.token(k_##name)
        DEF_KEYWORD(auto);
        DEF_KEYWORD(bool);
        DEF_KEYWORD(break);
        DEF_KEYWORD(case);
        DEF_KEYWORD(char);
        DEF_KEYWORD(const);
        DEF_KEYWORD(continue);
        DEF_KEYWORD(default);
        DEF_KEYWORD(do);
        DEF_KEYWORD(double);
        DEF_KEYWORD(else);
        DEF_KEYWORD(enum);
        DEF_KEYWORD(extern);
        DEF_KEYWORD(false);
        DEF_KEYWORD(float);
        DEF_KEYWORD(for);
        DEF_KEYWORD(goto);
        DEF_KEYWORD(if);
        DEF_KEYWORD(int);
        DEF_KEYWORD(long);
        DEF_KEYWORD(register);
        DEF_KEYWORD(return);
        DEF_KEYWORD(short);
        DEF_KEYWORD(signed);
        DEF_KEYWORD(sizeof);
        DEF_KEYWORD(static);
        DEF_KEYWORD(struct);
        DEF_KEYWORD(switch);
        DEF_KEYWORD(true);
        DEF_KEYWORD(typedef);
        DEF_KEYWORD(union);
        DEF_KEYWORD(unsigned);
        DEF_KEYWORD(void);
        DEF_KEYWORD(volatile);
        DEF_KEYWORD(while);
        DEF_KEYWORD(interrupt);
        DEF_KEYWORD(pragma);
#undef DEF_KEYWORD
#define DEF_OP(name) auto &_##name##_ = unit.token(op_##name)
        DEF_OP(assign);
        DEF_OP(equal);
        DEF_OP(plus);
        DEF_OP(plus_assign);
        DEF_OP(minus);
        DEF_OP(minus_assign);
        DEF_OP(times);
        DEF_OP(times_assign);
        DEF_OP(divide);
        DEF_OP(div_assign);
        DEF_OP(bit_and);
        DEF_OP(and_assign);
        DEF_OP(bit_or);
        DEF_OP(or_assign);
        DEF_OP(bit_xor);
        DEF_OP(xor_assign);
        DEF_OP(mod);
        DEF_OP(mod_assign);
        DEF_OP(less_than);
        DEF_OP(less_than_or_equal);
        DEF_OP(greater_than);
        DEF_OP(greater_than_or_equal);
        DEF_OP(logical_not);
        DEF_OP(not_equal);
        DEF_OP(escape);
        DEF_OP(query);
        DEF_OP(bit_not);
        DEF_OP(lparan);
        DEF_OP(rparan);
        DEF_OP(lbrace);
        DEF_OP(rbrace);
        DEF_OP(lsquare);
        DEF_OP(rsquare);
        DEF_OP(comma);
        DEF_OP(dot);
        DEF_OP(semi);
        DEF_OP(colon);
        DEF_OP(plus_plus);
        DEF_OP(minus_minus);
        DEF_OP(logical_and);
        DEF_OP(logical_or);
        DEF_OP(pointer);
        DEF_OP(left_shift);
        DEF_OP(right_shift);
        DEF_OP(left_shift_assign);
        DEF_OP(right_shift_assign);
        DEF_OP(ellipsis);
#undef DEF_OP
#define DEF_LEX(name, real) auto &real = unit.token(l_##name)
        DEF_LEX(char, Char);
        DEF_LEX(uchar, UnsignedChar);
        DEF_LEX(short, Short);
        DEF_LEX(ushort, UnsignedShort);
        DEF_LEX(int, Integer);
        DEF_LEX(uint, UnsignedInteger);
        DEF_LEX(long, Long);
        DEF_LEX(ulong, UnsignedLong);
        DEF_LEX(float, Float);
        DEF_LEX(double, Double);
        DEF_LEX(identifier, Identifier);
        DEF_LEX(string, String);
        DEF_LEX(comment, Comment);
        DEF_LEX(space, Space);
        DEF_LEX(newline, Newline);
#undef DEF_LEX
#define DEF_RULE(name) auto &name = unit.rule(#name, c_##name)
#define DEF_RULE_ATTR(name, attr) auto &name = unit.rule(#name, c_##name, attr)
#define DEF_RULE_NOT_GREED(name) DEF_RULE_ATTR(name, r_not_greed)
#define DEF_RULE_EXP(name) DEF_RULE_ATTR(name, r_exp)
        DEF_RULE(program);
        DEF_RULE(primaryExpression);
        DEF_RULE(constant);
        DEF_RULE_EXP(postfixExpression);
        DEF_RULE(argumentExpressionList);
        DEF_RULE_EXP(unaryExpression);
        DEF_RULE_EXP(unaryOperator);
        DEF_RULE_EXP(castExpression);
        DEF_RULE_EXP(multiplicativeExpression);
        DEF_RULE_EXP(additiveExpression);
        DEF_RULE_EXP(shiftExpression);
        DEF_RULE_EXP(relationalExpression);
        DEF_RULE_EXP(equalityExpression);
        DEF_RULE_EXP(andExpression);
        DEF_RULE_EXP(exclusiveOrExpression);
        DEF_RULE_EXP(inclusiveOrExpression);
        DEF_RULE_EXP(logicalAndExpression);
        DEF_RULE_EXP(logicalOrExpression);
        DEF_RULE_EXP(conditionalExpression);
        DEF_RULE_EXP(assignmentExpression);
        DEF_RULE(assignmentOperator);
        DEF_RULE_EXP(expression);
        DEF_RULE(constantExpression);
        DEF_RULE(declaration);
        DEF_RULE_NOT_GREED(declarationSpecifiers);
        DEF_RULE_NOT_GREED(declarationSpecifiers2);
        DEF_RULE(declarationSpecifier);
        DEF_RULE(initDeclaratorList);
        DEF_RULE(initDeclarator);
        DEF_RULE(storageClassSpecifier);
        DEF_RULE_NOT_GREED(typeSpecifier);
        DEF_RULE(structOrUnionSpecifier);
        DEF_RULE(structOrUnion);
        DEF_RULE(structDeclarationList);
        DEF_RULE(structDeclaration);
        DEF_RULE(specifierQualifierList);
        DEF_RULE(structDeclaratorList);
        DEF_RULE(structDeclarator);
        DEF_RULE(enumSpecifier);
        DEF_RULE(enumeratorList);
        DEF_RULE(enumerator);
        DEF_RULE(enumerationConstant);
        DEF_RULE(typeQualifier);
        DEF_RULE(declarator);
        DEF_RULE(directDeclarator);
        DEF_RULE(pointer);
        DEF_RULE(typeQualifierList);
        DEF_RULE(parameterTypeList);
        DEF_RULE(parameterList);
        DEF_RULE(parameterDeclaration);
        DEF_RULE(identifierList);
        DEF_RULE(typeName);
        DEF_RULE(abstractDeclarator);
        DEF_RULE(directAbstractDeclarator);
        DEF_RULE(typedefName);
        DEF_RULE(initializer);
        DEF_RULE(initializerList);
        DEF_RULE(designation);
        DEF_RULE(designatorList);
        DEF_RULE(designator);
        DEF_RULE(statement);
        DEF_RULE(labeledStatement);
        DEF_RULE(compoundStatement);
        DEF_RULE(blockItemList);
        DEF_RULE(blockItem);
        DEF_RULE(expressionStatement);
        DEF_RULE(selectionStatement);
        DEF_RULE(iterationStatement);
        DEF_RULE(forCondition);
        DEF_RULE(forDeclaration);
        DEF_RULE(forExpression);
        DEF_RULE(jumpStatement);
        DEF_RULE(pragmaStatement);
        DEF_RULE(compilationUnit);
        DEF_RULE(translationUnit);
        DEF_RULE(externalDeclaration);
        DEF_RULE(functionDefinition);
        DEF_RULE(declarationList);
#undef DEF_RULE
#undef DEF_RULE_NOT_GREED
#undef DEF_RULE_EXP
        program = compilationUnit;
        primaryExpression = Identifier
            | constant
            | String
            | ~_lparan_ + expression + ~_rparan_;
        constant = Char | UnsignedChar | Short | UnsignedShort | Integer | UnsignedInteger |
            Long | UnsignedLong | Float | Double | _true_ | _false_;
        postfixExpression = primaryExpression
            | ~_lparan_ + typeName + ~_rparan_ + ~_lbrace_ + initializerList + *~_comma_ + ~_rbrace_
            | postfixExpression + (_lsquare_ + expression + ~_rsquare_
                | _lparan_ + *argumentExpressionList + _rparan_
                | (_dot_ | _pointer_) + Identifier
                | _plus_plus_
                | _minus_minus_);
        argumentExpressionList = *(argumentExpressionList + ~_comma_) + assignmentExpression;
        unaryExpression
            = postfixExpression
            | (_plus_plus_ | _minus_minus_) + unaryExpression
            | unaryOperator + castExpression
            | _sizeof_ + (unaryExpression | ~_lparan_ + typeName + ~_rparan_);
        unaryOperator = _bit_and_ | _times_ | _plus_ | _minus_ | _bit_not_ | _logical_not_;
        castExpression = ~_lparan_ + typeName + ~_rparan_ + castExpression | unaryExpression;
        multiplicativeExpression = *(multiplicativeExpression + (_times_ | _divide_ | _mod_)) + castExpression;
        additiveExpression = *(additiveExpression + (_plus_ | _minus_)) + multiplicativeExpression;
        shiftExpression = *(shiftExpression + (_left_shift_ | _right_shift_)) + additiveExpression;
        relationalExpression = *(relationalExpression +
            (_less_than_ | _greater_than_ | _less_than_or_equal_ | _greater_than_or_equal_)) +
            shiftExpression;
        equalityExpression = *(equalityExpression + (_equal_ | _not_equal_)) + relationalExpression;
        andExpression = *(andExpression + _bit_and_) + equalityExpression;
        exclusiveOrExpression = *(exclusiveOrExpression + _bit_xor_) + andExpression;
        inclusiveOrExpression = *(inclusiveOrExpression + _bit_or_) + exclusiveOrExpression;
        logicalAndExpression = *(logicalAndExpression + _logical_and_) + inclusiveOrExpression;
        logicalOrExpression = *(logicalOrExpression + _logical_or_) + logicalAndExpression;
        conditionalExpression = logicalOrExpression + *(_query_ + expression + _colon_ + conditionalExpression);
        assignmentExpression = conditionalExpression | unaryExpression + assignmentOperator + assignmentExpression;
        assignmentOperator = _assign_ | _times_assign_ | _div_assign_ | _mod_assign_ |
            _and_assign_ | _or_assign_ | _xor_assign_ |
            _plus_assign_ | _minus_assign_ | _left_shift_assign_ | _right_shift_assign_;
        expression = *(expression + ~_comma_) + assignmentExpression;
        constantExpression = conditionalExpression;
        declaration = declarationSpecifiers + *initDeclaratorList + ~_semi_;
        declarationSpecifiers = *declarationSpecifiers + declarationSpecifier;
        declarationSpecifiers2 = *declarationSpecifiers + declarationSpecifier;
        declarationSpecifier
            = storageClassSpecifier
            | typeSpecifier
            | typeQualifier;
        initDeclaratorList = *(initDeclaratorList + ~_comma_) + initDeclarator;
        initDeclarator = declarator + *(~_assign_ + initializer);
        storageClassSpecifier = _typedef_ | _extern_ | _static_ | _auto_ | _register_;
        typeSpecifier = _void_ | _char_ | _short_ | _int_ | _long_ | _float_ | _double_ | _signed_ | _unsigned_ | _bool_
            | structOrUnionSpecifier
            | enumSpecifier
            | typedefName
            | typeSpecifier + pointer;
        structOrUnionSpecifier =
            structOrUnion + Identifier + ~_lbrace_ + structDeclarationList + ~_rbrace_;
        structOrUnion = _struct_ | _union_;
        structDeclarationList = *structDeclarationList + structDeclaration;
        structDeclaration = specifierQualifierList + *structDeclaratorList + ~_semi_;
        specifierQualifierList = *specifierQualifierList + (typeSpecifier | typeQualifier);
        structDeclaratorList = *(structDeclaratorList + ~_comma_) + structDeclarator;
        structDeclarator = declarator | *declarator + ~_assign_ + constantExpression;
        enumSpecifier = _enum_ + ((*Identifier + _lbrace_ + enumeratorList + *~_comma_ + ~_rbrace_) | Identifier);
        enumeratorList = *(enumeratorList + ~_comma_) + enumerator;
        enumerator = enumerationConstant + *(~_assign_ + constantExpression);
        enumerationConstant = Identifier;
        typeQualifier = _const_ | _volatile_;
        declarator = *pointer + directDeclarator;
        directDeclarator
            = Identifier
            | ~_lparan_ + declarator + ~_rparan_
            | directDeclarator + ((_lsquare_ + Integer + ~_rsquare_)
                | (_lparan_ + (parameterTypeList | *identifierList) + ~_rparan_));
        pointer = (_times_ | _bit_xor_) + (*typeQualifierList + *pointer);
        typeQualifierList = *typeQualifierList + typeQualifier;
        parameterTypeList = parameterList + *(~_comma_ + _ellipsis_);
        parameterList = *(parameterList + ~_comma_) + parameterDeclaration;
        parameterDeclaration = declarationSpecifiers + declarator
            | declarationSpecifiers2 + *abstractDeclarator;
        identifierList = *(identifierList + ~_comma_) + Identifier;
        typeName = specifierQualifierList + *abstractDeclarator;
        abstractDeclarator = pointer
            | *pointer + directAbstractDeclarator;
        directAbstractDeclarator = ~_lparan_ + abstractDeclarator + ~_rparan_
            | *directAbstractDeclarator +
            ((_lsquare_ + ((*typeQualifierList + *assignmentExpression) | _times_) + _rsquare_)
                | _lparan_ + *parameterTypeList + _rparan_);
        typedefName = Identifier;
        initializer = assignmentExpression | _lbrace_ + initializerList + *~_comma_ + _rbrace_;
        initializerList = *(initializerList + ~_comma_) + *designation + initializer;
        designation = designatorList + _assign_;
        designatorList = *designatorList + designator;
        designator = _lsquare_ + constantExpression + _rsquare_
            | _dot_ + Identifier;
        statement = labeledStatement
            | compoundStatement
            | expressionStatement
            | selectionStatement
            | iterationStatement
            | jumpStatement;
        labeledStatement = (Identifier
            | _case_ + constantExpression
            | _default_) + ~_colon_ + statement;
        compoundStatement = ~_lbrace_ + *blockItemList + ~_rbrace_;
        blockItemList = *blockItemList + blockItem;
        blockItem = statement | declaration;
        expressionStatement = *expression + ~~_semi_;
        selectionStatement
            = _if_ + ~_lparan_ + expression + ~_rparan_ + statement + *(_else_ + statement)
            | _switch_ + ~_lparan_ + expression + ~_rparan_ + statement;
        iterationStatement
            = _while_ + ~_lparan_ + expression + ~_rparan_ + statement
            | _do_ + statement + _while_ + ~_lparan_ + expression + ~_rparan_ + _semi_
            | _for_ + ~_lparan_ + forCondition + ~_rparan_ + statement;
        forCondition = (forDeclaration | *expression) + _semi_ + *forExpression + _semi_ + *forExpression;
        forDeclaration = declarationSpecifiers + *initDeclaratorList;
        forExpression = *(forExpression + ~_comma_) + assignmentExpression;
        jumpStatement = (_goto_ + Identifier | _continue_ | _break_ | _return_ + *expression | _interrupt_ + Integer) + ~_semi_;
        pragmaStatement = (_pragma_ + String) + ~_semi_;
        compilationUnit = translationUnit + *compilationUnit;
        translationUnit = *translationUnit + externalDeclaration;
        externalDeclaration = functionDefinition | declaration | pragmaStatement | ~_semi_;
        functionDefinition = *declarationSpecifiers + declarator + *declarationList + compoundStatement;
        declarationList = *declarationList + declaration;
        unit.gen(&compilationUnit);
#if DUMP_PDA
        std::ofstream of(DUMP_PDA_FILE);
        unit.dump(of);
#endif
    }

    void check_ast(ast_node * node) {
#if CHECK_AST
        if (node->child) {
            auto& c = node->child;
            auto i = c;
            assert(i->parent == node);
            check_ast(i);
            if (i->next != i) {
                assert(i->prev->next == i);
                assert(i->next->prev == i);
                i = i->next;
                do {
                    assert(i->parent == node);
                    assert(i->prev->next == i);
                    assert(i->next->prev == i);
                    check_ast(i);
                    i = i->next;
                } while (i != c);
            }
            else {
                assert(i->prev == i);
            }
        }
#endif
    }

    void cparser::program() {
#if REPORT_ERROR
        std::ofstream log(REPORT_ERROR_FILE, std::ios::app | std::ios::out);
#endif
        base_type = l_none;
        next();
        state_stack.clear();
        ast_stack.clear();
        ast_cache.clear();
        ast_cache_index = 0;
        ast_coll_cache.clear();
        ast_reduce_cache.clear();
        state_stack.push_back(0);
        const auto& pdas = unit.get_pda();
        auto root = ast->new_node(ast_collection);
        root->line = root->column = 0;
        root->data._coll = pdas[0].coll;
        cast::set_child(ast->get_root(), root);
        ast_stack.push_back(root);
        std::vector<int> jumps;
        std::vector<int> trans_ids;
        backtrace_t bk_tmp;
        bk_tmp.lexer_index = 0;
        bk_tmp.state_stack = state_stack;
        bk_tmp.ast_stack = ast_stack;
        bk_tmp.current_state = 0;
        bk_tmp.coll_index = 0;
        bk_tmp.reduce_index = 0;
        bk_tmp.direction = b_next;
        std::vector<backtrace_t> bks;
        bks.push_back(bk_tmp);
        auto trans_id = -1;
        auto prev_idx = 0;
        auto jump_state = -1;
        while (!bks.empty()) {
            auto bk = &bks.back();
            if (bk->direction == b_success || bk->direction == b_fail) {
                break;
            }
            if (bk->direction == b_fallback) {
                if (bk->trans_ids.empty()) {
                    if (jump_state != -1) {
                        bool suc = false;
                        while (true) {
                            if (bks.back().current_state != jump_state) {
                                bks.pop_back();
                                if (bks.empty()) {
                                    break;
                                }
                            }
                            else {
                                jump_state = -1;
                                suc = true;
                                break;
                            }
                        }
                        if (suc) {
                            bks.back().direction = b_error;
                            bk = &bks.back();
                        }
                        else break;
                    }
                    else if (bks.size() > 1) {
                        bks.pop_back();
                        bks.back().direction = b_error;
                        bk = &bks.back();
                        if (bk->lexer_index < prev_idx) {
                            bk->direction = b_fail;
                            continue;
                        }
                    }
                    else {
                        bk->direction = b_fail;
                        continue;
                    }
                }
            }
            ast_cache_index = bk->lexer_index;
            state_stack = bk->state_stack;
            ast_stack = bk->ast_stack;
            auto state = bk->current_state;
            if (bk->direction != b_error)
                for (;;) {
                    auto is_end = lexer->is_type(l_end) && ast_cache_index >= ast_cache.size();
                    const auto& current_state = pdas[state];
                    if (is_end) {
                        if (current_state.final) {
                            if (state_stack.empty()) {
                                bk->direction = b_success;
                                break;
                            }
                        }
                    }
                    auto& trans = current_state.trans;
                    if (trans_id == -1 && !bk->trans_ids.empty()) {
                        trans_id = bk->trans_ids.back() & ((1 << 16) - 1);
                        bk->trans_ids.pop_back();
                    }
                    else {
                        trans_ids.clear();
                        if (is_end) {
                            for (size_t i = 0; i < trans.size(); ++i) {
                                auto& cs = trans[i];
                                if (valid_trans(cs) && (cs.type != e_move && cs.type != e_pass)) {
                                    trans_ids.push_back(i | pda_edge_priority(cs.type) << 16);
                                }
                            }
                        }
                        else {
                            for (size_t i = 0; i < trans.size(); ++i) {
                                auto& cs = trans[i];
                                if (valid_trans(cs)) {
                                    trans_ids.push_back(i | pda_edge_priority(cs.type) << 16);
                                }
                            }
                            if (trans.size() == 1 && !trans_ids.empty() &&
                                (trans[0].type == e_move || trans[0].type == e_pass) &&
                                trans[0].marked) {
                                prev_idx = bk->lexer_index;
                            }
                        }
                        if (!trans_ids.empty()) {
                            std::sort(trans_ids.begin(), trans_ids.end(), std::greater<>());
                            if (trans_ids.size() > 1) {
                                bk_tmp.lexer_index = ast_cache_index;
                                bk_tmp.state_stack = state_stack;
                                bk_tmp.ast_stack = ast_stack;
                                bk_tmp.current_state = state;
                                bk_tmp.trans_ids = trans_ids;
                                bk_tmp.coll_index = ast_coll_cache.size();
                                bk_tmp.reduce_index = ast_reduce_cache.size();
                                bk_tmp.direction = b_next;
#if DEBUG_AST
                                for (auto i = 0; i < bks.size(); ++i) {
                                    auto& _bk = bks[i];
                                    ATLTRACE("[DEBUG] Branch old: i=%d, LI=%d, SS=%d, AS=%d, S=%d, TS=%d, CI=%d, RI=%d, TK=%d\n",
                                        i, _bk.lexer_index, _bk.state_stack.size(),
                                        _bk.ast_stack.size(), _bk.current_state, _bk.trans_ids.size(),
                                        _bk.coll_index, _bk.reduce_index, _bk.ast_ids.size());
                                }
#endif
                                bks.push_back(bk_tmp);
                                bk = &bks.back();
#if DEBUG_AST
                                ATLTRACE("[DEBUG] Branch new: BS=%d, LI=%d, SS=%d, AS=%d, S=%d, TS=%d, CI=%d, RI=%d, TK=%d\n",
                                    bks.size(), bk_tmp.lexer_index, bk_tmp.state_stack.size(),
                                    bk_tmp.ast_stack.size(), bk_tmp.current_state, bk_tmp.trans_ids.size(),
                                    bk_tmp.coll_index, bk_tmp.reduce_index, bk_tmp.ast_ids.size());
#endif
                                bk->direction = b_next;
                                break;
                            }
                            else {
                                trans_id = trans_ids.back() & ((1 << 16) - 1);
                                trans_ids.pop_back();
                            }
                        }
                        else {
#if TRACE_PARSING
                            std::cout << "parsing error: " << current_state.label << std::endl;
#endif
#if REPORT_ERROR
                            log << "parsing error: " << current_state.label << std::endl;
#endif
                            bk->direction = b_error;
                            /*if (semantic) {
                                semantic->error_handler(state, current_state.trans, jump_state);
                            }*/
                            break;
                        }
                    }
                    auto& t = trans[trans_id];
                    if (t.type == e_finish) {
                        if (!is_end) {
#if TRACE_PARSING
                            std::cout << "parsing redundant code: " << current_state.label << std::endl;
#endif
#if REPORT_ERROR
                            log << "parsing redundant code: " << current_state.label << std::endl;
#endif
                            bk->direction = b_error;
                            break;
                        }
                    }
                    auto jump = trans[trans_id].jump;
#if TRACE_PARSING
                    ATLTRACE("[%d:%d:%d]%s State: %3d => To: %3d   -- Action: %-10s -- Rule: %s\n",
                        ast_cache_index, ast_stack.size(), bks.size(), is_end ? "*" : "", state, jump,
                        pda_edge_str(t.type).c_str(), current_state.label.c_str());

#endif
#if REPORT_ERROR
                    {
                        static char fmt[256];
                        int line = 0, column = 0;
                        if (ast_cache_index < ast_cache.size()) {
                            line = ast_cache[ast_cache_index]->line;
                            column = ast_cache[ast_cache_index]->column;
                        }
                        else {
                            line = lexer->get_line();
                            column = lexer->get_column();
                        }
                        snprintf(fmt, sizeof(fmt), "[%d,%d:%d:%d:%d]%s State: %3d => To: %3d   -- Action: %-10s -- Rule: %s",
                            line, column, ast_cache_index, ast_stack.size(),
                            bks.size(), is_end ? "*" : "", state, jump,
                            pda_edge_str(t.type).c_str(), current_state.label.c_str());
                        log << fmt << std::endl;
                    }
#endif
                    do_trans(state, *bk, trans[trans_id]);
                    auto old_state = state;
                    state = jump;
                    if (semantic) {
                        // DETERMINE LR JUMP BEFORE PARSING AST
                        bk->direction = semantic->check(trans[trans_id].type, ast_stack.back());
                        if (bk->direction == b_error) {
#if TRACE_PARSING
                            std::cout << "parsing semantic error: " << current_state.label << std::endl;
#endif
#if REPORT_ERROR
                            log << "parsing semantic error: " << current_state.label << std::endl;
#endif
                            break;
                        }
                    }
                }
            if (bk->direction == b_error) {
#if DEBUG_AST
                for (auto i = 0; i < bks.size(); ++i) {
                    auto& _bk = bks[i];
                    ATLTRACE("[DEBUG] Backtrace failed: i=%d, LI=%d, SS=%d, AS=%d, S=%d, TS=%d, CI=%d, RI=%d, TK=%d\n",
                        i, _bk.lexer_index, _bk.state_stack.size(),
                        _bk.ast_stack.size(), _bk.current_state, _bk.trans_ids.size(),
                        _bk.coll_index, _bk.reduce_index, _bk.ast_ids.size());
                }
#endif
                for (auto& i : bk->ast_ids) {
                    auto& token = ast_cache[i];
                    check_ast(token);
#if DEBUG_AST
                    ATLTRACE("[DEBUG] Backtrace failed, unlink token: %p, PB=%p\n", token, token->parent);
#endif
                    cast::unlink(token);
                    check_ast(token);
                }
                auto size = ast_reduce_cache.size();
                for (auto i = size; i > bk->reduce_index; --i) {
                    auto& coll = ast_reduce_cache[i - 1];
                    check_ast(coll);
#if DEBUG_AST
                    ATLTRACE("[DEBUG] Backtrace failed, unlink: %p, PB=%p, NE=%d, CB=%d\n",
                        coll, coll->parent, cast::children_size(coll->parent), cast::children_size(coll));
#endif
                    cast::unlink(coll);
                    check_ast(coll);
                }
                ast_reduce_cache.erase(ast_reduce_cache.begin() + bk->reduce_index, ast_reduce_cache.end());
                size = ast_coll_cache.size();
                for (auto i = size; i > bk->coll_index; --i) {
                    auto& coll = ast_coll_cache[i - 1];
                    assert(coll->flag == ast_collection);
                    check_ast(coll);
#if DEBUG_AST
                    ATLTRACE("[DEBUG] Backtrace failed, delete coll: %p, PB=%p, CB=%p, NE=%d, CS=%d\n",
                        coll, coll->parent, coll->child,
                        cast::children_size(coll->parent), cast::children_size(coll));
#endif
                    cast::unlink(coll);
                    check_ast(coll);
                    ast->remove(coll);
                }
                ast_coll_cache.erase(ast_coll_cache.begin() + bk->coll_index, ast_coll_cache.end());
                bk->direction = b_fallback;
            }
            trans_id = -1;
        }
    }

    ast_node* cparser::terminal() {
        if (lexer->is_type(l_end) && ast_cache_index >= ast_cache.size()) { // 结尾
            error("unexpected token EOF of expression");
        }
        if (ast_cache_index < ast_cache.size()) {
            return ast_cache[ast_cache_index++];
        }
        if (lexer->is_type(l_operator)) {
            auto node = ast->new_node(ast_operator);
            node->line = lexer->get_last_line();
            node->column = lexer->get_last_column();
            node->data._op = lexer->get_operator();
            match_operator(node->data._op);
            ast_cache.push_back(node);
            ast_cache_index++;
            return node;
        }
        if (lexer->is_type(l_keyword)) {
            auto node = ast->new_node(ast_keyword);
            node->line = lexer->get_last_line();
            node->column = lexer->get_last_column();
            node->data._keyword = lexer->get_keyword();
            match_keyword(node->data._keyword);
            ast_cache.push_back(node);
            ast_cache_index++;
            return node;
        }
        if (lexer->is_type(l_identifier)) {
            auto node = ast->new_node(ast_literal);
            node->line = lexer->get_last_line();
            node->column = lexer->get_last_column();
            ast->set_str(node, lexer->get_identifier());
            match_type(l_identifier);
            ast_cache.push_back(node);
            ast_cache_index++;
            return node;
        }
        if (lexer->is_number()) {
            ast_node* node = nullptr;
            auto type = lexer->get_type();
            switch (type) {
#define DEFINE_NODE_INT(t) \
            case l_##t: \
                node = ast->new_node(ast_##t); \
                node->line = lexer->get_last_line(); \
                node->column = lexer->get_last_column(); \
                node->data._##t = lexer->get_##t(); \
                break;
                DEFINE_NODE_INT(char)
                    DEFINE_NODE_INT(uchar)
                    DEFINE_NODE_INT(short)
                    DEFINE_NODE_INT(ushort)
                    DEFINE_NODE_INT(int)
                    DEFINE_NODE_INT(uint)
                    DEFINE_NODE_INT(long)
                    DEFINE_NODE_INT(ulong)
                    DEFINE_NODE_INT(float)
                    DEFINE_NODE_INT(double)
#undef DEFINE_NODE_INT
            default:
                error("invalid number");
                break;
            }
            match_number();
            ast_cache.push_back(node);
            ast_cache_index++;
            return node;
        }
        if (lexer->is_type(l_string)) {
            std::stringstream ss;
            ss << lexer->get_string();
#if 0
            ATLTRACE("[%04d:%03d] String> %04X '%s'\n", clexer->get_line(), clexer->get_column(), idx, clexer->get_string().c_str());
#endif
            match_type(l_string);

            while (lexer->is_type(l_string)) {
                ss << lexer->get_string();
#if 0
                ATLTRACE("[%04d:%03d] String> %04X '%s'\n", clexer->get_line(), clexer->get_column(), idx, clexer->get_string().c_str());
#endif
                match_type(l_string);
            }
            auto node = ast->new_node(ast_string);
            node->line = lexer->get_last_line();
            node->column = lexer->get_last_column();
            ast->set_str(node, ss.str());
            ast_cache.push_back(node);
            ast_cache_index++;
            return node;
        }
        error("invalid type");
        return nullptr;
    }

    bool cparser::valid_trans(const pda_trans & trans) const {
        auto& la = trans.LA;
        if (!la.empty()) {
            auto success = false;
            for (auto& _la : la) {
                if (LA(_la)) {
                    success = true;
                    break;
                }
            }
            if (!success)
                return false;
        }
        switch (trans.type) {
        case e_reduce:
        case e_reduce_exp: {
            if (ast_stack.size() <= 1)
                return false;
            if (state_stack.empty())
                return false;
            if (trans.status != state_stack.back())
                return false;
        }
                           break;
        default:
            break;
        }
        return true;
    }

    void cparser::do_trans(int state, backtrace_t & bk, const pda_trans & trans) {
        switch (trans.type) {
        case e_shift: {
            state_stack.push_back(state);
            auto new_node = ast->new_node(ast_collection);
            new_node->line = new_node->column = 0;
            auto& pdas = unit.get_pda();
            new_node->data._coll = pdas[trans.jump].coll;
#if DEBUG_AST
            ATLTRACE("[DEBUG] Shift: top=%p, new=%p, CS=%d\n", ast_stack.back(), new_node,
                cast::children_size(ast_stack.back()));
#endif
            ast_coll_cache.push_back(new_node);
            ast_stack.push_back(new_node);
        }
                      break;
        case e_pass: {
            bk.ast_ids.insert(ast_cache_index);
            terminal();
#if CHECK_AST
            check_ast(t);
#endif
#if DEBUG_AST
            ATLTRACE("[DEBUG] Move: parent=%p, child=%p, CS=%d\n", ast_stack.back(), t,
                cast::children_size(ast_stack.back()));
#endif
        }
                     break;
        case e_move: {
            bk.ast_ids.insert(ast_cache_index);
            auto t = terminal();
#if CHECK_AST
            check_ast(t);
#endif
#if DEBUG_AST
            ATLTRACE("[DEBUG] Move: parent=%p, child=%p, CS=%d\n", ast_stack.back(), t,
                cast::children_size(ast_stack.back()));
#endif
            cast::set_child(ast_stack.back(), t);
        }
                     break;
        case e_left_recursion:
            break;
        case e_left_recursion_not_greed:
            break;
        case e_reduce:
        case e_reduce_exp: {
            auto new_ast = ast_stack.back();
            check_ast(new_ast);
            if (new_ast->flag != ast_collection) {
                bk.ast_ids.insert(ast_cache_index);
            }
            state_stack.pop_back();
            ast_stack.pop_back();
            assert(!ast_stack.empty());
            ast_reduce_cache.push_back(new_ast);
#if DEBUG_AST
            ATLTRACE("[DEBUG] Reduce: parent=%p, child=%p, CS=%d, AS=%d, RI=%d\n",
                ast_stack.back(), new_ast, cast::children_size(ast_stack.back()),
                ast_stack.size(), ast_reduce_cache.size());
#endif
            if (trans.type == e_reduce_exp)
                ast_stack.back()->attr |= a_exp;
            cast::set_child(ast_stack.back(), new_ast);
            check_ast(ast_stack.back());
        }
                           break;
        case e_finish:
            state_stack.pop_back();
            break;
        }
    }

    bool cparser::LA(struct unit* u) const {
        if (u->t != u_token)
            return false;
        auto token = to_token(u);
        if (ast_cache_index < ast_cache.size()) {
            auto& cache = ast_cache[ast_cache_index];
            if (token->type == l_keyword)
                return cache->flag == ast_keyword && cache->data._keyword == token->value.keyword;
            if (token->type == l_operator)
                return cache->flag == ast_operator && cache->data._op == token->value.op;
            return cast::ast_equal((ast_t)cache->flag, token->type);
        }
        if (token->type == l_keyword)
            return lexer->is_keyword(token->value.keyword);
        if (token->type == l_operator)
            return lexer->is_operator(token->value.op);
        return lexer->is_type(token->type);
    }

    void cparser::expect(bool flag, const string_t & info) {
        if (!flag) {
            error(info);
        }
    }

    void cparser::match_keyword(keyword_t type) {
        expect(lexer->is_keyword(type), string_t("expect keyword ") + KEYWORD_STRING(type));
        next();
    }

    void cparser::match_operator(operator_t type) {
        expect(lexer->is_operator(type), string_t("expect operator " + OPERATOR_STRING(type)));
        next();
    }

    void cparser::match_type(lexer_t type) {
        expect(lexer->is_type(type), string_t("expect get_type " + LEX_STRING(type)));
        next();
    }

    void cparser::match_number() {
        expect(lexer->is_number(), "expect number");
        next();
    }

    void cparser::match_integer() {
        expect(lexer->is_integer(), "expect integer");
        next();
    }

    void cparser::error(const string_t & info) {
        std::stringstream ss;
        ss << '[' << std::setfill('0') << std::setw(4) << lexer->get_line();
        ss << ':' << std::setfill('0') << std::setw(3) << lexer->get_column();
        ss << ']' << ' ' << info;
        throw cexception(ex_parser, ss.str());
    }
}

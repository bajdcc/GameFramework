//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <fstream>
#include <algorithm>
#include "cjsparser.h"
#include "cjsast.h"

#define REPORT_ERROR 0
#define REPORT_ERROR_FILE "js_parsing.log"

#define TRACE_PARSING 0
#define TRACE_PARSING_LOG 0
#define DUMP_LEXER 0
#define DUMP_PDA 0
#define DUMP_PDA_FILE "js_PDA.txt"
#define DEBUG_AST 0
#define CHECK_AST 0

namespace clib {

    js_ast_node *cjsparser::parse(const std::string &str, std::string &error_string, csemantic *s) {
        semantic = s;
        lexer = std::make_unique<cjslexer>();
        lexer->input(str, error_string);
        if (!error_string.empty()) {
            lexer.reset(nullptr);
            return nullptr;
        }
#if DUMP_LEXER
        lexer->dump();
#endif
        ast = std::make_unique<cjsast>();
        current = nullptr;
        // 清空AST
        ast->reset();
        // 产生式
        if (unit.get_pda().empty())
            gen();
        // 语法分析（递归下降）
        program();
        return ast->get_root();
    }

    js_ast_node *cjsparser::root() const {
        return ast->get_root();
    }

    void cjsparser::clear_ast() {
        ast.reset();
    }

    void cjsparser::next() {
        current = &lexer->get_current_unit();
        lexer->inc_index();
    }

    void cjsparser::gen() {
        // REFER: antlr/grammars-v4
        // URL: https://github.com/antlr/grammars-v4/blob/master/javascript/javascript/JavaScriptParser.g4
#define DEF_LEXER(name) auto &_##name = unit.token(name);
        DEF_LEXER(NUMBER)
        DEF_LEXER(ID)
        DEF_LEXER(REGEX)
        DEF_LEXER(STRING)
        // ---
        DEF_LEXER(RULE_NO_LINE)
        DEF_LEXER(RULE_LINE)
        DEF_LEXER(RULE_RBRACE)
        DEF_LEXER(RULE_EOF)
        // ---
        DEF_LEXER(K_NEW)
        DEF_LEXER(K_VAR)
        DEF_LEXER(K_LET)
        DEF_LEXER(K_FUNCTION)
        DEF_LEXER(K_IF)
        DEF_LEXER(K_ELSE)
        DEF_LEXER(K_FOR)
        DEF_LEXER(K_WHILE)
        DEF_LEXER(K_IN)
        DEF_LEXER(K_DO)
        DEF_LEXER(K_BREAK)
        DEF_LEXER(K_CONTINUE)
        DEF_LEXER(K_RETURN)
        DEF_LEXER(K_SWITCH)
        DEF_LEXER(K_DEFAULT)
        DEF_LEXER(K_CASE)
        DEF_LEXER(K_NULL)
        DEF_LEXER(K_UNDEFINED)
        DEF_LEXER(K_TRUE)
        DEF_LEXER(K_FALSE)
        DEF_LEXER(K_INSTANCEOF)
        DEF_LEXER(K_TYPEOF)
        DEF_LEXER(K_VOID)
        DEF_LEXER(K_DELETE)
        DEF_LEXER(K_CLASS)
        DEF_LEXER(K_THIS)
        DEF_LEXER(K_SUPER)
        DEF_LEXER(K_WITH)
        DEF_LEXER(K_TRY)
        DEF_LEXER(K_THROW)
        DEF_LEXER(K_CATCH)
        DEF_LEXER(K_FINALLY)
        DEF_LEXER(K_DEBUGGER)
        // ---
        DEF_LEXER(T_ADD)
        DEF_LEXER(T_SUB)
        DEF_LEXER(T_MUL)
        DEF_LEXER(T_DIV)
        DEF_LEXER(T_MOD)
        DEF_LEXER(T_POWER)
        DEF_LEXER(T_INC)
        DEF_LEXER(T_DEC)
        DEF_LEXER(T_ASSIGN)
        DEF_LEXER(T_ASSIGN_ADD)
        DEF_LEXER(T_ASSIGN_SUB)
        DEF_LEXER(T_ASSIGN_MUL)
        DEF_LEXER(T_ASSIGN_DIV)
        DEF_LEXER(T_ASSIGN_MOD)
        DEF_LEXER(T_ASSIGN_LSHIFT)
        DEF_LEXER(T_ASSIGN_RSHIFT)
        DEF_LEXER(T_ASSIGN_URSHIFT)
        DEF_LEXER(T_ASSIGN_AND)
        DEF_LEXER(T_ASSIGN_OR)
        DEF_LEXER(T_ASSIGN_XOR)
        DEF_LEXER(T_ASSIGN_POWER)
        DEF_LEXER(T_LESS)
        DEF_LEXER(T_LESS_EQUAL)
        DEF_LEXER(T_GREATER)
        DEF_LEXER(T_GREATER_EQUAL)
        DEF_LEXER(T_EQUAL)
        DEF_LEXER(T_FEQUAL)
        DEF_LEXER(T_NOT_EQUAL)
        DEF_LEXER(T_FNOT_EQUAL)
        DEF_LEXER(T_LOG_NOT)
        DEF_LEXER(T_LOG_AND)
        DEF_LEXER(T_LOG_OR)
        DEF_LEXER(T_BIT_NOT)
        DEF_LEXER(T_BIT_AND)
        DEF_LEXER(T_BIT_OR)
        DEF_LEXER(T_BIT_XOR)
        DEF_LEXER(T_DOT)
        DEF_LEXER(T_COMMA)
        DEF_LEXER(T_SEMI)
        DEF_LEXER(T_COLON)
        DEF_LEXER(T_QUERY)
        DEF_LEXER(T_LSHIFT)
        DEF_LEXER(T_RSHIFT)
        DEF_LEXER(T_URSHIFT)
        DEF_LEXER(T_LPARAN)
        DEF_LEXER(T_RPARAN)
        DEF_LEXER(T_LSQUARE)
        DEF_LEXER(T_RSQUARE)
        DEF_LEXER(T_LBRACE)
        DEF_LEXER(T_RBRACE)
        DEF_LEXER(T_COALESCE)
        DEF_LEXER(T_SHARP)
        DEF_LEXER(T_ELLIPSIS)
        DEF_LEXER(T_ARROW)
#undef DEF_LEXER
#define DEF_RULE(name) auto &name = unit.rule(#name, c_##name);
#define DEF_RULE_ATTR(name, attr) auto &name = unit.rule(#name, c_##name, attr);
#define DEF_RULE_NOT_GREED(name) DEF_RULE_ATTR(name, r_not_greed)
#define DEF_RULE_EXP(name) DEF_RULE_ATTR(name, r_exp)
        DEF_RULE(program)
        DEF_RULE(sourceElement)
        DEF_RULE(statement)
        DEF_RULE(block)
        DEF_RULE(statementList)
        DEF_RULE(variableStatement)
        DEF_RULE(variableDeclarationList)
        DEF_RULE(variableDeclaration)
        DEF_RULE(emptyStatement)
        DEF_RULE(expressionStatement)
        DEF_RULE(ifStatement)
        DEF_RULE(iterationStatement)
        DEF_RULE(doStatement)
        DEF_RULE(whileStatement)
        DEF_RULE(forStatement)
        DEF_RULE(forInStatement)
        DEF_RULE(continueStatement)
        DEF_RULE(breakStatement)
        DEF_RULE(returnStatement)
        DEF_RULE(withStatement)
        DEF_RULE(switchStatement)
        DEF_RULE(functionStatement)
        DEF_RULE(caseBlock)
        DEF_RULE(caseClauses)
        DEF_RULE(caseClause)
        DEF_RULE(defaultClause)
        DEF_RULE(labelledStatement)
        DEF_RULE(throwStatement)
        DEF_RULE(tryStatement)
        DEF_RULE(catchProduction)
        DEF_RULE(finallyProduction)
        DEF_RULE(debuggerStatement)
        DEF_RULE(functionDeclaration)
        DEF_RULE(classDeclaration)
        DEF_RULE(classTail)
        DEF_RULE(classElement)
        DEF_RULE(classElements)
        DEF_RULE(methodDefinition)
        DEF_RULE(formalParameterList)
        DEF_RULE(formalParameterArg)
        DEF_RULE(lastFormalParameterArg)
        DEF_RULE(functionBody)
        DEF_RULE(sourceElements)
        DEF_RULE(arrayLiteral)
        DEF_RULE(elementList)
        DEF_RULE(arrayElement)
        DEF_RULE(commaList)
        DEF_RULE(objectLiteral)
        DEF_RULE(propertyAssignment)
        DEF_RULE(propertyAssignments)
        DEF_RULE(propertyName)
        DEF_RULE(arguments)
        DEF_RULE(argument)
        DEF_RULE(expressionSequence)
        DEF_RULE_EXP(singleExpression)
        DEF_RULE(assignable)
        DEF_RULE(anonymousFunction)
        DEF_RULE(arrowFunctionParameters)
        DEF_RULE(arrowFunctionBody)
        DEF_RULE(literal)
        DEF_RULE(numericLiteral)
        DEF_RULE(identifierName)
        DEF_RULE(reservedWord)
        DEF_RULE(keyword)
        DEF_RULE(eos)
        DEF_RULE(propertyExpressionAssignment)
        DEF_RULE(computedPropertyExpressionAssignment)
        DEF_RULE(propertyShorthand)
        DEF_RULE(functionDecl)
        DEF_RULE(anonymousFunctionDecl)
        DEF_RULE(arrowFunction)
        DEF_RULE_EXP(functionExpression)
        DEF_RULE_EXP(classExpression)
        DEF_RULE(memberIndexExpression)
        DEF_RULE(memberDotExpression)
        DEF_RULE(argumentsExpression)
        DEF_RULE(newExpression)
        DEF_RULE_EXP(newExpressionArgument)
        DEF_RULE_EXP(primaryExpression)
        DEF_RULE(prefixExpression)
        DEF_RULE(prefixExpressionList)
        DEF_RULE(postIncrementExpression)
        DEF_RULE(postDecreaseExpression)
        DEF_RULE(postfixExpression)
        DEF_RULE(deleteExpression)
        DEF_RULE(voidExpression)
        DEF_RULE(typeofExpression)
        DEF_RULE(preIncrementExpression)
        DEF_RULE(preDecreaseExpression)
        DEF_RULE(unaryPlusExpression)
        DEF_RULE(unaryMinusExpression)
        DEF_RULE(bitNotExpression)
        DEF_RULE(notExpression)
        DEF_RULE_EXP(powerExpression)
        DEF_RULE_EXP(multiplicativeExpression)
        DEF_RULE_EXP(additiveExpression)
        DEF_RULE_EXP(coalesceExpression)
        DEF_RULE_EXP(bitShiftExpression)
        DEF_RULE_EXP(relationalExpression)
        DEF_RULE_EXP(instanceofExpression)
        DEF_RULE_EXP(inExpression)
        DEF_RULE_EXP(equalityExpression)
        DEF_RULE_EXP(bitAndExpression)
        DEF_RULE_EXP(bitXOrExpression)
        DEF_RULE_EXP(bitOrExpression)
        DEF_RULE_EXP(logicalAndExpression)
        DEF_RULE_EXP(logicalOrExpression)
        DEF_RULE_EXP(ternaryExpression)
        DEF_RULE_EXP(assignmentExpression)
        DEF_RULE_EXP(assignmentOperatorExpression)
        DEF_RULE_EXP(thisExpression)
        DEF_RULE_EXP(identifierExpression)
        DEF_RULE_EXP(superExpression)
        DEF_RULE_EXP(literalExpression)
        DEF_RULE_EXP(arrayLiteralExpression)
        DEF_RULE_EXP(objectLiteralExpression)
        DEF_RULE_EXP(parenthesizedExpression)
#undef DEF_RULE
#undef DEF_RULE_NOT_GREED
#undef DEF_RULE_EXP
        program = sourceElements;
        variableStatement = _K_VAR + variableDeclarationList + eos;
        variableDeclarationList = *(variableDeclarationList + ~_T_COMMA) + variableDeclaration;
        variableDeclaration = assignable + *(~_T_ASSIGN + singleExpression);
        emptyStatement = _T_SEMI;
        expressionStatement = expressionSequence + eos;
        ifStatement = _K_IF + ~_T_LPARAN + expressionSequence + ~_T_RPARAN + statement + *(~_K_ELSE + statement);
        iterationStatement = doStatement | whileStatement | forStatement | forInStatement;
        doStatement = _K_DO + statement + ~_K_WHILE + ~_T_LPARAN + expressionSequence + ~_T_RPARAN + eos;
        whileStatement = _K_WHILE + ~_T_LPARAN + expressionSequence + ~_T_RPARAN + statement;
        forStatement = _K_FOR + ~_T_LPARAN + *(expressionSequence | (_K_VAR + variableDeclarationList)) +
                       _T_SEMI + *expressionSequence + _T_SEMI + *expressionSequence + ~_T_RPARAN + statement;
        forInStatement = _K_FOR + ~_T_LPARAN + (singleExpression | (_K_VAR + variableDeclarationList)) +
                         ~_K_IN + expressionSequence + ~_T_RPARAN + statement;
        continueStatement = _K_CONTINUE + *(_RULE_NO_LINE + _ID) + eos;
        breakStatement = _K_BREAK + *(_RULE_NO_LINE + _ID) + eos;
        returnStatement = _K_RETURN + *(_RULE_NO_LINE + expressionSequence) + eos;
        withStatement = _K_WITH + ~_T_LPARAN + expressionSequence + ~_T_RPARAN + statement;
        switchStatement = _K_SWITCH + ~_T_LPARAN + expressionSequence + ~_T_RPARAN + caseBlock;
        caseBlock = ~_T_LBRACE + *caseClauses + *(defaultClause + *caseClauses) + _T_RBRACE;
        caseClauses = caseClause + *caseClauses;
        caseClause = _K_CASE + expressionSequence + _T_COLON + *statementList;
        defaultClause = _K_DEFAULT + _T_COLON + *statementList;
        labelledStatement = _ID + _T_COLON + statement;
        throwStatement = _K_THROW + _RULE_NO_LINE + expressionSequence + eos;
        tryStatement = _K_TRY + block + ((catchProduction + *finallyProduction) | finallyProduction);
        catchProduction = ~_K_CATCH + *(~_T_LPARAN + *_ID + ~_T_RPARAN) + block;
        finallyProduction = ~_K_FINALLY + block;
        debuggerStatement = _K_DEBUGGER + eos;
        classDeclaration = _K_CLASS + _ID + classTail;
        classTail = ~_T_LBRACE + classElements + ~_T_RBRACE;
        classElements = *(classElements + ~_T_COMMA) + classElement;
        classElement = methodDefinition
                       | emptyStatement
                       | (*_T_SHARP + propertyName + _T_ASSIGN + singleExpression);
        methodDefinition = *_T_SHARP + propertyName + ~_T_LPARAN + *formalParameterList + ~_T_RPARAN +
                           ~_T_LBRACE + *functionBody + ~_T_RBRACE;
        formalParameterList = (formalParameterArg + *(~_T_COMMA + lastFormalParameterArg)) |
                              lastFormalParameterArg;
        formalParameterArg = *(formalParameterArg + ~_T_COMMA) + assignable;
        lastFormalParameterArg = _T_ELLIPSIS + assignable;
        functionBody = *functionBody + sourceElements;
        sourceElements = *sourceElements + statement;
        sourceElement = statement;
        statement = block
                    | variableStatement
                    | emptyStatement
                    | classDeclaration
                    | expressionStatement
                    | ifStatement
                    | iterationStatement
                    | continueStatement
                    | breakStatement
                    | returnStatement
                    | withStatement
                    | labelledStatement
                    | switchStatement
                    | throwStatement
                    | tryStatement
                    | debuggerStatement
                    | functionStatement;
        block = _T_LBRACE + *statementList + _T_RBRACE;
        statementList = *statementList + statement;
        expressionStatement = expressionSequence + eos;
        expressionSequence = *(expressionSequence + ~_T_COMMA) + singleExpression;
        thisExpression = _K_THIS;
        identifierExpression = _ID;
        superExpression = _K_SUPER;
        literalExpression = literal;
        arrayLiteralExpression = arrayLiteral;
        objectLiteralExpression = objectLiteral;
        parenthesizedExpression = _T_LPARAN + expressionSequence + _T_RPARAN;
        newExpression = newExpressionArgument | (_K_NEW + singleExpression);
        newExpressionArgument = _K_NEW + singleExpression + arguments;
        functionExpression = anonymousFunction
                             | classExpression
                             | thisExpression
                             | identifierExpression
                             | superExpression
                             | literalExpression
                             | arrayLiteralExpression
                             | objectLiteralExpression
                             | parenthesizedExpression
                             | newExpression;
        classExpression = _K_CLASS + ~_ID + classTail;
        memberIndexExpression = _T_LSQUARE + expressionSequence + _T_RSQUARE;
        memberDotExpression = *_T_QUERY + ~_T_DOT + *_T_SHARP + identifierName;
        argumentsExpression = arguments;
        postIncrementExpression = _T_INC;
        postDecreaseExpression = _T_DEC;
        postfixExpression = *postfixExpression +
                            ((_RULE_NO_LINE + memberIndexExpression)
                             | memberDotExpression
                             | argumentsExpression
                             | (_RULE_NO_LINE + postIncrementExpression)
                             | (_RULE_NO_LINE + postDecreaseExpression));
        deleteExpression = _K_DELETE;
        voidExpression = _K_VOID;
        typeofExpression = _K_TYPEOF;
        preIncrementExpression = _T_INC;
        preDecreaseExpression = _T_DEC;
        unaryPlusExpression = _T_ADD;
        unaryMinusExpression = _T_SUB;
        bitNotExpression = _T_BIT_NOT;
        notExpression = _T_LOG_NOT;
        primaryExpression = functionExpression + *postfixExpression;
        prefixExpressionList = *prefixExpressionList +
                               (deleteExpression
                                | voidExpression
                                | typeofExpression
                                | preIncrementExpression
                                | preDecreaseExpression
                                | unaryPlusExpression
                                | unaryMinusExpression
                                | bitNotExpression
                                | notExpression);
        prefixExpression = *prefixExpressionList + primaryExpression;
        powerExpression = *(powerExpression + _T_POWER) + prefixExpression;
        multiplicativeExpression = *(multiplicativeExpression + (_T_MUL | _T_DIV | _T_MOD)) + powerExpression;
        additiveExpression = *(additiveExpression + (_T_ADD | _T_SUB)) + multiplicativeExpression;
        coalesceExpression = *(coalesceExpression + (_T_COALESCE)) + additiveExpression;
        bitShiftExpression = *(bitShiftExpression + (_T_LSHIFT | _T_RSHIFT | _T_URSHIFT)) + coalesceExpression;
        relationalExpression = *(relationalExpression + (_T_LESS | _T_LESS_EQUAL | _T_GREATER | _T_GREATER_EQUAL)) +
                               bitShiftExpression;
        instanceofExpression = *(instanceofExpression + _K_INSTANCEOF) + relationalExpression;
        inExpression = *(inExpression + _K_IN) + instanceofExpression;
        equalityExpression = *(equalityExpression + (_T_EQUAL | _T_NOT_EQUAL | _T_FEQUAL | _T_FNOT_EQUAL)) +
                             inExpression;
        bitAndExpression = *(bitAndExpression + _T_BIT_AND) + equalityExpression;
        bitXOrExpression = *(bitXOrExpression + _T_BIT_XOR) + bitAndExpression;
        bitOrExpression = *(bitOrExpression + _T_BIT_OR) + bitXOrExpression;
        logicalAndExpression = *(logicalAndExpression + _T_LOG_AND) + bitOrExpression;
        logicalOrExpression = *(logicalOrExpression + _T_LOG_OR) + logicalAndExpression;
        ternaryExpression = logicalOrExpression + *(_T_QUERY + singleExpression + _T_COLON + singleExpression);
        assignmentOperatorExpression = *(assignmentOperatorExpression +
                                         (_T_ASSIGN_ADD | _T_ASSIGN_SUB | _T_ASSIGN_MUL | _T_ASSIGN_DIV |
                                          _T_ASSIGN_MOD | _T_ASSIGN_LSHIFT | _T_ASSIGN_RSHIFT | _T_ASSIGN_URSHIFT |
                                          _T_ASSIGN_AND | _T_ASSIGN_OR | _T_ASSIGN_XOR | _T_ASSIGN_POWER)) +
                                       ternaryExpression;
        assignmentExpression = *(assignmentExpression + _T_ASSIGN) + assignmentOperatorExpression;
        singleExpression = assignmentExpression;
        literal = _K_NULL | _K_UNDEFINED | _K_TRUE | _K_FALSE | _STRING | _REGEX | _NUMBER;
        commaList = *commaList + _T_COMMA;
        arrayLiteral = _T_LSQUARE + *elementList + _T_RSQUARE;
        elementList = *(elementList) + *commaList + arrayElement + *commaList;
        arrayElement = *_T_ELLIPSIS + singleExpression;
        objectLiteral = _T_LBRACE + *propertyAssignments + *~_T_COMMA + _T_RBRACE;
        identifierName = _ID | reservedWord;
        reservedWord = keyword | _K_TRUE | _K_FALSE;
        numericLiteral = _NUMBER;
        assignable = _ID | arrayLiteral | objectLiteral;
        arguments = ~_T_LPARAN + *argument + _T_RPARAN;
        argument = *(argument + ~_T_COMMA) + *_T_ELLIPSIS + singleExpression;
        propertyAssignments = *(propertyAssignments + ~_T_COMMA) + propertyAssignment;
        propertyAssignment = propertyExpressionAssignment
                             | computedPropertyExpressionAssignment
                             | propertyShorthand;
        propertyExpressionAssignment = propertyName + ~_T_COLON + singleExpression;
        computedPropertyExpressionAssignment = ~_T_LPARAN + singleExpression + ~_T_RPARAN +
                                               ~_T_COLON + singleExpression;
        propertyShorthand = _T_ELLIPSIS + singleExpression;
        propertyName = identifierName
                       | _STRING
                       | numericLiteral
                       | (~_T_LSQUARE + singleExpression + ~_T_RSQUARE);
        functionStatement = anonymousFunction;
        anonymousFunction = functionDecl
                            | anonymousFunctionDecl
                            | arrowFunction;
        functionDecl = functionDeclaration;
        functionDeclaration = _K_FUNCTION + _ID + ~_T_LPARAN + *formalParameterList + ~_T_RPARAN +
                              ~_T_LBRACE + *functionBody + _T_RBRACE;
        anonymousFunctionDecl = _K_FUNCTION + ~_T_LPARAN + *formalParameterList + ~_T_RPARAN +
                                ~_T_LBRACE + *functionBody + _T_RBRACE;
        arrowFunction = arrowFunctionParameters + ~_T_ARROW + arrowFunctionBody;
        arrowFunctionParameters = _ID | (_T_LPARAN + *formalParameterList + _T_RPARAN);
        arrowFunctionBody = singleExpression | (~_T_LBRACE + *functionBody + _T_RBRACE);
        eos = (~~_T_SEMI)((void *) &clear_bk) | _RULE_EOF | _RULE_LINE | _RULE_RBRACE;
        keyword = _K_BREAK
                  | _K_DO
                  | _K_INSTANCEOF
                  | _K_TYPEOF
                  | _K_CASE
                  | _K_ELSE
                  | _K_NEW
                  | _K_VAR
                  | _K_CATCH
                  | _K_FINALLY
                  | _K_RETURN
                  | _K_VOID
                  | _K_CONTINUE
                  | _K_FOR
                  | _K_SWITCH
                  | _K_WHILE
                  | _K_DEBUGGER
                  | _K_FUNCTION
                  | _K_THIS
                  | _K_WITH
                  | _K_DEFAULT
                  | _K_IF
                  | _K_THROW
                  | _K_DELETE
                  | _K_IN
                  | _K_TRY
                  | _K_CLASS
                  | _K_SUPER
                  | _K_LET;
        unit.adjust(&functionExpression, &anonymousFunction, e_shift, -1);
        unit.adjust(&iterationStatement, &forInStatement, e_shift, -1);
        unit.adjust(&iterationStatement, &forStatement, e_shift, 0, (void *) &pred_for);
        unit.adjust(&newExpression, &newExpressionArgument, e_shift, 1);
        unit.adjust(&inExpression, &inExpression, e_left_recursion, 0, (void *) &pred_in);
        unit.gen(&program);
#if DUMP_PDA
        std::ofstream of(DUMP_PDA_FILE);
        unit.dump(of);
#endif
    }

    static void check_ast(js_ast_node *node) {
#if CHECK_AST
        if (node->child) {
            auto &c = node->child;
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
            } else {
                assert(i->prev == i);
            }
        }
#endif
    }

    void cjsparser::program() {
#if REPORT_ERROR
        std::ofstream log(REPORT_ERROR_FILE, std::ios::app | std::ios::out);
#endif
        next();
        state_stack.clear();
        ast_stack.clear();
        ast_cache.clear();
        ast_cache_index = 0;
        ast_coll_cache.clear();
        ast_reduce_cache.clear();
        state_stack.push_back(0);
        const auto &pdas = unit.get_pda();
        auto root = ast->new_node(a_collection);
        root->line = root->column = 0;
        root->data._coll = pdas[0].coll;
        cjsast::set_child(ast->get_root(), root);
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
        while (!bks.empty()) {
            auto bk = &bks.back();
            if (bk->direction == b_success || bk->direction == b_fail) {
                break;
            }
            if (bk->direction == b_fallback) {
                if (bk->trans_ids.empty()) {
                    if (bks.size() > 1) {
                        bks.pop_back();
                        bks.back().direction = b_error;
                        bk = &bks.back();
                        if (bk->lexer_index < prev_idx) {
                            bk->direction = b_fail;
                            continue;
                        }
                    } else {
                        bk->direction = b_fail;
                        continue;
                    }
                }
            }
            ast_cache_index = (size_t) bk->lexer_index;
            state_stack = bk->state_stack;
            ast_stack = bk->ast_stack;
            auto state = bk->current_state;

#if TRACE_PARSING && !TRACE_PARSING_LOG
            {
                auto line = 0;
                auto column = 0;
                if (!ast_cache.empty() && ast_cache_index > 0 && ast_cache_index <= ast_cache.size()) {
                    line = ast_cache[ast_cache_index - 1]->line;
                    column = ast_cache[ast_cache_index - 1]->column;
                }
                fprintf(stdout, "[%d:%d:%d:%d:%d] State: %3d\n",
                        ast_cache_index, ast_stack.size(), bks.size(),
                        line, column, state);
            };
#endif
            if (bk->direction != b_error)
                for (;;) {
                    auto is_end = current->t == END && ast_cache_index >= ast_cache.size();
                    const auto &current_state = pdas[state];
                    if (is_end) {
                        if (current_state.final) {
                            if (state_stack.empty()) {
                                bk->direction = b_success;
                                break;
                            }
                        }
                    }
                    auto &trans = current_state.trans;
                    if (trans_id == -1 && !bk->trans_ids.empty()) {
                        trans_id = bk->trans_ids.back() & ((1 << 16) - 1);
                        bk->trans_ids.pop_back();
                    } else {
                        trans_ids.clear();
                        if (is_end) {
                            for (size_t i = 0; i < trans.size(); ++i) {
                                auto &cs = trans[i];
                                if (valid_trans(cs) && (cs.type != e_move && cs.type != e_pass)) {
                                    trans_ids.push_back(i);
                                }
                            }
                        } else {
                            for (size_t i = 0; i < trans.size(); ++i) {
                                auto &cs = trans[i];
                                if (valid_trans(cs)) {
                                    trans_ids.push_back(i);
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
                            if (current_state.pred) {
                                std::vector<int> then;
                                std::vector<int> add;
                                for (const auto t : trans_ids) {
                                    if (trans[t].pred) {
                                        auto cb = (pda_coll_pred_cb) trans[t].pred;
                                        auto r = cb(lexer.get(), current->id);
                                        if (r == p_ALLOW) {
                                            add.push_back(t);
                                        } else if (r == p_DELAY) {
                                            then.push_back(t);
                                        }
                                    } else {
                                        add.push_back(t);
                                    }
                                }
                                if (trans_ids.size() > add.size()) {
                                    trans_ids = add;
                                    std::copy(trans_ids.cbegin(), trans_ids.cend(), back_inserter(then));
                                }
                            }
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
                                    auto &_bk = bks[i];
                                    fprintf(stdout,
                                            "[DEBUG] Branch old: i=%d, LI=%d, SS=%d, AS=%d, S=%d, TS=%d, CI=%d, RI=%d, TK=%d\n",
                                            i, _bk.lexer_index, _bk.state_stack.size(),
                                            _bk.ast_stack.size(), _bk.current_state, _bk.trans_ids.size(),
                                            _bk.coll_index, _bk.reduce_index, _bk.ast_ids.size());
                                }
#endif
                                bks.push_back(bk_tmp);
                                bk = &bks.back();
#if DEBUG_AST
                                fprintf(stdout,
                                        "[DEBUG] Branch new: BS=%d, LI=%d, SS=%d, AS=%d, S=%d, TS=%d, CI=%d, RI=%d, TK=%d\n",
                                        bks.size(), bk_tmp.lexer_index, bk_tmp.state_stack.size(),
                                        bk_tmp.ast_stack.size(), bk_tmp.current_state, bk_tmp.trans_ids.size(),
                                        bk_tmp.coll_index, bk_tmp.reduce_index, bk_tmp.ast_ids.size());
#endif
                                bk->direction = b_next;
                                break;
                            } else {
                                trans_id = trans_ids.back();
                                trans_ids.pop_back();
                            }
                        } else {
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
                    auto &t = trans[trans_id];
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
                    } else if (current_state.cb && t.cb != nullptr) {
                        ((terminal_cb) (t.cb))(lexer.get(), current->id, bks, bk);
                    }
                    auto jump = trans[trans_id].jump;
#if TRACE_PARSING && TRACE_PARSING_LOG
                    {
                        auto line = 0;
                        auto column = 0;
                        if (!ast_cache.empty() && ast_cache_index > 0 && ast_cache_index <= ast_cache.size()) {
                            line = ast_cache[ast_cache_index - 1]->line;
                            column = ast_cache[ast_cache_index - 1]->column;
                        }
                        fprintf(stdout, "[%d:%d:%d:%d:%d]%s State: %3d => To: %3d   -- Action: %-10s -- Rule: %s\n",
                                ast_cache_index, ast_stack.size(), bks.size(),
                                line, column, is_end ? "*" : "", state, jump,
                                pda_edge_str(t.type).c_str(), current_state.label.c_str());
                    };
#endif
#if REPORT_ERROR
                    {
                        static char fmt[256];
                        int line = 0, column = 0;
                        if (ast_cache_index < ast_cache.size()) {
                            line = ast_cache[ast_cache_index]->line;
                            column = ast_cache[ast_cache_index]->column;
                        } else {
                            line = current->line;
                            column = current->column;
                        }
                        snprintf(fmt, sizeof(fmt),
                                 "[%d,%d:%d:%d:%d]%s State: %3d => To: %3d   -- Action: %-10s -- Rule: %s",
                                 line, column, ast_cache_index, ast_stack.size(),
                                 bks.size(), is_end ? "*" : "", state, jump,
                                 pda_edge_str(t.type).c_str(), current_state.label.c_str());
                        log << fmt << std::endl;
                    }
#endif
                    do_trans(state, *bk, trans[trans_id]);
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
                    auto &_bk = bks[i];
                    fprintf(stdout,
                            "[DEBUG] Backtrace failed: i=%d, LI=%d, SS=%d, AS=%d, S=%d, TS=%d, CI=%d, RI=%d, TK=%d\n",
                            i, _bk.lexer_index, _bk.state_stack.size(),
                            _bk.ast_stack.size(), _bk.current_state, _bk.trans_ids.size(),
                            _bk.coll_index, _bk.reduce_index, _bk.ast_ids.size());
                }
#endif
                for (auto &i : bk->ast_ids) {
                    auto &token = ast_cache[i];
                    check_ast(token);
#if DEBUG_AST
                    fprintf(stdout, "[DEBUG] Backtrace failed, unlink token: %p, PB=%p\n", token, token->parent);
#endif
                    cjsast::unlink(token);
                    check_ast(token);
                }
                auto size = ast_reduce_cache.size();
                for (auto i = size; i > bk->reduce_index; --i) {
                    auto &coll = ast_reduce_cache[i - 1];
                    check_ast(coll);
#if DEBUG_AST
                    fprintf(stdout, "[DEBUG] Backtrace failed, unlink: %p, PB=%p, NE=%d, CB=%d\n",
                            coll, coll->parent, cjsast::children_size(coll->parent), cjsast::children_size(coll));
#endif
                    cjsast::unlink(coll);
                    check_ast(coll);
                }
                ast_reduce_cache.erase(ast_reduce_cache.begin() + bk->reduce_index, ast_reduce_cache.end());
                size = ast_coll_cache.size();
                for (auto i = size; i > bk->coll_index; --i) {
                    auto &coll = ast_coll_cache[i - 1];
                    assert(coll->flag == a_collection);
                    check_ast(coll);
#if DEBUG_AST
                    fprintf(stdout, "[DEBUG] Backtrace failed, delete coll: %p, PB=%p, CB=%p, NE=%d, CS=%d\n",
                            coll, coll->parent, coll->child,
                            cjsast::children_size(coll->parent), cjsast::children_size(coll));
#endif
                    cjsast::unlink(coll);
                    check_ast(coll);
                    ast->remove(coll);
                }
                ast_coll_cache.erase(ast_coll_cache.begin() + bk->coll_index, ast_coll_cache.end());
                bk->direction = b_fallback;
            }
            trans_id = -1;
        }
    }

    js_ast_node *cjsparser::terminal() {
        if (current->t == END && ast_cache_index >= ast_cache.size()) { // 结尾
            error("unexpected token EOF of expression");
        }
        if (ast_cache_index < ast_cache.size()) {
            return ast_cache[ast_cache_index++];
        }
        if (current->t > OPERATOR_START && current->t < OPERATOR_END) {
            auto node = ast->new_node(a_operator);
            node->line = current->line;
            node->column = current->column;
            node->start = current->start;
            node->end = current->end;
            node->data._op = current->t;
            match_type(node->data._op);
            ast_cache.push_back(node);
            ast_cache_index++;
            return node;
        }
        if (current->t > KEYWORD_START && current->t < KEYWORD_END) {
            auto node = ast->new_node(a_keyword);
            node->line = current->line;
            node->column = current->column;
            node->start = current->start;
            node->end = current->end;
            node->data._keyword = current->t;
            match_type(node->data._keyword);
            ast_cache.push_back(node);
            ast_cache_index++;
            return node;
        }
        if (current->t == ID) {
            auto node = ast->new_node(a_literal);
            node->line = current->line;
            node->column = current->column;
            node->start = current->start;
            node->end = current->end;
            ast->set_str(node, lexer->get_data(current->idx));
            match_type(current->t);
            ast_cache.push_back(node);
            ast_cache_index++;
            return node;
        }
        if (current->t == NUMBER) {
            auto node = ast->new_node(a_number);
            node->line = current->line;
            node->column = current->column;
            node->start = current->start;
            node->end = current->end;
            node->data._number = *(double *) lexer->get_data(current->idx);
            match_type(current->t);
            ast_cache.push_back(node);
            ast_cache_index++;
            return node;
        }
        if (current->t == STRING) {
            auto node = ast->new_node(a_string);
            node->line = current->line;
            node->column = current->column;
            node->start = current->start;
            node->end = current->end;
            std::stringstream ss;
            ss << lexer->get_data(current->idx);
            match_type(current->t);

            while (current->t == STRING) {
                ss << lexer->get_data(current->idx);
                match_type(current->t);
            }
            ast->set_str(node, ss.str());
            ast_cache.push_back(node);
            ast_cache_index++;
            return node;
        }
        if (current->t == REGEX) {
            auto node = ast->new_node(a_regex);
            node->line = current->line;
            node->column = current->column;
            node->start = current->start;
            node->end = current->end;
            ast->set_str(node, lexer->get_data(current->idx));
            match_type(current->t);
            ast_cache.push_back(node);
            ast_cache_index++;
            return node;
        }
        if (current->t > RULE_START && current->t < RULE_END) {
            auto node = ast->new_node(a_rule);
            node->line = current->line;
            node->column = current->column;
            node->start = current->start;
            node->end = current->end;
            node->data._ins._1 = current->t;
            node->data._ins._2 = (uint32_t) current->id;
            match_type(current->t);
            ast_cache.push_back(node);
            ast_cache_index++;
            return node;
        }
        error("invalid type");
        return nullptr;
    }

    bool cjsparser::valid_trans(const js_pda_trans &trans) const {
        auto &la = trans.LA;
        if (!la.empty()) {
            auto success = false;
            for (auto &_la : la) {
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

    void cjsparser::do_trans(int state, backtrace_t &bk, const js_pda_trans &trans) {
        switch (trans.type) {
            case e_shift: {
                state_stack.push_back(state);
                auto new_node = ast->new_node(a_collection);
                new_node->line = new_node->column = 0;
                auto &pdas = unit.get_pda();
                new_node->data._coll = pdas[trans.jump].coll;
#if DEBUG_AST
                fprintf(stdout, "[DEBUG] Shift: top=%p, new=%p, CS=%d\n", ast_stack.back(), new_node,
                        cjsast::children_size(ast_stack.back()));
#endif
                ast_coll_cache.push_back(new_node);
                ast_stack.push_back(new_node);
            }
                break;
            case e_pass: {
                bk.ast_ids.insert(ast_cache_index);
#if CHECK_AST
                auto t = terminal();
                check_ast(t);
#else
                terminal();
#endif
#if DEBUG_AST
                fprintf(stdout, "[DEBUG] Move: parent=%p, child=%p, CS=%d\n", ast_stack.back(), t,
                        cjsast::children_size(ast_stack.back()));
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
                fprintf(stdout, "[DEBUG] Move: parent=%p, child=%p, CS=%d\n", ast_stack.back(), t,
                        cjsast::children_size(ast_stack.back()));
#endif
                cjsast::set_child(ast_stack.back(), t);
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
                if (new_ast->flag != a_collection) {
                    bk.ast_ids.insert(ast_cache_index);
                }
                state_stack.pop_back();
                ast_stack.pop_back();
                assert(!ast_stack.empty());
#if DEBUG_AST
                fprintf(stdout, "[DEBUG] Reduce: parent=%p, child=%p, CS=%d, AS=%d, RI=%d\n",
                        ast_stack.back(), new_ast, cjsast::children_size(ast_stack.back()),
                        ast_stack.size(), ast_reduce_cache.size());
#endif
                if (trans.type == e_reduce_exp)
                    ast_stack.back()->attr |= a_exp;
                if (new_ast->flag != a_collection || new_ast->child != nullptr) {
                    ast_reduce_cache.push_back(new_ast);
                    cjsast::set_child(ast_stack.back(), new_ast);
                    check_ast(ast_stack.back());
                } else {
                    cjsast::unlink(new_ast);
                    ast->remove(new_ast);
                }
            }
                break;
            case e_finish:
                state_stack.pop_back();
                break;
            default:
                break;
        }
    }

    bool cjsparser::LA(struct js_unit *u) const {
        if (u->t != u_token)
            return false;
        auto token = js_to_token(u);
        if (token->type > RULE_START && token->type < RULE_END) {
            if (ast_cache_index < ast_cache.size()) {
                auto &cache = ast_cache[ast_cache_index];
                if (cache->flag == a_rule) {
                    return lexer->valid_rule(cache->data._ins._2, (js_lexer_t) cache->data._ins._1);
                }
                return false;
            }
            return lexer->valid_rule(current->id, token->type);
        }
        if (ast_cache_index < ast_cache.size()) {
            auto &cache = ast_cache[ast_cache_index];
            if (token->type > KEYWORD_START && token->type < KEYWORD_END)
                return cache->flag == a_keyword && cache->data._keyword == token->type;
            if (token->type > OPERATOR_START && token->type < OPERATOR_END)
                return cache->flag == a_operator && cache->data._op == token->type;
            return cjsast::ast_equal((js_ast_t) cache->flag, token->type);
        }
        return current->t == token->type;
    }

    void cjsparser::expect(bool flag, const std::string &info) {
        if (!flag) {
            error(info);
        }
    }

    void cjsparser::match_type(js_lexer_t type) {
        expect(current->t == type, std::string("expect get_type ") + js_lexer_string(type));
        next();
    }

    void cjsparser::error(const std::string &info) {
        std::stringstream ss;
        ss << '[' << std::setfill('0') << std::setw(4) << current->line;
        ss << ':' << std::setfill('0') << std::setw(3) << current->column;
        ss << ']' << ' ' << info;
        throw cjs_exception(ss.str());
    }

    js_pda_coll_pred cjsparser::pred_for(const cjslexer *lexer, int idx) {
        auto end = lexer->get_unit_size();
        auto find_in = false;
        for (auto i = idx + 1; i < end; i++) {
            const auto &U = lexer->get_unit(i);
            if (U.t == T_SEMI) {
                break;
            }
            if (U.t == K_IN) {
                find_in = true;
                break;
            }
        }
        return find_in ? p_DELAY : p_ALLOW;
    }

    js_pda_coll_pred cjsparser::pred_in(const cjslexer *lexer, int idx) {
        auto find_for = false;
        for (auto i = idx - 1; i >= 0; i--) {
            const auto &U = lexer->get_unit(i);
            if (U.t == T_LPARAN) {
                if (i > 0 && lexer->get_unit(i - 1).t == K_FOR) {
                    find_for = true;
                }
                break;
            }
            if (U.t != ID) {
                break;
            }
        }
        return find_for ? p_REMOVE : p_ALLOW;
    }

    void cjsparser::clear_bk(const cjslexer *, int idx, std::vector<backtrace_t> &bks, backtrace_t *&bk) {
        std::vector<backtrace_t> b(1);
        std::swap(b[0], bks.back());
        bks = b;
        bk = &bks.back();
    }
}
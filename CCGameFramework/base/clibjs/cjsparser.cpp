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
#define TRACE_PARSING_LOG 1
#define TRACE_PARSING_AST 1
#define DUMP_LEXER 0
#define DUMP_PDA 0
#define DUMP_PDA_FILE "js_PDA.log"
#define DEBUG_AST 0
#define CHECK_AST 0

namespace clib {

    std::unique_ptr<cjsunit> cjsparser::unit;

    bool cjsparser::parse(const std::string &str, std::string &error_string, csemantic *s) {
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
        if (!unit) {
            unit = std::make_unique<cjsunit>();
            gen();
        }
        // 语法分析（递归下降）
        auto r = program(error_string, str);
        return r;
    }

    js_ast_node *cjsparser::root() const {
        return ast->get_root();
    }

    void cjsparser::clear_ast() {
        ast.reset();
    }

    void cjsparser::dump_pda(std::ostream& os)
    {
#if DUMP_PDA
        unit->dump(os);
#endif
    }

    void cjsparser::next() {
        current = &lexer->get_current_unit();
        lexer->inc_index();
    }

    void cjsparser::gen() {
        // REFER: antlr/grammars-v4
        // URL: https://github.com/antlr/grammars-v4/blob/master/javascript/javascript/JavaScriptParser.g4
#define DEF_LEXER(name) auto &_##name = unit->token(name);
#define DEF_LEXER_RULE(name) DEF_LEXER(name)
#define DEF_LEXER_RULE_LA(name) auto &_##name = unit->token(name, true);
        DEF_LEXER(NUMBER)
        DEF_LEXER(ID)
        DEF_LEXER(REGEX)
        DEF_LEXER(STRING)
        // ---
        DEF_LEXER_RULE_LA(RULE_NO_LINE)
        DEF_LEXER_RULE(RULE_LINE)
        DEF_LEXER_RULE(RULE_RBRACE)
        DEF_LEXER_RULE(RULE_EOF)
        // ---  LEXER
        DEF_LEXER(K_NEW)
        DEF_LEXER(K_VAR)
        DEF_LEXER(K_LET)
        DEF_LEXER(K_CONST)
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
#define DEF_RULE(name) auto &name = unit->rule(#name, c_##name);
#define DEF_RULE_ATTR(name, attr) auto &name = unit->rule(#name, c_##name, attr);
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
        variableStatement = (_K_VAR | _K_LET | _K_CONST) + variableDeclarationList + eos;
        variableDeclarationList = *(variableDeclarationList + ~_T_COMMA) + variableDeclaration;
        variableDeclaration = assignable + *(~_T_ASSIGN + singleExpression);
        emptyStatement = _T_SEMI;
        expressionStatement = expressionSequence + eos;
        ifStatement = _K_IF + ~_T_LPARAN + expressionSequence + ~_T_RPARAN + statement + *(~_K_ELSE + statement);
        iterationStatement = doStatement | whileStatement | forStatement | forInStatement;
        doStatement = _K_DO + statement + ~_K_WHILE + ~_T_LPARAN + expressionSequence + ~_T_RPARAN + eos;
        whileStatement = _K_WHILE + ~_T_LPARAN + expressionSequence + ~_T_RPARAN + statement;
        forStatement = _K_FOR + ~_T_LPARAN + *(expressionSequence | ((_K_VAR | _K_LET | _K_CONST) + variableDeclarationList)) +
                       _T_SEMI + *expressionSequence + _T_SEMI + *expressionSequence + ~_T_RPARAN + statement;
        forInStatement = _K_FOR + ~_T_LPARAN + (singleExpression | ((_K_VAR | _K_LET | _K_CONST) + variableDeclarationList)) +
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
        newExpression = _K_NEW + newExpressionArgument;
        newExpressionArgument = identifierExpression + *arguments;
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
        propertyExpressionAssignment = propertyName + *(~_T_COLON + singleExpression);
        computedPropertyExpressionAssignment = ~_T_LPARAN + singleExpression + ~_T_RPARAN +
                                               ~_T_COLON + singleExpression;
        propertyShorthand = _T_ELLIPSIS + singleExpression;
        propertyName = identifierName
                       | _STRING
                       | numericLiteral
                       | (_T_LSQUARE + singleExpression + _T_RSQUARE);
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
        eos = ~_T_SEMI | _RULE_EOF | _RULE_LINE | _RULE_RBRACE;
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
        unit->adjust(&functionExpression, &anonymousFunction, e_shift, 1);
        unit->adjust(&iterationStatement, &forInStatement, e_shift, -1);
        unit->adjust(&statement, &expressionStatement, e_shift, 1);
        unit->adjust(&iterationStatement, &forStatement, e_shift, 0, (void *) &pred_for);
        unit->adjust(&newExpression, &newExpressionArgument, e_shift, 1);
        unit->adjust(&inExpression, &inExpression, e_left_recursion, 0, (void *) &pred_in);
        unit->gen(&program);
#if DUMP_PDA
        std::ofstream of(DUMP_PDA_FILE);
        unit->dump(of);
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

    extern const std::string& js_pda_edge_str(js_pda_edge_t type);

    bool cjsparser::program(std::string& error_string, const std::string& str) {
#if REPORT_ERROR
        std::ofstream log(REPORT_ERROR_FILE, std::ios::app | std::ios::out);
#endif
        next();
        auto &pdas = unit->get_pda();
        auto root = ast->new_node(a_collection);
        root->line = root->column = 0;
        root->data._coll = pdas[0].coll;
        std::vector<std::shared_ptr<backtrace_t>> bks;
        {
            auto bk_tmp = std::make_shared<backtrace_t>();
            bk_tmp->lexer_index = 0;
            bk_tmp->state_stack.push_back(0);
            bk_tmp->ast_stack.push_back(root);
            bk_tmp->cache.push_back(root);
            bk_tmp->current_state = 0;
            bk_tmp->reduced_rule = -1;
            bk_tmp->trans = -1;
            bk_tmp->parent = -1;
            bk_tmp->init = true;
            bks.push_back(bk_tmp);
        }
        std::vector<int> trans_ids;
        using namespace std::chrono;
        using namespace std::chrono_literals;
        auto start_time = system_clock::now();
#if TRACE_PARSING && TRACE_PARSING_LOG
        auto idx = 0;
        {
            fprintf(stdout, "Parsing Code:\n---->\n%s\n<----\n", str.c_str());
        };
#endif
        while (!bks.empty()) {
#if NDEBUG
            if (duration_cast<milliseconds>(system_clock::now() - start_time) >= 15s) {
                error_string = "[COMPILE] Timeout";
                return false;
            }
#endif
            auto bk = bks.back();
            if (!bk->init) {
                auto p = bks[bk->parent];
                bk->init = true;
                copy_bk(*bk, *p);
            }
            lexer->set_index(bk->lexer_index);
            next();
            auto reduced = false;
            for (;;) {
#if TRACE_PARSING
                idx++;
                fflush(stdout);
#endif
                auto& current_state = pdas[bk->current_state];
                if (current->t == END) {
                    if (current_state.final) {
                        if (bk->state_stack.empty()) {
                            // success
                            cjsast::set_child(ast->get_root(), bk->ast_stack.back());
                            return true;
                        }
                    }
                }
                auto& trans = current_state.trans;
                trans_ids.clear();
                if (bk->trans != -1) {
                    trans_ids.push_back(bk->trans);
                    bk->trans = -1;
                }
                else {
                    for (size_t i = 0; i < trans.size(); ++i) {
                        auto& cs = trans[i];
                        if (valid_trans(*bk, cs)) {
                            trans_ids.push_back(i);
                        }
                    }
                }
                if (!trans_ids.empty()) {
                    if (current_state.pred) {
                        std::vector<int> then;
                        std::vector<int> add;
                        for (const auto t : trans_ids) {
                            if (trans[t].pred) {
                                auto cb = (pda_coll_pred_cb)trans[t].pred;
                                auto r = cb(lexer.get(), current->id);
                                if (r == p_ALLOW) {
                                    add.push_back(t);
                                }
                                else if (r == p_DELAY) {
                                    then.push_back(t);
                                }
                            }
                            else {
                                add.push_back(t);
                            }
                        }
                        if (trans_ids.size() > add.size()) {
                            trans_ids = then;
                            std::copy(add.cbegin(), add.cend(), std::back_inserter(trans_ids));
                        }
                    }
                }
                if (!trans_ids.empty()) {
                    if (reduced) {
                        auto parent = bk->parent;
                        if (parent >= 0) {
                            auto i = parent;
                            bk->parent = bks[i]->parent;
                            bk->reduced_rule = bks[i]->reduced_rule;
                            for (size_t j = i; j + 1 < bks.size(); j++) {
                                del_bk(*bks[j]);
                            }
                            bks.erase(bks.begin() + i, bks.end() - 1);
#if TRACE_PARSING
                            print_bk(bks);
#endif
                        }
                        reduced = false;
                    }
                    if (trans_ids.size() > 1) {
                        auto parent = (int)bks.size() - 1;
                        for (const auto& i : trans_ids) {
                            auto bk_tmp = std::make_shared<backtrace_t>();
                            bk_tmp->current_state = bk->current_state;
                            bk_tmp->parent = parent;
                            bk_tmp->reduced_rule = trans[trans_ids.back()].reduced_rule;
                            bk_tmp->trans = i;
                            bk_tmp->init = false;
                            bks.push_back(bk_tmp);
                        }
#if TRACE_PARSING
                        std::cout << "create branch" << std::endl;
                        print_bk(bks);
#endif
                        break;
                    }
                    const auto& t = trans[trans_ids.back()];
                    if (t.type == e_finish) {
                        if (current->t != END) {
#if TRACE_PARSING
                            std::cout << "parsing redundant code: " << current_state.label << std::endl;
#endif
#if REPORT_ERROR
                            log << "parsing redundant code: " << current_state.label << std::endl;
#endif
                            error_string = "[COMPILE] Redundant code";
                            return false;
                        }
                    }
                    else if (current_state.cb && t.cb != nullptr) {
                        ((terminal_cb)(t.cb))(lexer.get(), current->id, bks, bk);
                    }
                    const auto& jump = t.jump;
#if TRACE_PARSING && TRACE_PARSING_LOG
                    {
                        const auto& line = current->line;
                        const auto& column = current->column;
                        auto s = str.substr(current->start, current->end - current->start);
                        auto end = current->t == END ? "*" : "";
                        do_trans(*bk, t);
                        fprintf(stdout, "[%d|%d|%d:%d|%d:%d]%s State: %3d => To: %3d  %.100s -- Action: %-10s -- Rule: %s\n",
                            idx, bks.size(), bk->reduced_rule, t.reduced_rule, line, column, end, bk->current_state, jump, s.c_str(),
                            js_pda_edge_str(t.type).c_str(), current_state.label.c_str());
                    }
#else
                    do_trans(*bk, t);
#endif
                    bk->current_state = jump;
                    if (semantic) {
                        // DETERMINE LR JUMP BEFORE PARSING AST
                        auto dir = semantic->check(t.type, bk->ast_stack.back());
                        if (dir == b_error) {
#if TRACE_PARSING
                            std::cout << "parsing semantic error: " << current_state.label << std::endl;
#endif
#if REPORT_ERROR
                            log << "parsing semantic error: " << current_state.label << std::endl;
#endif
                            error_string = "[COMPILE] Semantic error";
                            return false;
                        }
                    }
                    if (bk->reduced_rule == t.reduced_rule && (t.type == e_reduce || t.type == e_reduce_exp)) {
                        reduced = true;
                    }
                }
                else {
#if TRACE_PARSING
                    std::cout << "parsing error" << std::endl;
#endif
                    auto parent = bk->parent;
                    if (parent == -1) {
                        return false;
                    }
                    if (parent + 2 == bks.size()) {
                        auto i = parent;
                        for (;;) {
                            auto p = bks[i];
                            auto pp = p->parent;
                            if (pp != -1 && pp + 1 == i) {
                                i = pp;
                            }
                            else {
                                break;
                            }
                        }
                        parent = i;
                        for (size_t j = parent; j < bks.size(); j++) {
                            del_bk(*bks[j]);
                        }
                        bks.erase(bks.begin() + parent, bks.end());
                        if (!bks.empty())
                            bk = bks.back();
                        else
                            return false;
#if TRACE_PARSING
                        print_bk(bks);
#endif
                    }
                    else {
                        del_bk(*bk);
                        bks.pop_back();
                        bk = bks.back();
#if TRACE_PARSING
                        print_bk(bks);
#endif
                    }
                    break;
                }
            }
        }
        error_string = "[COMPILE] Compile error";
        return false;
    }

    void cjsparser::print_bk(std::vector<std::shared_ptr<backtrace_t>>& bks) const
    {
        const auto& pdas = unit->get_pda();
        for (size_t i = 0; i < bks.size(); ++i) {
            auto& _bk = bks[i];
            fprintf(stdout,
                "[BACKTRACE#%d] [%d:%d] %s   -- %s\n", i, _bk->parent, _bk->reduced_rule,
                pdas[_bk->current_state].label.c_str(), lexer->get_unit_desc(_bk->lexer_index).c_str());
            if (_bk->trans != -1) {
                auto t = pdas[_bk->current_state].trans[_bk->trans];
                if (t.label.empty()) {
                    fprintf(stdout, "[BACKTRACE#%d]     %s :: %s\n", i, js_pda_edge_str(t.type).c_str(), pdas[t.jump].label.c_str());
                }
                else {
                    fprintf(stdout, "[BACKTRACE#%d]     %s :: %s\n", i, js_pda_edge_str(t.type).c_str(), t.label.c_str());
                }
            }
#if 0
            if (_bk->init) {
                for (size_t j = 0; j < _bk->ast_stack.size(); j++) {
                    auto& token = _bk->ast_stack[j];
                    fprintf(stdout, "[STACK: %d] ", j);
                    cjsast::print(token, 0, str, std::cout);
                }
            }
#endif
        }
    }

    void cjsparser::copy_bk(backtrace_t& bk, backtrace_t& p)
    {
        bk.lexer_index = p.lexer_index;
        bk.state_stack = p.state_stack;
        bk.current_state = p.current_state;
        std::unordered_map<js_ast_node*, js_ast_node*> map_ast1, map_ast2;
        bk.cache.reserve(p.cache.size());
        for (const auto& c : p.cache) {
            if (c->idx != -1) {
                lexer->set_index(c->idx);
                next();
                bk.cache.push_back(terminal());
            }
            else {
                assert(c->flag == a_collection);
                auto new_ast = ast->new_node(a_collection);
                memcpy(new_ast, c, sizeof(*c));
                bk.cache.push_back(new_ast);
            }
            map_ast1.insert({ c, bk.cache.back() });
            map_ast2.insert({ bk.cache.back(), c });
        }
        for (const auto& c : bk.cache) {
            auto c2 = map_ast2.at(c);
            if (c2->child)
                c->child = map_ast1.at(c2->child);
            if (c2->prev)
                c->prev = map_ast1.at(c2->prev);
            if (c2->next)
                c->next = map_ast1.at(c2->next);
            if (c2->parent) {
                if (c2->parent->flag == a_root)
                    c->parent = c2->parent;
                else
                    c->parent = map_ast1.at(c2->parent);
            }
        }
        bk.ast_stack.reserve(p.ast_stack.size());
        for (const auto& c : p.ast_stack) {
            bk.ast_stack.push_back(map_ast1.at(c));
        }
    }

    void cjsparser::del_bk(backtrace_t& bk)
    {
        for (const auto& c : bk.cache) {
            ast->remove_force(c);
        }
    }

    js_ast_node *cjsparser::terminal() {
        if (current->t == END) { // 结尾
            error("unexpected token EOF of expression");
        }
        if (current->t > OPERATOR_START && current->t < OPERATOR_END) {
            auto node = ast->new_node(a_operator);
            node->line = current->line;
            node->column = current->column;
            node->start = current->start;
            node->end = current->end;
            node->data._op = current->t;
            node->idx = lexer->get_index() - 1;
            match_type(node->data._op);
            return node;
        }
        if (current->t > KEYWORD_START && current->t < KEYWORD_END) {
            auto node = ast->new_node(a_keyword);
            node->line = current->line;
            node->column = current->column;
            node->start = current->start;
            node->end = current->end;
            node->data._keyword = current->t;
            node->idx = lexer->get_index() - 1;
            match_type(node->data._keyword);
            return node;
        }
        if (current->t == ID) {
            auto node = ast->new_node(a_literal);
            node->line = current->line;
            node->column = current->column;
            node->start = current->start;
            node->end = current->end;
            ast->set_str(node, lexer->get_data(current->idx));
            node->idx = lexer->get_index() - 1;
            match_type(current->t);
            return node;
        }
        if (current->t == NUMBER) {
            auto node = ast->new_node(a_number);
            node->line = current->line;
            node->column = current->column;
            node->start = current->start;
            node->end = current->end;
            node->data._number = *(double *) lexer->get_data(current->idx);
            node->idx = lexer->get_index() - 1;
            match_type(current->t);
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
            node->idx = lexer->get_index() - 1;
            match_type(current->t);

            while (current->t == STRING) {
                ss << lexer->get_data(current->idx);
                match_type(current->t);
            }
            ast->set_str(node, ss.str());
            return node;
        }
        if (current->t == REGEX) {
            auto node = ast->new_node(a_regex);
            node->line = current->line;
            node->column = current->column;
            node->start = current->start;
            node->end = current->end;
            ast->set_str(node, lexer->get_data(current->idx));
            node->idx = lexer->get_index() - 1;
            match_type(current->t);
            return node;
        }
        if (current->t > RULE_START && current->t < RULE_END) {
            error("invalid rule type");
        }
        error("invalid type");
        return nullptr;
    }

    bool cjsparser::valid_trans(backtrace_t& bk, js_pda_trans& trans, js_unit_token** out) {
        auto& la = trans.LA;
        if (!la.empty()) {
            auto success = false;
            for (auto& _la : la) {
                if (LA(bk, _la, trans, out)) {
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
            if (bk.ast_stack.size() <= 1)
                return false;
            if (bk.state_stack.empty())
                return false;
            if (trans.status != bk.state_stack.back())
                return false;
        }
                         break;
        default:
            break;
        }
        return true;
    }

    void cjsparser::do_trans(backtrace_t &bk, const js_pda_trans &trans) {
        switch (trans.type) {
            case e_shift: {
                bk.state_stack.push_back(bk.current_state);
                auto new_node = ast->new_node(a_collection);
                new_node->line = new_node->column = 0;
                auto &pdas = unit->get_pda();
                new_node->data._coll = pdas[trans.jump].coll;
#if DEBUG_AST
                fprintf(stdout, "[DEBUG] Shift: top=%p, new=%p, CS=%d\n", ast_stack.back(), new_node,
                        cjsast::children_size(ast_stack.back()));
#endif
                bk.ast_stack.push_back(new_node);
                bk.cache.push_back(new_node);
            }
                break;
            case e_pass: {
                if (current->t == END) {
                    error("unexpected token EOF of expression");
                }
                next();
                bk.lexer_index++;
            }
                break;
            case e_move: {
                auto t = terminal();
                bk.lexer_index++;
#if CHECK_AST
                check_ast(t);
#endif
#if DEBUG_AST
                fprintf(stdout, "[DEBUG] Move: parent=%p, child=%p, CS=%d\n", ast_stack.back(), t,
                        cjsast::children_size(ast_stack.back()));
#endif
                cjsast::set_child(bk.ast_stack.back(), t);
                bk.cache.push_back(t);
            }
                break;
            case e_left_recursion:
                break;
            case e_left_recursion_not_greed:
                break;
            case e_reduce:
            case e_reduce_exp: {
                auto new_ast = bk.ast_stack.back();
                check_ast(new_ast);
                bk.state_stack.pop_back();
                bk.ast_stack.pop_back();
                assert(!bk.ast_stack.empty());
#if DEBUG_AST
                fprintf(stdout, "[DEBUG] Reduce: parent=%p, child=%p, CS=%d, AS=%d, RI=%d\n",
                        ast_stack.back(), new_ast, cjsast::children_size(ast_stack.back()),
                        ast_stack.size(), ast_reduce_cache.size());
#endif
                if (trans.type == e_reduce_exp)
                    bk.ast_stack.back()->attr |= a_exp;
                if (new_ast->flag != a_collection || new_ast->child != nullptr) {
                    cjsast::set_child(bk.ast_stack.back(), new_ast);
                    check_ast(bk.ast_stack.back());
                }
            }
                break;
            case e_finish:
                bk.state_stack.pop_back();
                break;
            default:
                break;
        }
    }

    bool cjsparser::LA(backtrace_t& bk, struct js_unit* u, js_pda_trans& trans, js_unit_token** out) {
        if (u->t != u_token)
            return false;
        auto token = js_to_token(u);
        if (token->type > RULE_START && token->type < RULE_END) {
            auto r = lexer->valid_rule(bk.lexer_index, token->type);
            if (!r)
                return false;
            if (!token->LA)
                return true;
            auto& pdas = unit->get_pda();
            auto& target = pdas[trans.jump];
            auto& t = target.trans;
            auto id = token->type - RULE_START - 1;
            if (trans.rule && trans.rule->at(id)) {
                for (auto& _la : *trans.rule->at(id)) {
                    if (LA(bk, _la, trans)) {
                        return true;
                    }
                }
            }
            js_unit_token* out2 = nullptr;
            for (size_t i = 0; i < t.size(); ++i) {
                auto& cs = t[i];
                if (valid_trans(bk, cs, &out2)) {
                    auto id = token->type - RULE_START - 1;
                    if (!trans.rule) {
                        trans.rule = std::make_shared<std::array<std::shared_ptr<std::vector<js_unit*>>, RULE_END - RULE_START - 1>>();
                    }
                    auto& rule = *trans.rule;
                    if (!rule[id]) {
                        rule[id] = std::make_shared<std::vector<js_unit*>>();
                    }
                    rule[id]->push_back(out2);
                    return true;
                }
            }
            return false;
        }
        if (out)
            *out = token;
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
}
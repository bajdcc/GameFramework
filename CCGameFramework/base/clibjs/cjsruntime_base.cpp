//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <fstream>
#include "cjsruntime.h"
#include "cjsast.h"
#include "cjsgui.h"
#include "cjsparser.h"

#define DUMP_PRINT_FILE_ENABLE 0
#define DUMP_PRINT_FILE_AUTO_CLEAR 1
#define DUMP_PRINT_FILE "js_debug_print.txt"

#define LOG_AST 0
#define LOG_FILE 0

namespace clib {

    void cjsruntime::init() {
        // proto
        permanents._proto_root = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_root->__proto__.reset();
        permanents._proto_object = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_number = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_string = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_boolean = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_function = _new_object(js_value::at_const | js_value::at_readonly);
        // const value
        permanents.global_env = _new_object(js_value::at_const);
        permanents._null = _new_null(js_value::at_const | js_value::at_readonly);
        permanents._undefined = _new_undefined(js_value::at_const | js_value::at_readonly);
        permanents._true = _new_boolean(true, js_value::at_const | js_value::at_readonly);
        permanents._false = _new_boolean(false, js_value::at_const | js_value::at_readonly);
        permanents.__nan = _new_number(NAN, js_value::at_const | js_value::at_readonly);
        permanents._inf = _new_number(INFINITY, js_value::at_const | js_value::at_readonly);
        permanents._minus_inf = _new_number(-INFINITY, js_value::at_const);
        permanents._zero = _new_number(0.0, js_value::at_const);
        permanents._minus_zero = _new_number(-0.0, js_value::at_const);
        permanents._one = _new_number(1.0, js_value::at_const);
        permanents._minus_one = _new_number(-1.0, js_value::at_const);
        permanents._empty = _new_string("");
        // length
        auto _int_0 = _new_number(0, js_value::at_const | js_value::at_refs);
        auto _int_1 = _new_number(1, js_value::at_const | js_value::at_refs);
        auto _int_2 = _new_number(1, js_value::at_const | js_value::at_refs);
        auto _empty_string = _new_string("", js_value::at_const | js_value::at_refs);
        // proto
        permanents._proto_root->add("__type__", _new_string("Root", js_value::at_const | js_value::at_refs));
        permanents._proto_object->add("__type__", _new_string("Object", js_value::at_const | js_value::at_refs));
        permanents._proto_object_hasOwnProperty = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_object_hasOwnProperty->add("length", _int_1);
        permanents._proto_object_hasOwnProperty->name = "hasOwnProperty";
        permanents._proto_object_hasOwnProperty->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            if (args.empty()) {
                func->stack.push_back(js.new_boolean(false));
                return 0;
            }
            auto f = _this.lock();
            if (f->get_type() != r_object) {
                func->stack.push_back(js.new_boolean(false));
                return 0;
            }
            const auto& obj = JS_OBJ(f);
            auto r = 0;
            func->stack.push_back(js.new_boolean(obj->get(args.front().lock()->to_string(&js, 0, &r)) != nullptr));
            return r;
        };
        permanents._proto_object->add(permanents._proto_object_hasOwnProperty->name, permanents._proto_object_hasOwnProperty);
        permanents._proto_object->add("__type__", _new_string("Object", js_value::at_const | js_value::at_refs));
        permanents._proto_object_toString = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_object_toString->add("length", _int_0);
        permanents._proto_object_toString->name = "toString";
        permanents._proto_object_toString->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            auto r = 0;
            func->stack.push_back(js.new_string(_this.lock()->to_string(&js, 2, &r)));
            return r;
        };
        permanents._proto_object->add(permanents._proto_object_toString->name, permanents._proto_object_toString);
        permanents._proto_object_valueOf = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_object_valueOf->add("length", _int_0);
        permanents._proto_object_valueOf->name = "valueOf";
        permanents._proto_object_valueOf->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            if (_this.lock()->is_primitive()) {
                func->stack.push_back(_this);
                return 0;
            }
            auto o = JS_OBJ(_this.lock());
            if (!o->special.empty()) {
                auto f = o->special.find("PrimitiveValue");
                if (f != o->special.end()) {
                    func->stack.push_back(f->second);
                    return 0;
                }
            }
            func->stack.push_back(_this);
            return 0;
        };
        permanents._proto_object->add(permanents._proto_object_valueOf->name, permanents._proto_object_valueOf);
        permanents._proto_boolean->add("__type__", _new_string("Boolean", js_value::at_const | js_value::at_refs));
        permanents._proto_function->add("__type__", _new_string("Function", js_value::at_const | js_value::at_refs));
        permanents._proto_function_call = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_function_call->add("length", _int_1);
        permanents._proto_function_call->name = "call";
        permanents._proto_function_call->builtin = [](auto& func, auto& _this, auto& __args, auto& js, auto attr) {
            auto f = _this.lock();
            if (f->get_type() != r_function) {
                func->stack.push_back(js.new_undefined());
                return 0;
            }
            auto fun = JS_FUN(f);
            auto __this = __args.empty() ? _this : __args.front();
            auto args = __args.size() > 1 ?
                std::vector<js_value::weak_ref>(__args.begin() + 1, __args.end()) :
                std::vector<js_value::weak_ref>();
            return js.call_api(fun, __this, args, jsv_function::at_fast);
        };
        permanents._proto_function->add(permanents._proto_function_call->name, permanents._proto_function_call);
        permanents._proto_function_apply = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_function_apply->add("length", _int_1);
        permanents._proto_function_apply->name = "apply";
        permanents._proto_function_apply->builtin = [](auto& func, auto& _this, auto& __args, auto& js, auto attr) {
            auto f = _this.lock();
            if (f->get_type() != r_function) {
                func->stack.push_back(js.new_undefined());
                return 0;
            }
            auto fun = JS_FUN(f);
            auto __this = __args.empty() ? _this : __args.front();
            auto r = 0;
            auto args = __args.size() > 1 ?
                to_array(__args[1].lock(), &r) :
                std::vector<js_value::weak_ref>();
            if (r != 0)
                return 0;
            return js.call_api(fun, __this, args, jsv_function::at_fast);
        };
        permanents._proto_function->add(permanents._proto_function_apply->name, permanents._proto_function_apply);
        permanents._proto_number->add("__type__", _new_string("Number", js_value::at_const | js_value::at_refs));
        permanents._proto_string->add("__type__", _new_string("String", js_value::at_const | js_value::at_refs));
        permanents._proto_string_replace = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_string_replace->add("length", _int_2);
        permanents._proto_string_replace->name = "replace";
        permanents._proto_string_replace->builtin = [](auto& func, auto& _this, auto& __args, auto& js, auto attr) {
            auto f = _this.lock();
            if (__args.size() < 2) {
                func->stack.push_back(_this);
                return 0;
            }
            auto r = 0;
            auto origin = f->to_string(&js, 0, &r);
            if (r != 0)
                return r;
            auto re = __args[0].lock();
            auto _replacer = __args[1].lock();
            if (re->get_type() == r_regex) {
                if (_replacer->get_type() != r_function) {
                    func->stack.push_back(js.new_string(JS_RE(re)->replace(origin, _replacer->to_string(&js, 0, &r))));
                    return r;
                }
                else {
                    auto rep = JS_FUN(_replacer);
                    std::vector<std::tuple<std::string, bool>> matches;
                    if (!JS_RE(re)->match(origin, matches)) {
                        func->stack.push_back(js.new_string(origin));
                        return 0;
                    }
                    for (auto& s : matches) {
                        if (std::get<1>(s)) {
                            auto& str = std::get<0>(s);
                            auto r = 0;
                            std::vector<js_value::weak_ref> args;
                            args.push_back(js.new_string(str));
                            auto v = js.fast_api(rep, _this, args, 0, &r);
                            if (r != 0)
                                return r;
                            str = v->to_string(&js, 0, &r);
                            if (r != 0)
                                return r;
                        }
                    }
                    std::stringstream ss;
                    for (auto& s : matches) {
                        ss << std::get<0>(s);
                    }
                    func->stack.push_back(js.new_string(ss.str()));
                    return 0;
                }
            }
            else {
                if (_replacer->get_type() != r_function) {
                    auto str_replacer = _replacer->to_string(&js, 0, &r);
                    if (r != 0)
                        return r;
                    auto a = re->to_string(&js, 0, &r);
                    if (r != 0)
                        return r;
                    func->stack.push_back(js.new_string(jsv_regexp::replace(origin, a, str_replacer)));
                    return 0;
                }
                else {
                    auto rep = JS_FUN(_replacer);
                    auto pat = re->to_string(&js, 0, &r);
                    if (r != 0)
                        return r;
                    auto r = 0;
                    std::vector<js_value::weak_ref> args;
                    args.push_back(js.new_string(pat));
                    auto v = js.fast_api(rep, _this, args, 0, &r);
                    if (r != 0)
                        return r;
                    auto v2 = v->to_string(&js, 0, &r);
                    if (r != 0)
                        return r;
                    func->stack.push_back(js.new_string(jsv_regexp::replace(origin, pat, v2)));
                    return 0;
                }
            }
        };
        permanents._proto_string->add(permanents._proto_string_replace->name, permanents._proto_string_replace);
#if DUMP_PRINT_FILE_ENABLE
        permanents._debug_print = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._debug_print->add("length", _int_1);
#if DUMP_PRINT_FILE_AUTO_CLEAR
        permanents._debug_print->name = "debug_print";
        {
            std::ofstream ofs(DUMP_PRINT_FILE);
        }
#endif
#endif
#if DUMP_PRINT_FILE_ENABLE
        permanents._debug_print->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            std::ofstream ofs(DUMP_PRINT_FILE, std::ios::app);
            for (size_t i = 0; i < args.size(); i++) {
                ofs << args[i].lock()->to_string(&js, 0);
                if (i + 1 < args.size())
                    ofs << " ";
            }
            ofs << std::endl;
            func->stack.push_back(js.new_undefined());
            return 0;
        };
        permanents.global_env->add(permanents._debug_print->name, permanents._debug_print);
#endif
        permanents.global_env->add("NaN", permanents.__nan);
        permanents.global_env->add("Infinity", permanents._inf);
        // debug
        permanents._debug_dump = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._debug_dump->add("length", _int_1);
        permanents._debug_dump->name = "debug_dump";
        permanents._debug_dump->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            char buf[128];
            auto t = _this.lock();
            assert(t);
            auto r = 0;
            auto t1 = t->unary_op(js, UNARY_TYPEOF, &r);
            if (r != 0)
                return r;
            auto type = t1->to_string(&js, 0, &r);
            if (r != 0)
                return r;
            auto proto = t->__proto__.lock();
            auto p = std::string("none");
            if (proto->get_type() == r_object) {
                const auto& o = JS_OBJ(proto);
                auto of = o->get("__type__");
                if (of) {
                    p = of->to_string(&js, 0, &r);
                    if (r != 0)
                        return r;
                }
            }
            auto t2 = t->to_string(&js, 0, &r);
            if (r != 0)
                return r;
            snprintf(buf, sizeof(buf),
                "Str: %s, Type: %s, Proto: %s, Ptr: %p",
                t2.c_str(), type.c_str(), p.c_str(), t.get());
            func->stack.push_back(js.new_string(buf));
            return 0;
        };
        permanents._proto_root->add(permanents._debug_dump->name, permanents._debug_dump);
        // console
        permanents.console = _new_object(js_value::at_const | js_value::at_readonly);
        permanents.console_log = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.console_log->add("length", _int_1);
        permanents.console_log->name = "log";
        permanents.console_log->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            std::stringstream ss;
            auto r = 0;
            for (size_t i = 0; i < args.size(); i++) {
                ss << args[i].lock()->to_string(&js, 1, &r);
                if (r != 0)
                    return r;
                if (i + 1 < args.size())
                    ss << " ";
            }
            ss << std::endl;
            cjsgui::singleton().put_string(ss.str());
            func->stack.push_back(js.new_undefined());
            return 0;
        };
        permanents.console->add(permanents.console_log->name, permanents.console_log);
        permanents.console_error = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.console_error->add("length", _int_1);
        permanents.console_error->name = "error";
        permanents.console_error->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            std::stringstream ss;
            auto r = 0;
            for (size_t i = 0; i < args.size(); i++) {
                ss << args[i].lock()->to_string(&js, 1, &r);
                if (r != 0)
                    return r;
                if (i + 1 < args.size())
                    ss << " ";
            }
            ss << std::endl;
            cjsgui::singleton().put_string("\033FFF0000\033");
            cjsgui::singleton().put_string(ss.str());
            cjsgui::singleton().put_string("\033S4\033");
            func->stack.push_back(js.new_undefined());
            return 0;
        };
        permanents.console->add(permanents.console_error->name, permanents.console_error);
        permanents.console_trace = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.console_trace->add("length", _new_number(0, js_value::at_const | js_value::at_refs));
        permanents.console_trace->name = "trace";
        permanents.console_trace->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            func->stack.push_back(js.new_string(js.get_stacktrace()));
            return 0;
        };
        permanents.console->add(permanents.console_trace->name, permanents.console_trace);
        permanents.global_env->add("console", permanents.console);
        // sys
        permanents.sys = _new_object(js_value::at_const | js_value::at_readonly);
        permanents.sys_exec_file = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.sys_exec_file->add("length", _int_1);
        permanents.sys_exec_file->name = "exec_file";
        permanents.sys_exec_file->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            if (args.empty()) {
                func->stack.push_back(js.new_undefined());
                return 0;
            }
            auto r = 0;
            auto filename = args.front().lock()->to_string(&js, 0, &r);
            if (r != 0)
                return r;
            std::string content;
            if (js.get_file(filename, content)) {
                func->pc++;
                return js.exec(filename, content);
            }
            func->stack.push_back(js.new_undefined());
            return 0;
        };
        permanents.sys->add(permanents.sys_exec_file->name, permanents.sys_exec_file);
        permanents.sys_builtin = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.sys_builtin->add("length", _int_1);
        permanents.sys_builtin->name = "builtin";
        permanents.sys_builtin->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            if (args.empty() || args.front().lock()->is_primitive()) {
                func->stack.push_back(js.new_boolean(false));
                return 0;
            }
            func->stack.push_back(js.new_boolean(js.set_builtin(JS_O(args.front().lock()))));
            return 0;
        };
        permanents.sys->add(permanents.sys_builtin->name, permanents.sys_builtin);
        permanents.sys_eval = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.sys_eval->add("length", _int_1);
        permanents.sys_eval->name = "eval";
        permanents.sys_eval->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_eval, _this, args, 0);
        };
        permanents.sys->add(permanents.sys_eval->name, permanents.sys_eval);
        permanents.sys_http = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.sys_http->add("length", _int_1);
        permanents.sys_http->name = "http";
        permanents.sys_http->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_http, _this, args, 0);
        };
        permanents.sys->add(permanents.sys_http->name, permanents.sys_http);
        permanents.global_env->add("sys", permanents.sys);
        // number
        permanents.f_number = _new_function(permanents._proto_number, js_value::at_const | js_value::at_readonly);
        permanents.f_number->add("length", _int_1);
        permanents.f_number->name = "Number";
        permanents.f_number->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            js_value::ref pri;
            if (args.empty()) {
                pri = js.new_number(0.0);
            }
            else {
                auto n = args.front().lock();
                if (n->get_type() == r_number) {
                    pri = n;
                }
                else {
                    double d = 0.0;
                    if (to_number(n, d)) {
                        pri = js.new_number(d);
                    }
                    else {
                        pri = js.new_number(NAN);
                    }
                }
            }
            if (attr & jsv_function::at_new_function) {
                pri = js.new_object_box(pri);
            }
            func->stack.push_back(pri);
            return 0;
        };
        permanents.global_env->add(permanents.f_number->name, permanents.f_number);
        // boolean
        permanents.f_boolean = _new_function(permanents._proto_boolean, js_value::at_const | js_value::at_readonly);
        permanents.f_boolean->add("length", _int_1);
        permanents.f_boolean->name = "Boolean";
        permanents.f_boolean->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            js_value::ref pri;
            if (args.empty()) {
                pri = js.new_boolean(false);
            }
            else {
                pri = js.new_boolean(args.front().lock()->to_bool());
            }
            if (attr & jsv_function::at_new_function) {
                pri = js.new_object_box(pri);
            }
            func->stack.push_back(pri);
            return 0;
        };
        permanents.global_env->add(permanents.f_boolean->name, permanents.f_boolean);
        // object
        permanents.f_object = _new_function(permanents._proto_object, js_value::at_const | js_value::at_readonly);
        permanents.f_object->add("length", _int_1);
        permanents.f_object->name = "Object";
        permanents.f_object->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            js_value::ref pri;
            if (args.empty()) {
                pri = js.new_object();
            }
            else {
                auto n = args.front().lock();
                if (n->is_primitive()) {
                    pri = js.new_object_box(n);
                }
                else {
                    pri = n;
                }
            }
            func->stack.push_back(pri);
            return 0;
        };
        permanents.global_env->add(permanents.f_object->name, permanents.f_object);
        // string
        permanents.f_string = _new_function(permanents._proto_string, js_value::at_const | js_value::at_readonly);
        permanents.f_string->add("length", _int_1);
        permanents.f_string->name = "String";
        permanents.f_string->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            js_value::ref pri;
            if (args.empty()) {
                pri = js.new_string("");
            }
            else {
                auto r = 0;
                auto n = args.front().lock();
                if (n->get_type() == r_string) {
                    pri = n;
                }
                else {
                    pri = js.new_string(args.front().lock()->to_string(&js, 0, &r));
                    if (r != 0)
                        return r;
                }
            }
            if (attr & jsv_function::at_new_function) {
                pri = js.new_object_box(pri);
            }
            func->stack.push_back(pri);
            return 0;
        };
        permanents.global_env->add(permanents.f_string->name, permanents.f_string);
        // function
        permanents.f_function = _new_function(permanents._proto_function, js_value::at_const | js_value::at_readonly);
        permanents.f_function->add("length", _int_1);
        permanents.f_function->name = "Function";
        permanents.f_function->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            std::vector<js_value::weak_ref> _args;
            if (args.empty()) {
                _args.push_back(js.new_string("(function anonymous() {})"));
                return js.call_api(API_eval, _this, _args, 0);
            }
            else {
                auto n = args.front().lock();
                auto r = 0;
                auto s = n->to_string(&js, 0, &r);
                if (r != 0)
                    return r;
                std::stringstream ss;
                ss << "(function anonymous() { " << s << " })";
                _args.push_back(js.new_string(ss.str()));
                return js.call_api(API_eval, _this, _args, 0);
            }
        };
        permanents.global_env->add(permanents.f_function->name, permanents.f_function);
        // array
        permanents._proto_array = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_array->add("__type__", _new_string("Array", js_value::at_const | js_value::at_refs));
        permanents.f_array = _new_function(permanents._proto_array, js_value::at_const | js_value::at_readonly);
        permanents.f_array->add("length", _int_1);
        permanents.f_array->name = "Array";
        permanents.f_array->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            js_value::ref pri;
            if (args.empty()) {
                pri = js.new_array();
            }
            else {
                auto arr = js.new_array();
                if (args.size() == 1 && args.front().lock()->get_type() == r_number) {
                    arr->add("length", args.front().lock());
                }
                else {
                    auto i = 0;
                    for (const auto& s : args) {
                        std::stringstream ss;
                        ss << i++;
                        arr->add(ss.str(), s);
                    }
                    arr->add("length", js.new_number(i));
                }
                pri = arr;
            }
            func->stack.push_back(pri);
            return 0;
        };
        permanents.global_env->add(permanents.f_array->name, permanents.f_array);
        // regexp
        permanents._proto_regexp = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_regexp->add("__type__", _new_string("RegExp", js_value::at_const | js_value::at_refs));
        permanents.f_regexp = _new_function(permanents._proto_array, js_value::at_const | js_value::at_readonly);
        permanents.f_regexp->add("length", _int_1);
        permanents.f_regexp->name = "RegExp";
        permanents.f_regexp->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            auto regexp = js.new_regexp();
            auto r = 0;
            if (args.empty()) {
                regexp->init("");
            }
            else if (args.size() == 1) {
                regexp->init(args.front().lock()->to_string(&js, 0, &r));
            }
            else if (args.size() >= 2) {
                regexp->init(args.front().lock()->to_string(&js, 0, &r), args[1].lock()->to_string(&js, 0, &r));
            }
            if (r != 0)
                return r;
            if (!regexp->error.empty()) {
                std::stringstream ss;
                ss << "throw new SyntaxError('Invalid regular expression: " << jsv_string::convert(regexp->str) << ": " << jsv_string::convert(regexp->error) << "')";
                return js.exec("<regex::error>", ss.str(), true);
            }
            func->stack.push_back(regexp);
            return 0;
        };
        permanents.global_env->add(permanents.f_regexp->name, permanents.f_regexp);
        permanents._proto_regexp_test = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_regexp_test->add("length", _int_1);
        permanents._proto_regexp_test->name = "test";
        permanents._proto_regexp_test->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            if (args.empty()) {
                func->stack.push_back(js.new_boolean(false));
                return 0;
            }
            auto r = 0;
            auto str = args.front().lock()->to_string(&js, 0, &r);
            if (r != 0)
                return r;
            auto t = _this.lock();
            if (t->get_type() != r_regex) {
                func->stack.push_back(js.new_boolean(false));
                return 0;
            }
            auto re = JS_RE(t);
            func->stack.push_back(js.new_boolean(re->test(str)));
            return 0;
        };
        permanents._proto_regexp->add(permanents._proto_regexp_test->name, permanents._proto_regexp_test);
        // global function
        permanents.global_setTimeout = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.global_setTimeout->add("length", _int_1);
        permanents.global_setTimeout->name = "setTimeout";
        permanents.global_setTimeout->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_setTimeout, _this, args, 0);
        };
        permanents.global_env->add(permanents.global_setTimeout->name, permanents.global_setTimeout);
        permanents.global_setInterval = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.global_setInterval->add("length", _int_1);
        permanents.global_setInterval->name = "setInterval";
        permanents.global_setInterval->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_setInterval, _this, args, 0);
        };
        permanents.global_env->add(permanents.global_setInterval->name, permanents.global_setInterval);
        permanents.global_clearTimeout = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.global_clearTimeout->add("length", _int_1);
        permanents.global_clearTimeout->name = "clearTimeout";
        permanents.global_clearTimeout->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_clearTimeout, _this, args, 0);
        };
        permanents.global_env->add(permanents.global_clearTimeout->name, permanents.global_clearTimeout);
        permanents.global_clearInterval = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.global_clearInterval->add("length", _int_1);
        permanents.global_clearInterval->name = "clearInterval";
        permanents.global_clearInterval->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_clearInterval, _this, args, 0);
        };
        permanents.global_env->add(permanents.global_clearInterval->name, permanents.global_clearInterval);
        // error
        permanents._proto_error = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_error->add("__type__", _new_string("Error", js_value::at_const | js_value::at_refs));
        permanents._proto_error->add("name", permanents._proto_error->get("__type__"));
        permanents._proto_error->add("message", _empty_string);
        permanents.f_error = _new_function(permanents._proto_error, js_value::at_const | js_value::at_readonly);
        permanents.f_error->add("length", _int_1);
        permanents.f_error->name = "Error";
        permanents.f_error->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            auto err = js.new_error(0);
            err->add("name", js.new_string("Error"));
            if (!args.empty()) {
                auto r = 0;
                err->add("message", js.new_string(args.front().lock()->to_string(&js, 0, &r)));
                if (r != 0)
                    return r;
            }
            err->add("stack", js.new_string(js.get_stacktrace()));
            func->stack.push_back(err);
            return 0;
        };
        permanents.global_env->add(permanents.f_error->name, permanents.f_error);
        // default func
        permanents.default_stack = std::make_shared<cjs_function>();
        permanents.default_stack->name = "<default>";
        permanents.default_stack->info = cjs_function_info::create_default();
        // http method
        tools.http_method_map["GET"] = M_GET;
        tools.http_method_map["POST"] = M_POST;
        tools.http_method_map["PUT"] = M_PUT;
        tools.http_method_map["PATCH"] = M_PATCH;
        tools.http_method_map["DELETE"] = M_DELETE;
        tools.http_method_map["HEAD"] = M_HEAD;
        tools.http_method_map["OPTIONS"] = M_OPTIONS;
    }

    void cjsruntime::destroy() {
        using namespace std::chrono_literals;
        while (!global_http.caches.empty()) {
            for (auto k = global_http.caches.begin(); k != global_http.caches.end();) {
                auto& s = *k;
                if (s->state == 1) {
                    if (s->fut.wait_for(10ms) == std::future_status::ready) {
                        k = global_http.caches.erase(k);
                    }
                    else {
                        k++;
                    }
                }
                else {
                    k = global_http.caches.erase(k);
                }
            }
        }
    }
}
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
        permanents._null = _new_null(js_value::at_const);
        permanents._undefined = _new_undefined(js_value::at_const);
        permanents._true = _new_boolean(true, js_value::at_const);
        permanents._false = _new_boolean(false, js_value::at_const);
        permanents.__nan = _new_number(std::numeric_limits<double>::quiet_NaN(), js_value::at_const);
        permanents._inf = _new_number(INFINITY, js_value::at_const);
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
        auto _int_3 = _new_number(1, js_value::at_const | js_value::at_refs);
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
            func->stack.push_back(js.new_boolean(obj->exists(args.front().lock()->to_string(&js, 0, &r))));
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
            auto f = o->get_special("PrimitiveValue");
            if (f) {
                func->stack.push_back(f);
                return 0;
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
                    std::vector<std::tuple<std::string, bool, int>> matches;
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
                            args.push_back(js.new_number(std::get<2>(s)));
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
                    auto f = origin.find(pat);
                    if (f != std::string::npos) {
                        std::vector<js_value::weak_ref> args;
                        args.push_back(js.new_string(pat));
                        args.push_back(js.new_number(f));
                        auto v = js.fast_api(rep, _this, args, 0, &r);
                        if (r != 0)
                            return r;
                        auto v2 = v->to_string(&js, 0, &r);
                        if (r != 0)
                            return r;
                        func->stack.push_back(js.new_string(jsv_regexp::replace(origin, pat, v2)));
                        return 0;
                    }
                    else {
                        func->stack.push_back(_this);
                        return 0;
                    }
                }
            }
        };
        permanents._proto_string->add(permanents._proto_string_replace->name, permanents._proto_string_replace);
        permanents._proto_string_match = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_string_match->add("length", _int_1);
        permanents._proto_string_match->name = "match";
        permanents._proto_string_match->builtin = [](auto& func, auto& _this, auto& __args, auto& js, auto attr) {
            auto f = _this.lock();
            std::vector<js_value::ref> matches;
            if (__args.empty()) {
                matches.push_back(js.new_string(""));
                auto arr = js.new_array(matches);
                arr->add("index", js.new_number(0));
                arr->add("input", _this);
                func->stack.push_back(arr);
                return 0;
            }
            auto r = 0;
            auto origin = f->to_string(&js, 0, &r);
            if (r != 0)
                return r;
            auto a = __args.front().lock();
            if (a->get_type() != r_regex) {
                auto pat = a->to_string(&js, 0, &r);
                if (r != 0)
                    return r;
                if (pat.empty()) {
                    matches.push_back(js.new_string(""));
                    auto arr = js.new_array(matches);
                    arr->add("index", js.new_number(0));
                    arr->add("input", _this);
                    func->stack.push_back(arr);
                }
                auto ff = origin.find(pat);
                if (ff != std::string::npos) {
                    matches.push_back(a);
                    auto arr = js.new_array(matches);
                    arr->add("index", js.new_number(ff));
                    arr->add("input", _this);
                    func->stack.push_back(arr);
                    return 0;
                }
                else {
                    func->stack.push_back(js.new_null());
                    return 0;
                }
            }
            else {
                auto re = JS_RE(a);
                std::vector<std::tuple<std::string, int>> m;
                if (re->match(origin, m)) {
                    auto k = std::get<1>(m.front());
                    for (const auto& j : m) {
                        matches.push_back(js.new_string(std::get<0>(j)));
                    }
                    auto arr = js.new_array(matches);
                    if (k != -1) {
                        arr->add("index", js.new_number(k));
                        arr->add("input", _this);
                    }
                    func->stack.push_back(arr);
                    return 0;
                }
                else {
                    func->stack.push_back(js.new_null());
                    return 0;
                }
            }
        };
        permanents._proto_string->add(permanents._proto_string_match->name, permanents._proto_string_match);
        permanents._proto_string_substring = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_string_substring->add("length", _int_1);
        permanents._proto_string_substring->name = "substring";
        permanents._proto_string_substring->builtin = [](auto& func, auto& _this, auto& __args, auto& js, auto attr) {
            auto f = _this.lock();
            if (f->get_type() != r_string) {
                func->stack.push_back(js.new_undefined());
                return 0;
            }
            auto s = JS_S(f)->wstr;
            if (__args.empty()) {
                func->stack.push_back(_this);
                return 0;
            }
            auto _begin = __args.front().lock();
            auto r = 0;
            auto begin0 = _begin->to_number(&js, &r);
            if (r != 0)
                return r;
            if (std::isnan(begin0) || std::isinf(begin0)) {
                func->stack.push_back(js.new_string(""));
                return 0;
            }
            auto begin = (int)begin0;
            if (begin0 < 0 || begin0 >= s.length()) {
                func->stack.push_back(js.new_string(""));
                return 0;
            }
            if (__args.size() >= 2) {
                auto _end = __args[1].lock();
                auto r = 0;
                auto end0 = _end->to_number(&js, &r);
                if (r != 0)
                    return r;
                if (std::isnan(end0) || std::isinf(end0)) {
                    func->stack.push_back(js.new_string(""));
                    return 0;
                }
                auto end = (int)end0;
                if (end < 0) {
                    func->stack.push_back(js.new_string(""));
                    return 0;
                }
                if (end < begin) {
                    func->stack.push_back(js.new_string(""));
                    return 0;
                }
                func->stack.push_back(js.new_string(CString(s.substr(begin, end - begin).c_str())));
            }
            else {
                func->stack.push_back(js.new_string(CString(s.substr(begin).c_str())));
            }
            return 0;
        };
        permanents._proto_string->add(permanents._proto_string_substring->name, permanents._proto_string_substring);
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
        permanents.sys_music = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.sys_music->add("length", _int_1);
        permanents.sys_music->name = "music";
        permanents.sys_music->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_music, _this, args, 0);
        };
        permanents.sys->add(permanents.sys_music->name, permanents.sys_music);
        permanents.sys_config = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.sys_config->add("length", _int_1);
        permanents.sys_config->name = "config";
        permanents.sys_config->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_config, _this, args, 0);
        };
        permanents.sys->add(permanents.sys_config->name, permanents.sys_config);
        permanents.sys_get_config = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.sys_get_config->add("length", _int_1);
        permanents.sys_get_config->name = "get_config";
        permanents.sys_get_config->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_get_config, _this, args, 0);
        };
        permanents.sys->add(permanents.sys_get_config->name, permanents.sys_get_config);
        permanents.sys_math = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.sys_math->add("length", _int_1);
        permanents.sys_math->name = "math";
        permanents.sys_math->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_math, _this, args, 0);
        };
        permanents.sys->add(permanents.sys_math->name, permanents.sys_math);
        permanents.sys_helper = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.sys_helper->add("length", _int_1);
        permanents.sys_helper->name = "helper";
        permanents.sys_helper->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_helper, _this, args, 0);
        };
        permanents.sys->add(permanents.sys_helper->name, permanents.sys_helper);
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
                        pri = js.new_number(std::numeric_limits<double>::quiet_NaN());
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
                if (args.size() == 1) {
                    if (args.front().lock()->get_type() == r_number)
                        arr->add("length", args.front());
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
        // buffer
        permanents._proto_buffer = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_buffer->add("__type__", _new_string("Buffer", js_value::at_const | js_value::at_refs));
        permanents._proto_buffer->__proto__ = permanents._proto_array;
        permanents._proto_buffer_toString = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_buffer_toString->add("length", _int_0);
        permanents._proto_buffer_toString->name = "toString";
        permanents._proto_buffer_toString->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_buffer_toString, _this, args, 0);
        };
        permanents._proto_buffer->add(permanents._proto_buffer_toString->name, permanents._proto_buffer_toString);
        permanents.f_buffer = _new_function(permanents._proto_buffer, js_value::at_const | js_value::at_readonly);
        permanents.f_buffer->add("length", _int_1);
        permanents.f_buffer->name = "Buffer";
        permanents.f_buffer->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_buffer_from, _this, args, 0);
        };
        permanents.global_env->add(permanents.f_buffer->name, permanents.f_buffer);
        permanents.f_buffer_from = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.f_buffer_from->add("length", _int_3);
        permanents.f_buffer_from->name = "from";
        permanents.f_buffer_from->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_buffer_from, _this, args, 0);
        };
        permanents.f_buffer->add(permanents.f_buffer_from->name, permanents.f_buffer_from);
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
        // ui
        permanents._proto_ui = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_ui->add("__type__", _new_string("UI", js_value::at_const | js_value::at_refs));
        permanents._proto_ui->__proto__ = permanents._proto_array;
        permanents.f_ui = _new_function(permanents._proto_ui, js_value::at_const | js_value::at_readonly);
        permanents.f_ui->add("length", _int_1);
        permanents.f_ui->name = "UI";
        permanents.f_ui->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_UI_new, _this, args, 0);
        };
        permanents._proto_ui_render_internal = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_ui_render_internal->add("length", _int_2);
        permanents._proto_ui_render_internal->name = "render_internal";
        permanents._proto_ui_render_internal->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_UI_render, _this, args, 0);
        };
        permanents._proto_ui->add(permanents._proto_ui_render_internal->name, permanents._proto_ui_render_internal);
        permanents.global_env->add(permanents.f_ui->name, permanents.f_ui);
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
        permanents.global_parseInt = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.global_parseInt->add("length", _int_1);
        permanents.global_parseInt->name = "parseInt";
        permanents.global_parseInt->builtin = [](auto& func, auto& _this, auto& args, auto& js, auto attr) {
            return js.call_api(API_parseInt, _this, args, 0);
        };
        permanents.global_env->add(permanents.global_parseInt->name, permanents.global_parseInt);
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

    std::string cjsruntime::js_base64_encode(const std::vector<char>& data)
    {
        static const char* base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        unsigned char current;
        auto& bindata = data;
        auto binlength = data.size();
        std::string str;

        for (size_t i = 0; i < binlength; i += 3)
        {
            current = (((byte)bindata[i]) >> 2);
            current &= (byte)0x3F;
            str.push_back(base64char[(int)current]);

            current = ((byte)(bindata[i] << 4)) & ((byte)0x30);
            if (i + 1 >= binlength)
            {
                str.push_back(base64char[(int)current]);
                str.push_back('=');
                str.push_back('=');
                break;
            }
            current |= ((byte)(bindata[i + 1] >> 4)) & ((byte)0x0F);
            str.push_back(base64char[(int)current]);

            current = ((byte)(bindata[i + 1] << 2)) & ((byte)0x3C);
            if (i + 2 >= binlength)
            {
                str.push_back(base64char[(int)current]);
                str.push_back('=');
                break;
            }
            current |= ((byte)(bindata[i + 2] >> 6)) & ((byte)0x03);
            str.push_back(base64char[(int)current]);

            current = ((byte)bindata[i + 2]) & ((byte)0x3F);
            str.push_back(base64char[(int)current]);
        }

        return str;
    }

    std::vector<char> cjsruntime::js_base64_decode(const std::string& data)
    {
        static const char* base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        byte k;
        byte temp[4];
        auto len = data.size();
        std::vector<char> bindata;
        for (size_t i = 0, j = 0; i < len; i += 4)
        {
            memset(temp, 0xFF, sizeof(temp));
            for (k = 0; k < 64; k++)
            {
                if (base64char[k] == data[i])
                    temp[0] = k;
            }
            for (k = 0; k < 64; k++)
            {
                if (base64char[k] == data[i + 1])
                    temp[1] = k;
            }
            for (k = 0; k < 64; k++)
            {
                if (base64char[k] == data[i + 2])
                    temp[2] = k;
            }
            for (k = 0; k < 64; k++)
            {
                if (base64char[k] == data[i + 3])
                    temp[3] = k;
            }

            bindata.push_back((char)(((byte)(((byte)(temp[0] << 2)) & 0xFC)) |
                ((byte)((byte)(temp[1] >> 4) & 0x03))));
            if (data[i + 2] == '=')
                break;

            bindata.push_back((char)(((byte)(((byte)(temp[1] << 4)) & 0xF0)) |
                ((byte)((byte)(temp[2] >> 2) & 0x0F))));
            if (data[i + 3] == '=')
                break;

            bindata.push_back((char)(((byte)(((byte)(temp[2] << 6)) & 0xF0)) |
                ((byte)(temp[3] & 0x3F))));
        }
        return bindata;
    }

    static CStringA StringTToUtf8(CString str)
    {
        USES_CONVERSION;
        auto length = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
        CStringA s;
        auto buf = s.GetBuffer(length + 1);
        ZeroMemory(buf, (length + 1) * sizeof(CHAR));
        WideCharToMultiByte(CP_UTF8, 0, str, -1, buf, length, nullptr, nullptr);
        return s;
    }

    static CString Utf8ToStringT(LPCSTR str)
    {
        _ASSERT(str);
        USES_CONVERSION;
        auto length = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
        CString s;
        auto buf = s.GetBuffer(length + 1);
        ZeroMemory(buf, (length + 1) * sizeof(WCHAR));
        MultiByteToWideChar(CP_UTF8, 0, str, -1, buf, length);
        s.ReleaseBuffer();
        return s;
    }

    int cjsruntime::call_api(int type, js_value::weak_ref& _this,
        std::vector<js_value::weak_ref>& args, uint32_t attr) {
        auto r = 0;
        switch ((js_value_new::api) type) {
        case API_none:
            break;
        case API_setTimeout:
        case API_setInterval: {
            if (args.empty()) {
                push(new_undefined());
                break;
            }
            auto arg = args.front().lock();
            if (arg->get_type() != r_function) {
                push(new_undefined());
                break;
            }
            auto arg_n = 1;
            auto time = 0;
            if (args.size() > 1) {
                if (args[1].lock()->get_type() != r_undefined) {
                    auto r = 0;
                    time = (int)args[1].lock()->to_number(this, &r);
                    if (r != 0)
                        return r;
                }
                arg_n++;
            }
            if (r != 0)
                return r;
            std::vector<js_value::weak_ref> _args(args.begin() + arg_n, args.end());
            push(new_number(api_setTimeout(time, JS_FUN(arg), _args, attr, type == API_setTimeout)));
        }
                            break;
        case API_clearTimeout:
        case API_clearInterval: {
            if (args.empty()) {
                push(new_undefined());
                break;
            }
            api_clearTimeout(args.front().lock()->to_number(this, &r));
            if (r != 0)
                return r;
            push(new_undefined());
        }
                              break;
        case API_eval: {
            if (args.empty()) {
                push(new_undefined());
                break;
            }
            auto code = args.front().lock()->to_string(this, 0, &r);
            if (r != 0)
                return r;
            current_stack->pc++;
            return exec("<eval>", code, true, false);
        }
                     break;
        case API_http: {
            if (args.empty()) {
                push(new_undefined());
                break;
            }
            auto obj = args.front().lock();
            if (obj->is_primitive()) {
                push(new_string("missing args"));
                break;
            }
            auto code = JS_O(obj);
            auto http = std::make_shared<http_struct>();
            auto r = 0;
            // url
            auto v = code->get("url", this);
            if (!v) {
                push(new_string("missing url"));
                break;
            }
            http->url = v->to_string(this, 0, &r);
            if (r != 0)
                return r;
            // method
            v = code->get("method", this);
            if (!v) {
                http->method = "get";
            }
            else {
                http->method = v->to_string(this, 0, &r);
                transform(http->method.begin(), http->method.end(), http->method.begin(), ::toupper);
                if (r != 0)
                    return r;
                auto m = tools.http_method_map.find(http->method);
                if (m == tools.http_method_map.end()) {
                    push(new_string("invalid method: " + http->method));
                    break;
                }
                http->method_type = m->second;
            }
            // headers
            v = code->get("headers", this);
            if (v && !v->is_primitive()) {
                auto headers = JS_O(v);
                auto o = headers->get_obj();
                for (const auto& x : o) {
                    const auto& key = x.first;
                    const auto& value = x.second;
                    auto _v = value.lock()->to_string(this, 0, &r);
                    if (r != 0)
                        return r;
                    http->headers.push_back({ key, _v });
                }
            }
            // payload
            v = code->get("payload", this);
            if (v) {
                http->payload = v->to_string(this, 0, &r);
                if (r != 0)
                    return r;
            }
            // callback
            v = code->get("callback", this);
            if (v && v->get_type() == r_function) {
                http->callback = JS_FUN(v);
            }
            // last
            global_http.caches.emplace_back(http);
            push(new_boolean(true));
        }
                     break;
        case API_music: {
            if (args.empty()) {
                push(new_undefined());
                break;
            }
            auto obj = args.front().lock();
            if (obj->is_primitive()) {
                push(new_string("missing args"));
                break;
            }
            auto code = JS_O(obj);
            auto r = 0;
            // method
            auto v = code->get("method", this);
            if (!v) {
                push(new_string("missing method"));
                break;
            }
            auto method = v->to_string(this, 0, &r);
            if (r != 0)
                return r;
            if (method == "play") {
                auto _payload = code->get("payload", this);
                if (!_payload) {
                    push(new_string("missing payload"));
                    break;
                }
                if (_payload->is_primitive()) {
                    push(new_string("payload must be object"));
                    break;
                }
                auto payload = JS_O(_payload);
                auto _data = payload->get("data", this);
                if (!_data) {
                    push(new_string("missing payload: data"));
                    break;
                }
                if (_data->__proto__.lock() != permanents._proto_buffer) {
                    push(new_string("data must be type of buffer"));
                    break;
                }
                auto data = JS_O(_data)->get_buffer();
                auto _title = payload->get("title", this);
                std::string title;
                if (_title) {
                    title = _title->to_string(this, 0, &r);
                    if (r != 0)
                        return r;
                }
                auto _ext = payload->get("type", this);
                std::string ext = "mp3";
                if (_ext) {
                    ext = _ext->to_string(this, 0, &r);
                    if (r != 0)
                        return r;
                }
                push(new_number(cjsgui::singleton().play_music(title, ext, data)));
            }
            else {
                push(new_string("invalid method"));
                break;
            }
        }
                      break;
        case API_buffer_from: {
            auto buf = new_buffer();
            if (!args.empty()) {
                auto arg_1 = args[0].lock();
                if (arg_1->get_type() == r_string) {
                    auto str = JS_STR(arg_1);
                    std::string type = "utf8";
                    auto r = 0;
                    if (args.size() > 1) {
                        auto _type = args[1].lock()->to_string(this, 0, &r);
                        if (r != 0)
                            return r;
                        std::transform(_type.begin(), _type.end(), _type.begin(), ::tolower);
                        if (_type != "utf-8")
                            type = _type;
                    }
                    auto wstr = CString(CStringA(str.c_str()));
                    std::vector<char> data;
                    if (type == "utf8") {
                        auto u = std::string(StringTToUtf8(wstr).GetBuffer(0));
                        data.resize(u.size());
                        std::copy(u.begin(), u.end(), data.begin());
                    }
                    else if (type == "ascii") {
                        data.resize(wstr.GetLength());
                        for (auto i = 0; i < wstr.GetLength(); i++) {
                            data[i] = (char)(((unsigned int)wstr[i]) & 0xff);
                        }
                    }
                    else if (type == "base64") {
                        data = js_base64_decode(str);
                    }
                    buf->set_buffer(*this, data);
                    push(buf);
                    break;
                }
                if (arg_1->__proto__.lock() == permanents._proto_buffer) {
                    auto o = JS_O(arg_1);
                    buf->set_buffer(*this, o->get_buffer());
                    push(buf);
                    break;
                }
                if (arg_1->__proto__.lock() == permanents._proto_array) {
                    auto o = JS_O(arg_1);
                    std::vector<char> data;
                    auto len = o->get("length", this);
                    size_t i = 0;
                    size_t idx = 0;
                    if (len->get_type() == r_number) {
                        idx = (size_t)JS_NUM(len);
                        if (idx < INT_MAX) {
                            data.resize(idx);
                            auto obj = o->get_obj();
                            std::stringstream ss;
                            for (i = 0; i < idx; i++) {
                                ss.str("");
                                ss << i;
                                auto f = obj.find(ss.str());
                                if (f == obj.end())
                                    break;
                                auto t = f->second.lock();
                                if (t->get_type() != r_number)
                                    break;
                                auto num = JS_NUM(t);
                                if (isnan(num) || isinf(num))
                                    break;
                                if (num < 0)
                                    num = 0;
                                if (num > 255)
                                    num = 255;
                                data[i] = (char)(byte)num;
                            }
                        }
                    }
                    if (i != idx)
                        data.clear();
                    buf->set_buffer(*this, data);
                    push(buf);
                    break;
                }
            }
            std::vector<char> data;
            buf->set_buffer(*this, data);
            push(buf);
        }
                      break;
        case API_buffer_toString: {
            auto r = 0;
            auto th = _this.lock();
            assert(th);
            if (!th->is_primitive() && th->__proto__.lock() == permanents._proto_buffer) {
                std::string enc = "utf8";
                if (!args.empty()) {
                    auto arg_1 = args[0].lock();
                    if (arg_1->get_type() == r_string) {
                        enc = JS_STR(arg_1);
                        std::transform(enc.begin(), enc.end(), enc.begin(), ::tolower);
                    }
                }
                auto __this = JS_O(th);
                auto buf = __this->get_buffer();
                if (enc == "utf8" || enc == "utf-8") {
                    buf.push_back(0);
                    auto wstr = Utf8ToStringT(buf.data());
                    push(new_string(wstr));
                    break;
                }
                else if (enc == "base64") {
                    push(new_string(js_base64_encode(buf)));
                    break;
                }
            }
            push(new_string(_this.lock()->to_string(this, 2, &r)));
            return r;
        }
                            break;
        case API_config: {
            if (args.empty()) {
                push(new_undefined());
                break;
            }
            auto obj = args.front().lock();
            if (obj->is_primitive()) {
                push(new_string("missing args"));
                break;
            }
            auto code = JS_O(obj);
            auto r = 0;
            // input
            auto v = code->get("input", this);
            if (v) {
                if (v->is_primitive()) {
                    push(new_string("invalid input"));
                    break;
                }
                auto _input = JS_O(v);
                auto input = _input->get_obj();
                auto f = input.find("enable");
                if (f != input.end()) {
                    cjsgui::singleton().input_set(f->second.lock()->to_bool());
                }
                f = input.find("single");
                if (f != input.end()) {
                    GLOBAL_STATE.input_s->input_single = (f->second.lock()->to_bool());
                }
                push(new_undefined());
                break;
            }
            push(new_string("invalid config"));
        }
                                break;
        case API_get_config: {
            if (args.empty()) {
                push(new_undefined());
                break;
            }
            auto obj = args.front().lock();
            if (obj->get_type() != r_string) {
                push(new_undefined());
                break;
            }
            auto type = JS_STR(obj);
            auto var = std::split(type, '/');
            if (var.size() >= 2) {
                if (var[0] == "screen") {
                    if (var[1] == "width") {
                        push(new_number(GLOBAL_STATE.bound.Width()));
                        break;
                    }
                    else if (var[1] == "height") {
                        push(new_number(GLOBAL_STATE.bound.Height()));
                        break;
                    }
                }
                else if (var[0] == "hit") {
                    if (var[1] == "obj") {
                        push(global_ui.hit.lock() ? global_ui.hit : new_undefined());
                        break;
                    }
                    else if (var[1] == "type") {
                        push(new_string(global_ui.hit_n));
                        break;
                    }
                    else if (var[1] == "x") {
                        push(new_number(global_ui.hit_x));
                        break;
                    }
                    else if (var[1] == "y") {
                        push(new_number(global_ui.hit_y));
                        break;
                    }
                }
                else if (var[0] == "time") {
                    if (var[1] == "timestamp") {
                        using namespace std::chrono;
                        auto n = system_clock::now();
                        push(new_number((double)duration_cast<milliseconds>(n.time_since_epoch()).count()));
                        break;
                    }
                }
            }
            push(new_undefined());
        }
                       break;
        case API_math: {
            if (args.size() < 2) {
                push(new_number(JS_NAN));
                break;
            }
            auto r = 0;
            auto id = args.front().lock()->to_number(this, &r);
            if (r != 0)
                return r;
            auto n = (int)id;
            if (n > 100 && n < 200) {
                auto a = args[1].lock()->to_number(this, &r);
                if (r != 0)
                    return r;
                switch (n) {
                case 101:
                    push(new_number(floor(a)));
                    return 0;
                case 102:
                    push(new_number(ceil(a)));
                    return 0;
                case 103:
                    push(new_number(round(a)));
                    return 0;
                }
            }
            if (n > 200 && n < 300 && args.size() >= 3) {
                auto a = args[1].lock()->to_number(this, &r);
                if (r != 0)
                    return r;
                auto b = args[2].lock()->to_number(this, &r);
                if (r != 0)
                    return r;
                switch (n) {
                case 201:
                    push(new_number(max(a, b)));
                    return 0;
                case 202:
                    push(new_number(min(a, b)));
                    return 0;
                }
            }
            push(new_number(JS_NAN));
        }
                           break;
        case API_UI_new: {
            if (args.empty()) {
                push(new_undefined());
                break;
            }
            auto a = args.front().lock();
            if (a->is_primitive()) {
                push(new_undefined());
                break;
            }
            auto ui = std::make_shared<jsv_ui>();
            if (attr > 0U) ui->attr = attr;
            ui->__proto__ = permanents._proto_ui;
            if (ui->init(JS_O(a), this)) {
                register_value(ui);
                global_ui.elements.insert(ui);
                push(ui);
                break;
            }
            push(new_undefined());
        }
                       break;
        case API_UI_render: {
            auto o = _this.lock();
            do {
                if (!o || o->is_primitive() || args.empty()) {
                    break;
                }
                auto obj = JS_O(o);
                if (obj->get_object_type() != jsv_object::T_UI)
                    break;
                auto r = 0;
                auto n = args.front().lock()->to_number(this, &r);
                if (r != 0)
                    return r;
                auto ui = JS_UI(obj);
                if (n == 1.0f)
                    GLOBAL_STATE.render_queue_auto.push_back(ui);
                else if (n == 2.0f)
                    GLOBAL_STATE.render_queue_auto_bk.push_back(ui);
            } while (0);
            push(new_undefined());
        }
                       break;
        case API_helper: {
            js_value::ref ret = new_undefined();
            auto r = call_helper(args, ret);
            push(ret);
            return r;
        }
                          break;
        case API_parseInt: {
            if (args.empty()) {
                push(new_number(JS_NAN));
                break;
            }
            auto a = args.front().lock();
            if (a->get_type() == r_number) { 
                push(a);
                break;
            }
            auto r = 0;
            auto n = a->to_number(this, &r);
            if (r != 0)
                return r;
            push(new_number(n));
        }
                       break;
        default:
            break;
        }
        return 0;
    }
}
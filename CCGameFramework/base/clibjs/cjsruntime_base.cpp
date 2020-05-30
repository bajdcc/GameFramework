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

    void cjsruntime::init(void *p) {
        pjs = p;
        // proto
        permanents._proto_root = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_number = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_string = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_boolean = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_object = _new_object(js_value::at_const | js_value::at_readonly);
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
        auto _empty_string = _new_string("", js_value::at_const | js_value::at_refs);
        // proto
        permanents._proto_root->obj["__type__"] = _new_string("Root", js_value::at_const | js_value::at_refs);
        permanents._proto_object->obj["__type__"] = _new_string("Object", js_value::at_const | js_value::at_refs);
        permanents._proto_object_hasOwnProperty = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_object_hasOwnProperty->obj.insert({"length", _int_1});
        permanents._proto_object_hasOwnProperty->name = "hasOwnProperty";
        permanents._proto_object_hasOwnProperty->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            if (args.empty()) {
                func->stack.push_back(js.new_boolean(false));
                return 0;
            }
            auto f = _this.lock();
            if (f->get_type() != r_object) {
                func->stack.push_back(js.new_boolean(false));
                return 0;
            }
            const auto &obj = JS_OBJ(f);
            func->stack.push_back(js.new_boolean(obj.find(args.front().lock()->to_string(&js, 0)) != obj.end()));
            return 0;
        };
        permanents._proto_object->obj.insert({permanents._proto_object_hasOwnProperty->name, permanents._proto_object_hasOwnProperty});
        permanents._proto_object->obj["__type__"] = _new_string("Object", js_value::at_const | js_value::at_refs);
        permanents._proto_object_toString = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_object_toString->obj.insert({"length", _int_0});
        permanents._proto_object_toString->name = "toString";
        permanents._proto_object_toString->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            func->stack.push_back(js.new_string(_this.lock()->to_string(&js, 0)));
            return 0;
        };
        permanents._proto_object->obj.insert({permanents._proto_object_toString->name, permanents._proto_object_toString});
        permanents._proto_boolean->obj["__type__"] = _new_string("Boolean", js_value::at_const | js_value::at_refs);
        permanents._proto_function->obj["__type__"] = _new_string("Function", js_value::at_const | js_value::at_refs);
        permanents._proto_function_call = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_function_call->obj.insert({"length", _int_1});
        permanents._proto_function_call->name = "call";
        permanents._proto_function_call->builtin = [](auto &func, auto &_this, auto &__args, auto &js, auto attr) {
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
        permanents._proto_function->obj.insert({permanents._proto_function_call->name, permanents._proto_function_call});
        permanents._proto_function_apply = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._proto_function_apply->obj.insert({"length", _int_1});
        permanents._proto_function_apply->name = "apply";
        permanents._proto_function_apply->builtin = [](auto &func, auto &_this, auto &__args, auto &js, auto attr) {
            auto f = _this.lock();
            if (f->get_type() != r_function) {
                func->stack.push_back(js.new_undefined());
                return 0;
            }
            auto fun = JS_FUN(f);
            auto __this = __args.empty() ? _this : __args.front();
            auto args = __args.size() > 1 ?
                        to_array(__args[1].lock()) :
                        std::vector<js_value::weak_ref>();
            return js.call_api(fun, __this, args, jsv_function::at_fast);
        };
        permanents._proto_function->obj.insert({permanents._proto_function_apply->name, permanents._proto_function_apply});
        permanents._proto_number->obj["__type__"] = _new_string("Number", js_value::at_const | js_value::at_refs);
        permanents._proto_string->obj["__type__"] = _new_string("String", js_value::at_const | js_value::at_refs);
#if DUMP_PRINT_FILE_ENABLE
        permanents._debug_print = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._debug_print->obj.insert({"length", _int_1});
#if DUMP_PRINT_FILE_AUTO_CLEAR
        permanents._debug_print->name = "debug_print";
        {
            std::ofstream ofs(DUMP_PRINT_FILE);
        }
#endif
#endif
#if DUMP_PRINT_FILE_ENABLE
        permanents._debug_print->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
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
        permanents.global_env->obj.insert({permanents._debug_print->name, permanents._debug_print});
#endif
        permanents.global_env->obj.insert({"NaN", permanents.__nan});
        permanents.global_env->obj.insert({"Infinity", permanents._inf});
        // debug
        permanents._debug_dump = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents._debug_dump->obj.insert({"length", _int_1});
        permanents._debug_dump->name = "debug_dump";
        permanents._debug_dump->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            char buf[128];
            auto t = _this.lock();
            assert(t);
            auto type = t->unary_op(js, UNARY_TYPEOF)->to_string(&js, 0);
            auto proto = t->__proto__.lock();
            auto p = std::string("none");
            if (proto->get_type() == r_object) {
                const auto &o = JS_OBJ(proto);
                auto of = o.find("__type__");
                if (of != o.end()) {
                    p = of->second.lock()->to_string(&js, 0);
                }
            }
            snprintf(buf, sizeof(buf),
                     "Str: %s, Type: %s, Proto: %s, Ptr: %p",
                     t->to_string(&js, 0).c_str(), type.c_str(), p.c_str(), t.get());
            func->stack.push_back(js.new_string(buf));
            return 0;
        };
        permanents._proto_root->obj.insert({permanents._debug_dump->name, permanents._debug_dump});
        // console
        permanents.console = _new_object(js_value::at_const | js_value::at_readonly);
        permanents.console_log = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.console_log->obj.insert({"length", _int_1});
        permanents.console_log->name = "log";
        permanents.console_log->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            std::stringstream ss;
            for (size_t i = 0; i < args.size(); i++) {
                ss << args[i].lock()->to_string(&js, 1);
                if (i + 1 < args.size())
                    ss << " ";
            }
            ss << std::endl;
            auto s = ss.str();
            cjsgui::singleton().put_string(ss.str());
            func->stack.push_back(js.new_undefined());
            return 0;
        };
        permanents.console->obj.insert({permanents.console_log->name, permanents.console_log});
        permanents.console_error = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.console_error->obj.insert({"length", _int_1});
        permanents.console_error->name = "error";
        permanents.console_error->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            std::stringstream ss;
            for (size_t i = 0; i < args.size(); i++) {
                ss << args[i].lock()->to_string(&js, 1);
                if (i + 1 < args.size())
                    ss << " ";
            }
            ss << std::endl;
            auto s = ss.str();
            cjsgui::singleton().put_string(ss.str());
            func->stack.push_back(js.new_undefined());
            return 0;
        };
        permanents.console->obj.insert({permanents.console_error->name, permanents.console_error});
        permanents.console_trace = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.console_trace->obj.insert({"length", _new_number(0, js_value::at_const | js_value::at_refs)});
        permanents.console_trace->name = "trace";
        permanents.console_trace->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            func->stack.push_back(js.new_string(js.get_stacktrace()));
            return 0;
        };
        permanents.console->obj.insert({permanents.console_trace->name, permanents.console_trace});
        permanents.global_env->obj.insert({"console", permanents.console});
        // sys
        permanents.sys = _new_object(js_value::at_const | js_value::at_readonly);
        permanents.sys_exec_file = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.sys_exec_file->obj.insert({"length", _int_1});
        permanents.sys_exec_file->name = "exec_file";
        permanents.sys_exec_file->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            if (args.empty() || args.front().lock()->get_type() != r_string) {
                func->stack.push_back(js.new_undefined());
                return 0;
            }
            auto filename = args.front().lock()->to_string(&js, 0);
            std::string content;
            if (js.get_file(filename, content)) {
                func->pc++;
                js.exec(filename, content);
                return 1;
            }
            func->stack.push_back(js.new_undefined());
            return 0;
        };
        permanents.sys->obj.insert({permanents.sys_exec_file->name, permanents.sys_exec_file});
        permanents.sys_builtin = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.sys_builtin->obj.insert({"length", _int_1});
        permanents.sys_builtin->name = "builtin";
        permanents.sys_builtin->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            if (args.empty() || args.front().lock()->is_primitive()) {
                func->stack.push_back(js.new_boolean(false));
                return 0;
            }
            func->stack.push_back(js.new_boolean(js.set_builtin(JS_O(args.front().lock()))));
            return 0;
        };
        permanents.sys->obj.insert({permanents.sys_builtin->name, permanents.sys_builtin});
        permanents.global_env->obj.insert({"sys", permanents.sys});
        // number
        permanents.f_number = _new_function(permanents._proto_number, js_value::at_const | js_value::at_readonly);
        permanents.f_number->obj.insert({"length", _int_1});
        permanents.f_number->name = "Number";
        permanents.f_number->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            js_value::ref pri;
            if (args.empty()) {
                pri = js.new_number(0.0);
            } else {
                auto n = args.front().lock();
                double d = 0.0;
                if (to_number(n, d)) {
                    pri = js.new_number(d);
                } else {
                    pri = js.new_number(NAN);
                }
            }
            if (attr & jsv_function::at_new_function) {
                pri = js.new_object_box(pri);
            }
            func->stack.push_back(pri);
            return 0;
        };
        permanents.global_env->obj.insert({permanents.f_number->name, permanents.f_number});
        // boolean
        permanents.f_boolean = _new_function(permanents._proto_boolean, js_value::at_const | js_value::at_readonly);
        permanents.f_boolean->obj.insert({"length", _int_1});
        permanents.f_boolean->name = "Boolean";
        permanents.f_boolean->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            js_value::ref pri;
            if (args.empty()) {
                pri = js.new_boolean(false);
            } else {
                pri = js.new_boolean(args.front().lock()->to_bool());
            }
            if (attr & jsv_function::at_new_function) {
                pri = js.new_object_box(pri);
            }
            func->stack.push_back(pri);
            return 0;
        };
        permanents.global_env->obj.insert({permanents.f_boolean->name, permanents.f_boolean});
        // object
        permanents.f_object = _new_function(permanents._proto_object, js_value::at_const | js_value::at_readonly);
        permanents.f_object->obj.insert({"length", _int_1});
        permanents.f_object->name = "Object";
        permanents.f_object->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            js_value::ref pri;
            if (args.empty()) {
                pri = js.new_boolean(false);
            } else {
                pri = js.new_boolean(args.front().lock()->to_bool());
            }
            if (attr & jsv_function::at_new_function) {
                pri = js.new_object_box(pri);
            }
            func->stack.push_back(pri);
            return 0;
        };
        permanents.global_env->obj.insert({permanents.f_object->name, permanents.f_object});
        // string
        permanents.f_string = _new_function(permanents._proto_string, js_value::at_const | js_value::at_readonly);
        permanents.f_string->obj.insert({"length", _int_1});
        permanents.f_string->name = "String";
        permanents.f_string->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            js_value::ref pri;
            if (args.empty()) {
                pri = js.new_string("");
            } else {
                pri = js.new_string(args.front().lock()->to_string(&js, 0));
            }
            if (attr & jsv_function::at_new_function) {
                pri = js.new_object_box(pri);
            }
            func->stack.push_back(pri);
            return 0;
        };
        permanents.global_env->obj.insert({permanents.f_string->name, permanents.f_string});
        // function
        permanents.f_function = _new_function(permanents._proto_function, js_value::at_const | js_value::at_readonly);
        permanents.f_function->obj.insert({"length", _int_1});
        permanents.f_function->name = "Function";
        permanents.f_function->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            js_value::ref pri;
            if (args.empty()) {
                pri = js.new_boolean(false);
            } else {
                pri = js.new_boolean(args.front().lock()->to_bool());
            }
            if (attr & jsv_function::at_new_function) {
                pri = js.new_object_box(pri);
            }
            func->stack.push_back(pri);
            return 0;
        };
        permanents.global_env->obj.insert({permanents.f_function->name, permanents.f_function});
        // array
        permanents._proto_array = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_array->obj["__type__"] = _new_string("Array", js_value::at_const | js_value::at_refs);
        permanents.f_array = _new_function(permanents._proto_array, js_value::at_const | js_value::at_readonly);
        permanents.f_array->obj.insert({"length", _int_1});
        permanents.f_array->name = "Array";
        permanents.f_array->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            js_value::ref pri;
            if (args.empty()) {
                pri = js.new_array();
            } else {
                auto arr = js.new_array();
                if (args.size() == 1 && args.front().lock()->get_type() == r_number) {
                    arr->obj["length"] = args.front().lock();
                } else {
                    auto i = 0;
                    for (const auto &s : args) {
                        std::stringstream ss;
                        ss << i++;
                        arr->obj.insert({ss.str(), s});
                    }
                    arr->obj["length"] = js.new_number(i);
                }
                pri = arr;
            }
            func->stack.push_back(pri);
            return 0;
        };
        permanents.global_env->obj.insert({permanents.f_array->name, permanents.f_array});
        // global function
        permanents.global_setTimeout = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.global_setTimeout->obj.insert({"length", _int_1});
        permanents.global_setTimeout->name = "setTimeout";
        permanents.global_setTimeout->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            return js.call_api(API_setTimeout, _this, args, 0);
        };
        permanents.global_env->obj.insert({permanents.global_setTimeout->name, permanents.global_setTimeout});
        permanents.global_setInterval = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.global_setInterval->obj.insert({"length", _int_1});
        permanents.global_setInterval->name = "setInterval";
        permanents.global_setInterval->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            return js.call_api(API_setInterval, _this, args, 0);
        };
        permanents.global_env->obj.insert({permanents.global_setInterval->name, permanents.global_setInterval});
        permanents.global_clearTimeout = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.global_clearTimeout->obj.insert({"length", _int_1});
        permanents.global_clearTimeout->name = "clearTimeout";
        permanents.global_clearTimeout->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            return js.call_api(API_clearTimeout, _this, args, 0);
        };
        permanents.global_env->obj.insert({permanents.global_clearTimeout->name, permanents.global_clearTimeout});
        permanents.global_clearInterval = _new_function(nullptr, js_value::at_const | js_value::at_readonly);
        permanents.global_clearInterval->obj.insert({"length", _int_1});
        permanents.global_clearInterval->name = "clearInterval";
        permanents.global_clearInterval->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            return js.call_api(API_clearInterval, _this, args, 0);
        };
        permanents.global_env->obj.insert({permanents.global_clearInterval->name, permanents.global_clearInterval});
        // error
        permanents._proto_error = _new_object(js_value::at_const | js_value::at_readonly);
        permanents._proto_error->obj["name"] =
        permanents._proto_error->obj["__type__"] = _new_string("Error", js_value::at_const | js_value::at_refs);
        permanents._proto_error->obj["message"] = _empty_string;
        permanents.f_error = _new_function(permanents._proto_error, js_value::at_const | js_value::at_readonly);
        permanents.f_error->obj.insert({"length", _int_1});
        permanents.f_error->name = "Error";
        permanents.f_error->builtin = [](auto &func, auto &_this, auto &args, auto &js, auto attr) {
            auto err = js.new_error(0);
            if (!args.empty()) {
                err->obj.insert({"message", js.new_string(args.front().lock()->to_string(&js, 0))});
            }
            err->obj.insert({"stack", js.new_string(js.get_stacktrace())});
            func->stack.push_back(err);
            return 0;
        };
        permanents.global_env->obj.insert({permanents.f_error->name, permanents.f_error});
        // default func
        permanents.default_stack = std::make_shared<cjs_function>();
        permanents.default_stack->name = "<default>";
        permanents.default_stack->info = cjs_function_info::create_default();
    }
}
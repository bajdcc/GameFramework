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
#include <utility>
#include <regex>
#include "cjsruntime.h"
#include "cjsast.h"
#include "cjsgui.h"
#include "cjs.h"

#define BINARY_VERBOSE 1
#if BINARY_VERBOSE
#define BINARY_EXT ".json.bin"
#else
#define BINARY_EXT ".bson.bin"
#endif
#define DUMP_STEP 0
#define DUMP_ENV 1
#define DUMP_CLOSURE 0
#define DUMP_GC 0
#define SHOW_EXTRA 1
#define GC_PERIOD 128

#if defined(WIN32) || defined(WIN64)

#include <windows.h>

#define cjs_sleep(n) Sleep(n)
#else
#include <unistd.h>
#define cjs_sleep(n) sleep(n / 1000)
#endif

#include <base\nlohmann_json\json.h>

namespace nlohmann {
    template<>
    struct adl_serializer<std::vector<char*>> {
        static void to_json(json& j, const clib::cjs_consts& v) {
            v.to_json(j);
        }

        static void from_json(const json& j, clib::cjs_consts& v) {
            v.from_json(j);
        }
    };

    template<>
    struct adl_serializer<clib::js_sym_code_t> {
        static void to_json(json& j, const clib::js_sym_code_t& v) {
            v.to_json(j);
        }

        static void from_json(const json& j, clib::js_sym_code_t& v) {
            v.from_json(j);
        }
    };

    template<>
    struct adl_serializer<clib::js_sym_code_t::ref> {
        static void to_json(json& j, const clib::js_sym_code_t::ref& v) {
            v->to_json(j);
        }
    };

    template<>
    struct adl_serializer<clib::cjs_code_result::ref> {
        static void to_json(json& j, const clib::cjs_code_result::ref& v) {
            j = nlohmann::json{
                {"code", v->code},
                {"funcs", v->funcs},
            };
        }

        static void from_json(const json& j, clib::cjs_code_result::ref& v) {
            v = std::make_shared<clib::cjs_code_result>();
            v->code = std::make_shared<clib::js_sym_code_t>();
            j.at("code").get_to(*v->code);
            v->funcs.resize(j.at("funcs").size());
            const auto& f = j.at("funcs");
            for (size_t i = 0; i < f.size(); i++) {
                v->funcs[i] = std::make_shared<clib::js_sym_code_t>();
                f[i].get_to(*v->funcs[i]);
            }
        }
    };
}

namespace clib {

    std::unordered_map<std::string, cjs_code_result::ref> cjsruntime::caches;

    std::string js_trim(std::string s) {
        if (s.empty()) {
            return s;
        }
        s.erase(0, s.find_first_not_of(' '));
        s.erase(s.find_last_not_of(' ') + 1);
        return s;
    }

    double fix(const double& d) {
        if (std::isinf(d) || std::isnan(d))
            return 0.0;
        if (d == 0) {
            return d;
        }
        if (d >= 0)return floor(d);
        return ceil(d);
    }

    int cjsruntime::eval(cjs_code_result::ref code, const std::string& _path) {
        if (code->code->codes.empty()) {
            return exec(jsv_string::convert(_path), "throw new SyntaxError('Compile error')", true);
        }
        if (_path.empty() || _path[0] == '<') {
            paths.emplace_back(ROOT_DIR);
        }
        else {
            if (_path.back() == '/') {
                paths.push_back(_path);
            }
            else {
                auto f = _path.find_last_of('/');
                if (f != std::string::npos) {
                    paths.push_back(_path.substr(0, f + 1));
                }
                else {
                    if (paths.empty())
                        paths.emplace_back(ROOT_DIR);
                    else
                        paths.push_back(paths.back());
                }
            }
        }
        if (!current_stack) {
            stack.clear();
            auto top_stack = std::make_shared<cjs_function>(code->code, *this);
            stack.push_back(top_stack);
            current_stack = stack.back();
            current_stack->exec = _path;
            current_stack->envs = permanents.global_env;
            current_stack->_this = permanents.global_env;
            current_stack->trys.push_back(std::make_shared<sym_try_t>(sym_try_t{}));
        }
        else {
            auto exec_stack = new_stack(code->code);
            exec_stack->exec = _path;
            exec_stack->envs = new_object();
            exec_stack->_this = stack.front()->envs;
            stack.push_back(exec_stack);
            current_stack = stack.back();
        }
        return 3;
    }

    int cjsruntime::call_internal(bool top, size_t stack_size) {
        auto r = 0;
        auto gc_period = 0;
        auto has_throw = false;
        sym_try_t::ref trys;
        while (stack.size() > stack_size) {
            const auto& codes = current_stack->info->codes;
            const auto& pc = current_stack->pc;
            while (true) {
                if (top) {
                    if (permanents.cycle <= 0)
                        return 10;
                    permanents.cycle--;
                }
                if (pc >= (int)codes.size()) {
                    if (current_stack == permanents.default_stack)
                        r = 2;
                    else
                        r = 4;
                    break;
                }
                const auto& c = codes.at(pc);
                if (pc + 1 == (int)codes.size() && c.code == POP_TOP) {
                    r = 2;
                    break;
                }
#if DUMP_STEP
                dump_step(c);
#endif
                r = run(c);
                permanents.cycles++;
#if DUMP_STEP && SHOW_EXTRA
                dump_step2(c);
#endif
                if (top && gc_period++ >= GC_PERIOD) {
                    gc_period = 0;
                    gc();
                }
                if (r != 0)
                    break;
            }
            if (r == 1) {
                current_stack = stack.back();
                continue;
            }
            else if (r == 2) {
                if (!current_stack->ret_value.lock())
                    current_stack->ret_value =
                    current_stack->stack.empty() ? new_undefined() : pop();
            }
            else if (r == 3) {
                continue;
            }
            else if (r == 4) {
                current_stack->ret_value = new_undefined();
            }
            else if (r == 9) { // throw
                trys = get_try();
                if (trys && trys->stack_size > 0 && trys->stack_size <= stack.size() && trys->obj_size <= current_stack->stack.size()) {
                    if (trys->stack_size <= stack.size()) {
                        for (auto s = stack.size(); s > trys->stack_size; s--) {
                            if (!stack.back()->exec.empty())
                                paths.pop_back();
                            delete_stack(stack.back());
                            stack.pop_back();
                        }
                        current_stack = stack.back();
                    }
                    if (trys->obj_size <= current_stack->stack.size()) {
                        for (auto s = current_stack->stack.size(); s > trys->obj_size; s--) {
                            pop();
                        }
                    }
                    if (trys->jump_catch != 0) {
                        if (trys->obj.lock())
                            push(trys->obj);
                        else
                            push(new_undefined());
                        current_stack->pc = trys->jump_catch;
                        trys->jump_catch = 0;
                    }
                    else if (trys->jump_finally != 0) {
                        current_stack->pc = trys->jump_finally;
                    }
                    if (trys->jump_finally == 0) {
                        assert(!current_stack->trys.empty());
                        current_stack->trys.pop_back();
                        trys = nullptr;
                    }
                    continue;
                }
                has_throw = true;
                break;
            }
            if (!current_stack->exec.empty()) {
                CString s;
                s.Format(L"退出：%S", current_stack->exec.c_str());
                cjsgui::singleton().add_stat(s);
                paths.pop_back();
            }
            if (stack.size() > 1) {
                auto ret = stack.back()->ret_value;
                delete_stack(current_stack);
                stack.pop_back();
                current_stack = stack.back();
                push(ret);
            }
            else {
                if (top)
                    break;
                delete_stack(current_stack);
                stack.pop_back();
            }
        }
        if (has_throw) {
            if (top) {
                auto obj = trys ? trys->obj.lock() : nullptr;
                auto n = stack.size();
                for (size_t i = 1; i < n; i++) {
                    if (!stack.back()->exec.empty())
                        paths.pop_back();
                    delete_stack(stack.back());
                    stack.pop_back();
                }
                current_stack = stack.back();
                current_stack->stack.clear();
                if (obj) {
                    cjsgui::singleton().put_string("\033FFF0000\033");
                    cjsgui::singleton().put_string("Uncaught ");
                    cjsgui::singleton().put_string(obj->to_string(this, 1));
                    cjsgui::singleton().put_string("\033S4\033");
                    cjsgui::singleton().put_char('\n');
                }
            }
            else {
                return 9;
            }
        }
        return 0;
    }

    int cjsruntime::call_api(int type, js_value::weak_ref& _this,
        std::vector<js_value::weak_ref>& args, uint32_t attr) {
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
                time = (int)args[1].lock()->to_number(this);
                arg_n++;
            }
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
            api_clearTimeout(args.front().lock()->to_number(this));
            push(new_undefined());
        }
                              break;
        default:
            break;
        }
        return 0;
    }

    int cjsruntime::call_api(const jsv_function::ref& func, js_value::weak_ref& _this,
        std::vector<js_value::weak_ref>& args, uint32_t attr) {
        assert(_this.lock());
        auto stack_size = stack.size();
        bool fast = attr & jsv_function::at_fast;
        if (fast) {
            attr &= ~(uint32_t)jsv_function::at_fast;
        }
        if (func->builtin)
            return func->builtin(current_stack, _this, args, *this, attr);
        auto _new_stack = new_stack(func->code);
        stack.push_back(_new_stack);
        auto env = _new_stack->envs.lock();
        _new_stack->_this = _this;
        _new_stack->name = func->name;
        if (!func->code->arrow && func->code->simpleName.front() != '<')
            env->add(func->code->simpleName, func);
        auto arg = new_object();
        env->add("arguments", arg);
        size_t i = 0;
        size_t args_num = func->code->args_num;
        auto n = args.size();
        for (; i < n; i++) {
            std::stringstream ss;
            ss << i;
            arg->add(ss.str(), args.at(i));
            if (i < args_num)
                env->add(func->code->args.at(i), args.at(i));
        }
        for (; i < args_num; i++) {
            env->add(func->code->args.at(i), new_undefined());
        }
        if (func->code->rest) {
            auto rest = new_array();
            env->add(func->code->args.at(args_num), rest);
            auto j = 0;
            for (i = args_num; i < n; i++) {
                std::stringstream ss;
                ss << j++;
                rest->add(ss.str(), args.at(i));
            }
            rest->add("length", new_number(j));
        }
        arg->add("length", new_number(n));
        if (func->closure.lock())
            _new_stack->closure = func->closure;
        if (fast) {
            current_stack->pc++;
            return 1;
        }
        current_stack = _new_stack;
        return call_internal(false, stack_size);
    }

    js_value::ref cjsruntime::fast_api(const jsv_function::ref& func, js_value::weak_ref& _this,
        std::vector<js_value::weak_ref>& args, uint32_t attr, int* r) {
        auto size = stack.size();
        auto ret = call_api(func, _this, args, attr);
        while (ret == 10) {
            call_internal(false, size);
        }
        if (r)
            *r = ret;
        if (ret != 0)
            return new_undefined();
        return pop().lock();
    }

    double cjsruntime::api_setTimeout(int time, const jsv_function::ref& func, std::vector<js_value::weak_ref> args, uint32_t attr, bool once) {
        using namespace std::chrono;
        using namespace std::chrono_literals;
        auto t = duration_cast<milliseconds>(system_clock::now() - timeout.startup_time + time * 1ms).count();
        if (timeout.queues.find(t) == timeout.queues.end()) {
            timeout.queues[t] = std::list<std::shared_ptr<timeout_t>>();
        }
        auto s = std::make_shared<timeout_t>();
        s->once = once;
        s->time = time;
        s->id = timeout.global_id++;
        s->func = func;
        s->args = std::move(args);
        s->attr = attr;
        timeout.queues[t].push_back(s);
        timeout.ids.insert({ s->id, s });
        return (double)s->id;
    }

    void cjsruntime::api_clearTimeout(double id) {
        if (std::isinf(id) || std::isnan(id))
            return;
        auto i = (uint32_t)id;
        auto f = timeout.ids.find(i);
        if (f != timeout.ids.end()) {
            auto time = f->second->time;
            auto ff = timeout.queues.find(time);
            if (ff != timeout.queues.end()) {
                auto f2 = std::find_if(ff->second.begin(), ff->second.end(),
                    [i](const auto& x) { return x->id == i; });
                if (f2 != ff->second.end()) {
                    ff->second.erase(f2);
                }
                if (ff->second.empty()) {
                    timeout.queues.erase(ff);
                }
            }
            timeout.ids.erase(i);
        }
    }

    void cjsruntime::eval_timeout() {
        if (current_stack || !stack.empty())
            return;
        if (!timeout.ids.empty()) {
            using namespace std::chrono;
            using namespace std::chrono_literals;
            auto now = duration_cast<milliseconds>(std::chrono::system_clock::now() - timeout.startup_time).count();
            if ((*timeout.queues.begin()).first < now) {
                auto& v = (*timeout.queues.begin()).second;
                auto callback = v.front();
                stack.clear();
                stack.push_back(permanents.default_stack);
                current_stack = stack.back();
                current_stack->name = callback->func->name;
                current_stack->exec = callback->func->name;
                current_stack->envs = permanents.global_env;
                current_stack->_this = permanents.global_env;
                current_stack->trys.push_back(std::make_shared<sym_try_t>(sym_try_t{}));
                paths.emplace_back(ROOT_DIR);
                js_value::weak_ref env = stack.front()->envs;
                CString s;
                s.Format(L"Timeout：%S", current_stack->exec.c_str());
                cjsgui::singleton().add_stat(s);
                call_api(callback->func, env, callback->args, callback->attr | jsv_function::at_fast);
                current_stack = stack.back();
                v.pop_front();
                if (v.empty()) {
                    timeout.queues.erase(timeout.queues.begin());
                }
                if (!callback->once) {
                    auto t = duration_cast<milliseconds>(system_clock::now() - timeout.startup_time + callback->time * 1ms).count();
                    if (timeout.queues.find(t) == timeout.queues.end()) {
                        timeout.queues[t] = std::list<std::shared_ptr<timeout_t>>();
                    }
                    timeout.queues[t].push_back(callback);
                }
                else {
                    timeout.ids.erase(callback->id);
                }
            }
        }
    }

    sym_try_t::ref cjsruntime::get_try() const {
        for (auto i = stack.rbegin(); i != stack.rend(); i++) {
            for (auto j = (*i)->trys.rbegin(); j != (*i)->trys.rend(); j++) {
                return *j;
            }
        }
        assert(!"has no try");
        return nullptr;
    }

    void cjsruntime::set_readonly(bool flag) {
        readonly = flag;
    }

    int cjsruntime::run_internal(int cycle, int& cycles)
    {
        permanents.cycle = cycle;
        auto result = 0;
        if (current_stack) {
            result = call_internal(true, 0);
            cycles += permanents.cycles;
            if (result != 10) {
                delete_stack(current_stack);
                stack.pop_back();
                current_stack = nullptr;
            }
        }
        if (result != 10) {
            if (permanents.state == 0 && !current_stack) {
                permanents.state = 1;
            }
            eval_timeout();
        }
        return result;
    }

    int cjsruntime::run(const cjs_code& code) {
        switch (code.code) {
        case LOAD_EMPTY: {
            js_value::ref v;
            push(v);
        }
                       break;
        case LOAD_NULL:
            push(new_null());
            break;
        case LOAD_UNDEFINED:
            push(new_undefined());
            break;
        case LOAD_TRUE:
            push(new_boolean(true));
            break;
        case LOAD_FALSE:
            push(new_boolean(false));
            break;
        case LOAD_ZERO:
            push(code.op1 == 0 ? permanents._zero : permanents._minus_zero);
            break;
        case LOAD_THIS: {
            auto failed = true;
            for (auto i = stack.rbegin(); i != stack.rend(); i++) {
                if (!(*i)->info->arrow) {
                    push((*i)->_this);
                    failed = false;
                    break;
                }
            }
            if (failed)
                push(new_undefined());
        }
                      break;
        case POP_TOP:
            pop();
            break;
        case DUP_TOP:
            push(top());
            break;
        case NOP:
            break;
        case INSTANCE_OF: {
            auto op2 = pop().lock();
            auto op1 = pop().lock();
            if (op1->get_type() != r_object) {
                push(new_boolean(false));
                break;
            }
            if (op2->get_type() != r_function) {
                push(new_boolean(false));
                break;
            }
            const auto& ff = JS_OBJ(op2)->get("prototype");
            if (!ff) {
                push(new_boolean(false));
                break;
            }
            auto proto = op1->__proto__.lock();
            if (!proto) {
                push(new_boolean(false));
                break;
            }
            auto p = proto;
            auto failed = true;
            while (p) {
                assert(!p->is_primitive());
                if (p == ff) {
                    failed = false;
                    break;
                }
                p = p->__proto__.lock();
            }
            push(new_boolean(!failed));
        }
                        break;
        case UNARY_POSITIVE:
        case UNARY_NEGATIVE:
        case UNARY_NOT:
        case UNARY_INVERT:
        case UNARY_NEW:
        case UNARY_TYPEOF: {
            auto op1 = pop().lock();
            auto result = op1->unary_op(*this, code.code);
            assert(result);
            push(result);
        }
                         break;
        case UNARY_DELETE: {
            auto n = code.op1;
            if (n >= -1) {
                auto key = n >= 0 ? current_stack->info->names.at(code.op1) : pop().lock()->to_string(this, 0);
                auto obj = pop().lock();
                if (!obj->is_primitive()) {
                    auto f = JS_OBJ(obj)->get(key);
                    if (f) {
                        if (f->attr & js_value::at_readonly) {
                            push(new_boolean(false));
                            break;
                        }
                        JS_OBJ(obj)->remove(key);
                    }
                }
                push(new_boolean(true));
            }
            else if (n == -2) {
                auto op = code.op2;
                auto var = load_global(op);
                if (var) {
                    if (var->attr & js_value::at_readonly) {
                        push(new_boolean(false));
                        break;
                    }
                    else {
                        remove_global(op);
                    }
                }
                push(new_boolean(true));
            }
            else if (n == -8) {
                pop();
                push(new_boolean(true));
            }
            else {
                assert(!"invalid delete");
            }
        }
                         break;
        case BINARY_MATRIX_MULTIPLY:
            break;
        case INPLACE_MATRIX_MULTIPLY:
            break;
        case COMPARE_LESS:
        case COMPARE_LESS_EQUAL:
        case COMPARE_EQUAL:
        case COMPARE_NOT_EQUAL:
        case COMPARE_GREATER:
        case COMPARE_GREATER_EQUAL:
        case COMPARE_FEQUAL:
        case COMPARE_FNOT_EQUAL:
        case BINARY_POWER:
        case BINARY_MULTIPLY:
        case BINARY_MODULO:
        case BINARY_ADD:
        case BINARY_SUBTRACT:
        case BINARY_FLOOR_DIVIDE:
        case BINARY_TRUE_DIVIDE:
        case BINARY_LSHIFT:
        case BINARY_RSHIFT:
        case BINARY_URSHIFT:
        case BINARY_AND:
        case BINARY_XOR:
        case BINARY_OR: {
            auto op2 = pop().lock();
            auto op1 = pop().lock();
            auto r = 0;
            auto ret = binop(code.code, op1, op2, &r);
            if (r != 0)
                return r;
            push(ret);
        }
                      break;
        case LOAD_ATTR:
        case BINARY_SUBSCR: {
            auto key = code.code == LOAD_ATTR ?
                current_stack->info->names.at(code.op1) :
                pop().lock()->to_string(this, 0);
            auto obj = pop().lock();
            if (obj->get_type() == r_undefined) {
                std::stringstream ss;
                ss << "throw new TypeError('Cannot read property \\'" << jsv_string::convert(key) << "\\' of undefined')";
                auto _stack_size = stack.size();
                auto r = exec("<load_property::error>", ss.str(), true);
                if (r != 0)
                    return r;
                return call_internal(false, _stack_size);
            }
            if (!obj->is_primitive()) {
                auto value = JS_O(obj)->gets(key);
                if (value) {
                    push(value);
                    break;
                }
            }
            else if (obj->get_type() == r_string) {
                push(JS_S(obj)->get(this, key)));
                break;
            }
            push(permanents._undefined);
        }
                          break;
        case BINARY_INC:
        case BINARY_DEC: {
            auto op1 = pop().lock();
            auto r = 0;
            auto ret = binop(
                code.code == BINARY_INC ? BINARY_ADD : BINARY_SUBTRACT,
                op1,
                permanents._one, &r);
            if (r != 0)
                return r;
            assert(ret);
            push(ret);
        }
                       break;
        case STORE_SUBSCR: {
            auto key = pop().lock()->to_string(this, 0);
            auto obj = pop().lock();
            auto value = top();
            auto o = JS_OBJ(obj);
            if (!obj->is_primitive()) {
                auto f = o->get(key);
                if (f && (readonly && (f->attr & js_value::at_readonly))) {
                    break;
                }
                o->add(key, std::move(value));
                break;
            }
        }
                         break;
        case GET_ITER: {
            auto obj = pop().lock();
            if (!obj->is_primitive()) {
                auto o = JS_OBJ(obj);
                auto arr = new_array();
                if (obj->__proto__.lock() != permanents._proto_array) {
                    auto ar = o->get_keys();
                    for (size_t i = 0; i < ar.size(); i++) {
                        std::stringstream ss;
                        ss << i;
                        arr->add(ss.str(), new_string(ar[i]));
                    }
                    arr->add("length", new_number(ar.size()));
                }
                else {
                    auto len = o->get("length");
                    if (len) {
                        auto i = 0, j = 0;
                        if (len->get_type() == r_number) {
                            auto l = JS_NUM(len);
                            if (!std::isinf(l) && !std::isnan(l)) {
                                auto L = (int)l;
                                std::stringstream ss;
                                while (i < L) {
                                    ss.str("");
                                    ss << j++;
                                    auto ff = o->get(ss.str());
                                    if (ff) {
                                        arr->add(ss.str(), new_string(ss.str()));
                                    }
                                    i++;
                                }
                            }
                        }
                        arr->add("length", new_number(j));
                    }
                    else {
                        arr->add("length", new_number(0.0));
                    }
                }
                push(arr);
                push(new_number(0.0));
            }
            else if (obj->get_type() == r_string) {
                auto arr = new_array();
                auto s = obj->to_string(this, 0);
                std::stringstream ss;
                for (size_t i = 0; i < s.size(); i++) {
                    ss.str("");
                    ss << i;
                    arr->add(ss.str(), new_number(i));
                }
                arr->add("length", new_number(s.size()));
                push(arr);
                push(new_number(0.0));
            }
            else {
                push(new_array());
                push(new_number(0.0));
            }
        }
                     break;
        case RETURN_VALUE:
            return 2;
        case STORE_NAME: {
            auto obj = top();
            auto id = code.op1;
            auto name = current_stack->info->names.at(id);
            current_stack->store_name(name, obj);
        }
                       break;
        case DELETE_NAME:
            break;
        case UNPACK_SEQUENCE: {
            auto obj = pop().lock();
            if (!obj->is_primitive()) {
                const auto o = JS_OBJ(obj);
                if (obj->__proto__.lock() == permanents._proto_array) {
                    auto len = o->get("length");
                    if (len) {
                        if (len->get_type() == r_number) {
                            auto l = JS_NUM(len);
                            if (!std::isinf(l) && !std::isnan(l)) {
                                for (auto i = 0; i < l; i++) {
                                    std::stringstream ss;
                                    ss << i;
                                    auto ff = o->get(ss.str());
                                    if (ff) {
                                        push(ff);
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
            push(obj); // failed
        }
                            break;
        case FOR_ITER: {
            auto jmp = code.op1;
            auto idx = pop().lock();
            assert(idx->get_type() == r_number);
            auto i = JS_NUM(idx);
            assert(!std::isinf(i) && !std::isnan(i));
            auto obj = top().lock();
            assert(!obj->is_primitive() && obj->__proto__.lock() == permanents._proto_array);
            const auto o = JS_OBJ(obj);
            auto len = o->get("length");
            if (len) {
                if (len->get_type() == r_number) {
                    auto l = JS_NUM(len);
                    if (!std::isinf(l) && !std::isnan(l)) {
                        if (i < l) {
                            std::stringstream ss;
                            auto failed = true;
                            while (i < l) {
                                ss.str("");
                                ss << i;
                                auto ff = o->get(ss.str());
                                if (ff) {
                                    push(new_number(i + 1));
                                    push(ff);
                                    failed = false;
                                    break;
                                }
                                i++;
                            }
                            if (failed) {
                                current_stack->pc += jmp;
                                pop();
                                return 0;
                            }
                            break;
                        }
                        current_stack->pc += jmp;
                        pop();
                        return 0;
                    }
                }
            }
            assert(!"invalid iter");
        }
                     break;
        case UNPACK_EX: {
            auto obj = pop().lock();
            if (!obj->is_primitive()) {
                const auto o = JS_OBJ(obj);
                if (obj->__proto__.lock() == permanents._proto_object) {
                    for (const auto& s : o->get_keys()) {
                        push(new_string(s));
                        push(o->get(s));
                    }
                }
            }
        }
                      break;
        case STORE_ATTR: {
            auto n = code.op1;
            auto key = current_stack->info->names.at(n);
            auto obj = pop().lock();
            auto value = top();
            if (!obj->is_primitive()) {
                auto o = JS_OBJ(obj);
                auto f = o->get(key);
                if (f && (readonly && (f->attr & js_value::at_readonly))) {
                    break;
                }
                o->add(key, std::move(value));
                break;
            }
        }
                       break;
        case STORE_GLOBAL: {
            auto obj = top();
            auto id = code.op1;
            auto name = current_stack->info->globals.at(id);
            stack.front()->store_name(name, obj);
        }
                         break;
        case LOAD_CONST: {
            auto op = code.op1;
            auto var = load_const(op);
            push(var);
        }
                       break;
        case LOAD_NAME: {
            auto op = code.op1;
            auto var = load_name(op);
            push(var);
        }
                      break;
        case BUILD_LIST: {
            auto n = code.op1;
            if (n == -1) {
                assert(!current_stack->rests.empty());
                n = (int)current_stack->stack.size() - current_stack->rests.back();
                current_stack->rests.pop_back();
            }
            assert(current_stack->stack.size() >= (size_t)n);
            auto obj = new_array();
            std::stringstream ss;
            for (auto i = n - 1; i >= 0; i--) {
                auto v = pop().lock();
                if (v) {
                    ss.str("");
                    ss << i;
                    obj->add(ss.str(), v);
                }
            }
            obj->add("length", new_number(n));
            push(obj);
        }
                       break;
        case BUILD_MAP: {
            auto n = code.op1;
            if (n == -1) {
                assert(!current_stack->rests.empty());
                n = ((int)current_stack->stack.size() - current_stack->rests.back()) / 2;
                current_stack->rests.pop_back();
            }
            assert(current_stack->stack.size() >= ((size_t)n) * 2);
            auto obj = new_object();
            for (auto i = 0; i < n; i++) {
                auto v = pop();
                auto k = pop().lock();
                if (k->get_type() == r_string) {
                    obj->add(JS_STR(k), v);
                }
                else if (k->get_type() == r_number) {
                    std::stringstream ss;
                    ss << JS_NUM(k);
                    obj->add(ss.str(), v);
                }
                else {
                    continue;
                }
            }
            push(obj);
        }
                      break;
        case JUMP_FORWARD: {
            auto jmp = code.op1;
            current_stack->pc += jmp;
            return 0;
        }
        case JUMP_IF_FALSE_OR_POP: {
            auto jmp = code.op1;
            const auto& t = top();
            if (!t.lock()->to_bool()) {
                current_stack->pc = jmp;
                return 0;
            }
            else {
                pop();
            }
        }
                                 break;
        case JUMP_IF_TRUE_OR_POP: {
            auto jmp = code.op1;
            const auto& t = top();
            if (t.lock()->to_bool()) {
                current_stack->pc = jmp;
                return 0;
            }
            else {
                pop();
            }
        }
                                break;
        case JUMP_ABSOLUTE: {
            auto jmp = code.op1;
            current_stack->pc = jmp;
            return 0;
        }
        case POP_JUMP_IF_FALSE: {
            auto jmp = code.op1;
            auto t = pop();
            if (!t.lock()->to_bool()) {
                current_stack->pc = jmp;
                return 0;
            }
        }
                              break;
        case POP_JUMP_IF_TRUE: {
            auto jmp = code.op1;
            auto t = pop();
            if (t.lock()->to_bool()) {
                current_stack->pc = jmp;
                return 0;
            }
        }
                             break;
        case LOAD_GLOBAL: {
            auto op = code.op1;
            auto var = load_global(op);
            if (var) {
                push(var);
            }
            else {
                auto name = current_stack->info->globals.at(op);
                std::stringstream ss;
                ss << "throw new ReferenceError('" << jsv_string::convert(name) << " is not defined')";
                auto _stack_size = stack.size();
                auto r = exec("<load_global::error>", ss.str(), true);
                if (r != 0)
                    return r;
                return call_internal(false, _stack_size);
            }
        }
                        break;
        case SETUP_FINALLY: {
            auto op1 = code.op1 == 0 ? 0 : (current_stack->pc + code.op1);
            auto op2 = code.op2 == 0 ? 0 : (current_stack->pc + code.op2);
            current_stack->trys.push_back(std::make_shared<sym_try_t>(sym_try_t{ stack.size(), current_stack->stack.size(), op1, op2, js_value::weak_ref() }));
        }
                          break;
        case THROW: {
            auto trys = get_try();
            assert(trys);
            trys->obj = pop().lock();
        }
                  return 9;
        case POP_FINALLY:
            return 9;
        case EXIT_FINALLY:
            assert(!current_stack->trys.empty());
            current_stack->trys.pop_back();
            break;
        case LOAD_FAST: {
            auto op = code.op1;
            auto var = load_fast(op);
            push(var);
        }
                      break;
        case STORE_FAST: {
            auto obj = top();
            auto id = code.op1;
            auto name = current_stack->info->names.at(id);
            current_stack->store_fast(name, obj);
        }
                       break;
        case CALL_FUNCTION: {
            auto n = code.op1;
            if (n == -1) {
                assert(!current_stack->rests.empty());
                n = (int)current_stack->stack.size() - current_stack->rests.back();
                current_stack->rests.pop_back();
            }
            assert((int)current_stack->stack.size() > n);
            std::vector<js_value::weak_ref> args;
            args.resize(n);
            auto m = n;
            while (m-- > 0) {
                args[m] = pop();
            }
            auto f = pop();
            if (f.lock()->get_type() != r_function) {
                auto x = current_stack->info->debugs.at(code.op2);
                std::stringstream ss;
                ss << "throw new TypeError('" << jsv_string::convert(x) << " is not a function')";
                auto _stack_size = stack.size();
                auto r = exec("<call_function::error>", ss.str(), true);
                if (r != 0)
                    return r;
                return call_internal(false, _stack_size);
            }
            auto func = JS_FUN(f.lock());
            js_value::weak_ref _this = JS_V(stack.front()->envs.lock());
            auto r = call_api(func, _this, args, jsv_function::at_fast);
            if (r != 0)
                return r;
        }
                          break;
        case MAKE_FUNCTION: {
            auto op = (uint32_t)code.op1;
            if (op & 8U) {
                auto name = pop();
                auto f = pop();
                auto closure = pop();
                if (f.lock()->get_type() != r_function) {
                    push(new_undefined());
                    break;
                }
                assert(name.lock()->get_type() == r_string);
                assert(!closure.lock()->is_primitive());
                auto parent = JS_FUN(f.lock());
                auto func = new_function();
                func->code = parent->code;
                func->name = JS_STR(name.lock());
                if (closure.lock()) {
                    auto c = new_object();
                    c->copy_from(JS_OBJ(closure.lock()));
                    func->closure = c;
                }
                auto len = new_number(func->code->args_num);
                func->add("length", len);
                push(func);
            }
            else {
                auto name = pop();
                auto f = pop();
                if (f.lock()->get_type() != r_function) {
                    push(new_undefined());
                    break;
                }
                assert(name.lock()->get_type() == r_string);
                auto parent = JS_FUN(f.lock());
                auto func = new_function();
                func->code = parent->code;
                func->name = JS_STR(name.lock());
                auto len = new_number(func->code->args_num);
                func->add("length", len);
                push(func);
            }
        }
                          break;
        case LOAD_CLOSURE: {
            auto op = code.op1;
            auto var = current_stack->info->names.at(op);
            push(new_string(var));
            push(load_closure(var));
        }
                         break;
        case LOAD_DEREF: {
            auto op = code.op1;
            auto var = current_stack->info->derefs.at(op);
            push(load_deref(var));
        }
                       break;
        case STORE_DEREF: {
            auto obj = top();
            auto id = code.op1;
            auto name = current_stack->info->derefs.at(id);
            current_stack->store_deref(name, obj);
        }
                        break;
        case REST_ARGUMENT:
            current_stack->rests.push_back(current_stack->stack.size());
            break;
        case CALL_FUNCTION_EX: {
            auto n = code.op1;
            if (n == -1) {
                assert(!current_stack->rests.empty());
                n = (int)current_stack->stack.size() - current_stack->rests.back();
                current_stack->rests.pop_back();
            }
            assert((int)current_stack->stack.size() > n);
            std::vector<js_value::weak_ref> args(n);
            auto m = n;
            while (m-- > 0) {
                args[m] = pop();
            }
            auto f = pop();
            if (f.lock()->get_type() != r_function) {
                push(new_undefined());
                break;
            }
            auto func = JS_FUN(f.lock());
            auto _this = new_object();
            auto prototype = func->get("prototype");
            if (!prototype || prototype->is_primitive())
                _this->__proto__ = permanents._proto_object;
            else
                _this->__proto__ = prototype;
            js_value::weak_ref t = _this;
            auto r = 0;
            auto ret = fast_api(func, t, args, jsv_function::at_new_function, &r);
            if (ret->is_primitive())
                push(t);
            else
                push(ret);
            if (r != 0)
                return r;
        }
                             break;
        case LOAD_METHOD: {
            auto n = code.op1;
            auto key = current_stack->info->names.at(n);
            auto obj = top().lock();
            if (!obj->is_primitive()) {
                const auto o = JS_OBJ(obj);
                auto f = o->gets(key);
                if (f) {
                    if (f->get_type() == r_function)
                        push(f);
                    else
                        push(new_undefined()); // type error
                    break;
                }
            }
            auto proto = obj->__proto__.lock();
            if (!proto) {
                push(new_undefined()); // type error
                break;
            }
            auto p = proto;
            auto failed = true;
            while (p) {
                if (p->get_type() != r_object) {
                    push(new_undefined()); // type error
                    break;
                }
                const auto f = JS_OBJ(p)->get(key);
                if (f) {
                    if (f->get_type() == r_function) {
                        failed = false;
                        push(f);
                    }
                    break;
                }
                p = p->__proto__.lock();
            }
            if (failed)
                push(new_undefined()); // type error
        }
                        break;
        case CALL_METHOD: {
            auto n = code.op1;
            if (n == -1) {
                assert(!current_stack->rests.empty());
                n = (int)current_stack->stack.size() - current_stack->rests.back();
                current_stack->rests.pop_back();
            }
            assert((int)current_stack->stack.size() > n);
            std::vector<js_value::weak_ref> args(n);
            auto m = n;
            while (m-- > 0) {
                args[m] = pop();
            }
            auto f = pop();
            auto _this = pop().lock();
            if (f.lock()->get_type() != r_function) {
                auto x = current_stack->info->debugs.at(code.op2);
                std::stringstream ss;
                ss << "throw new TypeError('" << jsv_string::convert(x) << " is not a function')";
                auto _stack_size = stack.size();
                auto r = exec("<call_method::error>", ss.str(), true);
                if (r != 0)
                    return r;
                return call_internal(false, _stack_size);
            }
            auto func = JS_FUN(f.lock());
            js_value::weak_ref t = _this;
            auto r = call_api(func, t, args, jsv_function::at_fast);
            if (r != 0)
                return r;
        }
                        break;
        default:
            assert(!"invalid opcode");
            return 1;
        }

        current_stack->pc++;
        return 0;
    }

    js_value::ref cjsruntime::load_const(int op) {
        auto v = current_stack->info->consts.at(op);
        if (v)
            return register_value(v);
        assert(!"invalid runtime type");
        return permanents._undefined;
    }

    js_value::ref cjsruntime::load_fast(int op) {
        auto name = current_stack->info->names.at(op);
        auto o = current_stack->envs.lock();
        auto L = o->get(name);
        if (L) {
            return L;
        }
        return permanents._undefined;
    }

    js_value::ref cjsruntime::load_name(int op) {
        auto name = current_stack->info->names.at(op);
        for (auto i = stack.rbegin(); i != stack.rend(); i++) {
            auto o = (*i)->envs.lock();
            auto L = o->get(name);
            if (L) {
                return L;
            }
        }
        assert(!"cannot find value by name");
        return permanents._undefined;
    }

    js_value::ref cjsruntime::load_global(int op) {
        auto g = current_stack->info->globals.at(op);
        auto o = stack.front()->envs.lock();
        auto G = o->get(g);
        if (G) {
            return G;
        }
        return nullptr;
    }

    bool cjsruntime::remove_global(int op) {
        auto g = current_stack->info->globals.at(op);
        auto o = stack.front()->envs.lock();
        auto G = o->get(g);
        if (G) {
            o->remove(g);
            return true;
        }
        return false;
    }

    js_value::ref cjsruntime::load_closure(const std::string& name) {
        for (auto i = stack.rbegin(); i != stack.rend(); i++) {
            if ((*i)->closure.lock()) {
                auto L = (*i)->closure.lock()->get(name);
                if (L) {
                    return L;
                }
            }
            auto L2 = (*i)->envs.lock()->get(name);
            if (L2) {
                return (*i)->envs.lock();
            }
        }
        assert(!"cannot load closure value by name");
        return permanents._undefined;
    }

    js_value::ref cjsruntime::load_deref(const std::string& name) {
        if (current_stack->closure.lock()) {
            auto ctx = current_stack->closure.lock()->get(name);
            if (ctx) {
                if (!ctx->is_primitive()) {
                    const auto obj = JS_OBJ(ctx);
                    auto f2 = obj->get(name);
                    if (f2) {
                        return f2;
                    }
                }
            }
        }
        assert(!"cannot load deref by name");
        return permanents._undefined;
    }

    void cjsruntime::push(js_value::weak_ref value) {
        current_stack->stack.push_back(std::move(value));
    }

    const js_value::weak_ref& cjsruntime::top() const {
        assert(!current_stack->stack.empty());
        return current_stack->stack.back();
    }

    js_value::weak_ref cjsruntime::pop() {
        assert(!current_stack->stack.empty());
        auto p = current_stack->stack.back();
        current_stack->stack.pop_back();
        return p;
    }

    js_value::ref cjsruntime::register_value(const js_value::ref& value) {
        if (value)
            objs.push_back(value);
        return value;
    }

    void cjsruntime::dump_step(const cjs_code& c) const {
        fprintf(stdout, "R [%04d] %s\n", current_stack->pc, c.desc.c_str());
    }

    void cjsruntime::dump_step2(const cjs_code& c) const {
        if (!stack.empty() && !stack.front()->stack.empty())
            std::cout << std::setfill('=') << std::setw(60) << "" << std::endl;
        for (auto s = stack.rbegin(); s != stack.rend(); s++) {
            fprintf(stdout, "**** Stack [%p] \"%.100s\" '%.100s'\n",
                s->get(), (*s)->name.c_str(),
                (*s)->info ? (*s)->info->text.c_str() : "[builtin]");
            const auto& st = (*s)->stack;
            auto sti = (int)st.size();
            fprintf(stdout, "this | [%p] \n", (*s)->_this.lock().get());
            for (auto s2 = st.rbegin(); s2 != st.rend(); s2++) {
                fprintf(stdout, "%4d | [%p] ", sti--, s2->lock().get());
                if (s2->lock() == permanents.global_env)
                    fprintf(stdout, "<global env>\n");
                else if (s2->lock()->attr & js_value::at_readonly)
                    fprintf(stdout, "<builtin>\n");
                else
                    print(s2->lock(), 0, std::cout);
            }
#if DUMP_ENV
            const auto& _env = (*s)->envs.lock();
            const auto& env = _env->get_keys();
            if (!env.empty()) {
                std::cout << std::setfill('-') << std::setw(60) << "" << std::endl;
                for (const auto& e : env) {
                    auto o = _env->get(e);
                    fprintf(stdout, " Env | [%p] \"%.100s\" '%.100s' ",
                        o.get(), e.c_str(),
                        o->to_string(nullptr, 0).c_str());
                    if (o == permanents.global_env)
                        fprintf(stdout, "<global env>\n");
                    else if (o->attr & js_value::at_readonly)
                        fprintf(stdout, "<builtin>\n");
                    else
                        print(o, 0, std::cout);
                }
            }
#endif
#if DUMP_CLOSURE
            if ((*s)->closure.lock()) {
                const auto& _cl = (*s)->closure.lock();
                const auto& cl = _cl->get_keys();
                std::cout << std::setfill('-') << std::setw(60) << "" << std::endl;
                for (const auto& e : cl) {
                    auto o = _cl->get(e);
                    fprintf(stdout, " Clo | [%p] \"%.100s\" '%.100s' ",
                        o.get(), e.c_str(),
                        o->to_string(nullptr, 1).c_str());
                    print(o, 0, std::cout);
                }
            }
            std::cout << std::setfill('-') << std::setw(60) << "" << std::endl;
#endif
        }
    }

    void cjsruntime::dump_step3() const {
        if (objs.empty())
            return;
        for (const auto& s : objs) {
            fprintf(stdout, " GC  | [%p] Mark: %d, ", s.get(), s->marked);
            print(s, 0, std::cout);
        }
    }

    jsv_number::ref cjsruntime::new_number(double n) {
        if (std::isnan(n)) {
            return permanents.__nan;
        }
        if (std::isinf(n)) {
            return std::signbit(n) == 0 ? permanents._inf : permanents._minus_inf;
        }
        if (n == 0.0) {
            return std::signbit(n) == 0 ? permanents._zero : permanents._minus_zero;
        }
        if (n == 1.0) {
            return permanents._one;
        }
        if (n == -1.0) {
            return permanents._minus_one;
        }
        if (reuse.reuse_numbers.empty()) {
            auto s = _new_number(n);
            register_value(s);
            return s;
        }
        auto r = reuse.reuse_numbers.back();
        register_value(r);
        r->number = n;
        r->__proto__ = permanents._proto_number;
        reuse.reuse_numbers.pop_back();
        return std::move(r);
    }

    jsv_string::ref cjsruntime::new_string(const std::string& s) {
        if (s.empty())
            return permanents._empty;
        if (reuse.reuse_strings.empty()) {
            auto t = _new_string(s);
            register_value(t);
            return t;
        }
        auto r = reuse.reuse_strings.back();
        register_value(r);
        r->str = s;
        r->__proto__ = permanents._proto_string;
        reuse.reuse_strings.pop_back();
        return std::move(r);
    }

    jsv_boolean::ref cjsruntime::new_boolean(bool b) {
        if (b) return permanents._true;
        return permanents._false;
    }

    jsv_object::ref cjsruntime::new_object() {
        if (reuse.reuse_objects.empty()) {
            auto s = _new_object();
            register_value(s);
            return s;
        }
        auto r = reuse.reuse_objects.back();
        register_value(r);
        r->__proto__ = permanents._proto_object;
        reuse.reuse_objects.pop_back();
        return std::move(r);
    }

    jsv_object::ref cjsruntime::new_object_box(const js_value::ref& v) {
        auto obj = new_object();
        switch (v->get_type()) {
        case r_number:
            obj->__proto__ = permanents._proto_number;
            break;
        case r_string:
            obj->__proto__ = permanents._proto_string;
            break;
        case r_boolean:
            obj->__proto__ = permanents._proto_boolean;
            break;
        case r_function:
            obj->__proto__ = permanents._proto_function;
            break;
        default:
            break;
        }
        obj->special.insert({ "PrimitiveValue", v });
        return obj;
    }

    jsv_function::ref cjsruntime::new_function() {
        if (reuse.reuse_functions.empty()) {
            auto s = _new_function(nullptr);
            register_value(s);
            return s;
        }
        auto r = reuse.reuse_functions.back();
        register_value(r);
        r->__proto__ = permanents._proto_function;
        reuse.reuse_functions.pop_back();
        return std::move(r);
    }

    jsv_null::ref cjsruntime::new_null() {
        return permanents._null;
    }

    jsv_undefined::ref cjsruntime::new_undefined() {
        return permanents._undefined;
    }

    cjs_function::ref cjsruntime::new_func(const cjs_function_info::ref& code) {
        auto f = new_stack(code);
        stack.push_back(f);
        return f;
    }

    jsv_object::ref cjsruntime::new_array() {
        auto arr = new_object();
        arr->__proto__ = permanents._proto_array;
        arr->add("length", new_number(0.0));
        return arr;
    }

    jsv_regexp::ref cjsruntime::new_regexp()
    {
        if (reuse.reuse_regexes.empty()) {
            auto s = _new_regex();
            register_value(s);
            return s;
        }
        auto r = reuse.reuse_regexes.back();
        register_value(r);
        r->__proto__ = permanents._proto_regexp;
        reuse.reuse_regexes.pop_back();
        return std::move(r);
    }

    jsv_object::ref cjsruntime::new_error(int type) {
        auto err = new_object();
        err->__proto__ = permanents._proto_error;
        return err;
    }

    int cjsruntime::exec(const std::string& filename, const std::string& input, bool error) {
        if (input.empty())
            return 0;
        auto last = std::chrono::system_clock::now();
        CString stat;
        auto p = std::make_unique<cjsparser>();
        std::string error_string;
        std::string code_name;
        cjs_code_result::ref code;
        if (!error) {
            auto f = caches.find(filename);
            if (f != caches.end()) {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last);
                stat.Format(L"快速加载：%S，耗时：%lldms", filename.c_str(), duration.count());
                cjsgui::singleton().add_stat(stat);
                caches.insert({ filename, code });
                return eval(f->second, filename);
            }
        }
        auto needCache = false;
        if (!filename.empty() && (filename[0] == '<' || filename[0] == '(')) {
            code_name = filename;
            if (filename == "<entry>" || filename == "<library>")
                needCache = true;
        }
        else {
            if (!error) {
                code = load_cache(filename);
                needCache = true;
                if (code) {
                    caches.insert({ filename, code });
                    return eval(std::move(code), filename);
                }
            }
            code_name = "(" + filename + ":1:1) <entry>";
        }
        try {
            if (p->parse(input, error_string, this) == nullptr) {
                std::stringstream ss;
                ss << "throw new SyntaxError('" << jsv_string::convert(error_string) << "')";
                return exec(code_name, ss.str(), true);
            }
#if LOG_AST
            cjsast::print(p->root(), 0, input, std::cout);
#endif
            auto g = std::make_unique<cjsgen>();
            g->gen_code(p->root(), &input, filename);
            p = nullptr;
#if LOG_FILE
            std::ofstream ofs(LOG_FILENAME);
            if (ofs)
                cjsast::print(p.root(), 0, input, ofs);
#endif
            code = g->get_code();
            assert(code);
            code->code->debugName = code_name;
            if (needCache) {
                save_cache(filename, code);
                caches.insert({ filename, code });
            }
            g = nullptr;
        }
        catch (const clib::cjs_exception & e) {
            stat.Format(L"编译：%S，失败", filename.c_str());
            cjsgui::singleton().add_stat(stat);
            std::stringstream ss;
            ss << "throw new SyntaxError('" << jsv_string::convert(e.message()) << "')";
            return exec(code_name, ss.str(), true);
        }
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last);
        stat.Format(L"编译：%S，耗时：%lldms", filename.c_str(), duration.count());
        cjsgui::singleton().add_stat(stat);
        return eval(std::move(code), filename);
    }

    std::string cjsruntime::get_stacktrace() const {
        std::stringstream ss, sx;
        auto j = stack.size();
        for (auto i = stack.rbegin(); i != stack.rend(); i++) {
            ss << j-- << ": ";
            if ((*i)->pc < (int)(*i)->info->codes.size()) {
                sx.str("");
                const auto& c = (*i)->info->codes[(*i)->pc];
                sx << "($1:" << c.line << ":" << c.column << ")$2";
                ss << std::regex_replace((*i)->name, tools.stacktrace, sx.str());
            }
            else {
                ss << (*i)->name;
            }
            if (!(*i)->info)
                ss << " (builtin)";
            ss << std::endl;
        }
        auto s = ss.str();
        if (!s.empty())
            s.pop_back();
        return s;
    }

    bool cjsruntime::set_builtin(const std::shared_ptr<jsv_object>& obj) {
        if (readonly)
            return false;
        obj->attr |= js_value::at_readonly;
        return true;
    }

    static bool check_file(const std::string& filename, std::string& content) {
        std::ifstream file(filename);
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            content = buffer.str();
            return true;
        }
        return false;
    }

    bool cjsruntime::get_file(std::string& filename, std::string& content) const {
        if (filename.empty())
            return false;
        if (check_file(ROOT_DIR + paths.back() + filename, content)) {
            if (paths.back() != ROOT_DIR)
                filename = paths.back() + filename;
            return true;
        }
        if (check_file(ROOT_DIR + filename, content)) {
            if (filename.substr(0, 2) != ROOT_DIR)
                filename = filename;
            return true;
        }
        return false;
    }

    bool cjsruntime::to_number(const js_value::ref& obj, double& d) {
        switch (obj->get_type()) {
        case r_number:
            d = JS_NUM(obj);
            return true;
        case r_string: {
            switch (JS_STR2NUM(obj, d)) {
            case 0:
            case 1:
                d = 0.0;
                return true;
            case 2:
                return true;
            case 3:
                return false;
            default:
                break;
            }
        }
                     break;
        case r_boolean:
            d = JS_BOOL(obj) ? 1.0 : 0.0;
            break;
        case r_function:
        case r_object:
        case r_null:
        case r_undefined:
            break;
        default:
            assert(!"invalid number");
            break;
        }
        return false;
    }

    std::vector<js_value::weak_ref> cjsruntime::to_array(const js_value::ref& f) {
        std::vector<std::weak_ptr<js_value>> ret;
        if (f->get_type() != r_object) {
            return ret;
        }
        const auto obj = JS_OBJ(f);
        auto len = obj->get("length");
        if (!len) {
            return ret;
        }
        auto length = 0;
        double d = 0.0;
        if (to_number(len, d)) {
            if (!(std::isinf(d) && std::isnan(d)))
                length = (int)std::floor(d);
        }
        for (auto i = 0; i < length; i++) {
            std::stringstream ss;
            ss << i;
            auto ff = obj->get(ss.str());
            if (ff) {
                ret.push_back(ff);
            }
        }
        return ret;
    }

    int cjsruntime::get_state() const
    {
        return permanents.state;
    }

    void cjsruntime::set_state(int n)
    {
        if (n == 2) {
            set_readonly(true);
        }
        permanents.state = n;
    }

    void cjsruntime::clear_cache()
    {
        caches.clear();
    }

    backtrace_direction cjsruntime::check(js_pda_edge_t edge, js_ast_node* node) {
        return b_next;
    }

    void cjsruntime::error_handler(int, const std::vector<js_pda_trans>&, int&) {
    }

    static bool ConvertFileTimeToLocalTime(const FILETIME* lpFileTime, SYSTEMTIME* lpSystemTime)
    {
        if (!lpFileTime || !lpSystemTime) {
            return false;
        }
        FILETIME ftLocal;
        FileTimeToLocalFileTime(lpFileTime, &ftLocal);
        FileTimeToSystemTime(&ftLocal, lpSystemTime);
        return true;
    }

    static bool ConvertLocalTimeToFileTime(const SYSTEMTIME* lpSystemTime, FILETIME* lpFileTime)
    {
        if (!lpSystemTime || !lpFileTime) {
            return false;
        }

        FILETIME ftLocal;
        SystemTimeToFileTime(lpSystemTime, &ftLocal);
        LocalFileTimeToFileTime(&ftLocal, lpFileTime);
        return true;
    }

    static bool get_fs_time(const std::string& name, std::vector<std::string>& time)
    {
        std::smatch res;

        HANDLE hFile;
        // Create, Access, Write
        FILETIME ft[3];
        SYSTEMTIME st[3];

        auto pp = CString(CStringA(name.c_str()));

        hFile = CreateFile(pp, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (INVALID_HANDLE_VALUE == hFile) {
            return false;
        }
        GetFileTime(hFile, &ft[0], &ft[1], &ft[2]);
        for (auto i = 0; i < 3; i++) {
            ConvertFileTimeToLocalTime(&ft[i], &st[i]);
            CStringA buf;
            buf.Format("%04d-%02d-%02d %02d:%02d:%02d.%03d",
                st[i].wYear,
                st[i].wMonth,
                st[i].wDay,
                st[i].wHour,
                st[i].wMinute,
                st[i].wSecond,
                st[i].wMilliseconds);
            time.push_back(buf.GetBuffer(0));
        }

        CloseHandle(hFile);
        return true;
    }

    cjs_code_result::ref cjsruntime::load_cache(const std::string& filename)
    {
        CString stat;
        auto start = std::chrono::system_clock::now();
        using nlohmann::json;
        auto file = ROOT_DIR + filename;
        auto json_name = file + BINARY_EXT;
        std::vector<std::string> t1, t2;
        if (get_fs_time(file, t1) && get_fs_time(json_name, t2) && t1[2] > t2[2]) {
            return nullptr;
        }
#if BINARY_VERBOSE
        std::ifstream ifs(json_name);
        if (!ifs)
            return nullptr;
        json j;
        ifs >> j;
        cjs_code_result::ref result = j;
#else
        std::ifstream ifs(json_name, std::ios::binary);
        if (!ifs)
            return nullptr;
        auto p = ifs.rdbuf();
        auto size = p->pubseekoff(0, std::ios::end, std::ios::in);
        p->pubseekpos(0, std::ios::in);
        std::vector<uint8_t> data;
        data.resize((size_t)size);
        p->sgetn((char*)data.data(), size);
        auto bson = json::from_bson(data);
        cjs_code_result::ref result = bson;
#endif
        std::vector<js_sym_code_t::ref> funcs;
        funcs.push_back(result->code);
        std::copy(result->funcs.begin(), result->funcs.end(), std::back_inserter(funcs));
        for (auto& f : funcs) {
            f->consts.restore_funcs(funcs);
        }
        for (auto& f : funcs) {
            f->consts.save();
        }
        auto end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        stat.Format(L"加载缓存：%S，耗时：%lldms", filename.c_str(), duration);
        cjsgui::singleton().add_stat(stat);
        return result;
    }

    void cjsruntime::save_cache(const std::string& filename, cjs_code_result::ref code) const
    {
        if (filename.empty())
            return;
        if ((filename[0] == '<' || filename[0] == '('))
            return;
        using nlohmann::json;
        json j = code;
        auto json_name = ROOT_DIR + filename + BINARY_EXT;
#if BINARY_VERBOSE
        std::ofstream ofs(json_name);
        ofs << j.dump(2);
#else
        std::ofstream ofs(json_name, std::ios::binary);
        auto bson = json::to_bson(j);
        ofs.write((const char*)bson.data(), bson.size());
#endif
    }

    void cjsruntime::reuse_value(const js_value::ref& v) {
        if (!v)
            return;
        v->__proto__.reset();
        switch (v->get_type()) {
        case r_number:
            reuse.reuse_numbers.push_back(
                std::dynamic_pointer_cast<jsv_number>(v));
            break;
        case r_string:
            reuse.reuse_strings.push_back(
                std::dynamic_pointer_cast<jsv_string>(v)->clear());
            break;
        case r_boolean:
            reuse.reuse_booleans.push_back(
                std::dynamic_pointer_cast<jsv_boolean>(v));
            break;
        case r_object:
            reuse.reuse_objects.push_back(
                std::dynamic_pointer_cast<jsv_object>(v)->clear());
            break;
        case r_function:
            reuse.reuse_functions.push_back(
                std::dynamic_pointer_cast<jsv_function>(v)->clear2());
            break;
        case r_regex:
            reuse.reuse_regexes.push_back(
                std::dynamic_pointer_cast<jsv_regexp>(v)->clear2());
            break;
        default:
            break;
        }
    }

    cjs_function::ref cjsruntime::new_stack(const js_sym_code_t::ref& code) {
        if (reuse_stack.empty()) {
            auto st = std::make_shared<cjs_function>(code, *this);
            st->envs = new_object();
            st->_this = stack.front()->envs;
            return st;
        }
        else {
            auto st = reuse_stack.back();
            st->envs = new_object();
            reuse_stack.pop_back();
            st->reset(code, *this);
            return st;
        }
    }

    cjs_function::ref cjsruntime::new_stack(const cjs_function_info::ref& code) {
        if (reuse_stack.empty()) {
            auto st = std::make_shared<cjs_function>(code);
            st->envs = new_object();
            st->_this = stack.front()->envs;
            return st;
        }
        else {
            auto st = reuse_stack.back();
            st->envs = new_object();
            reuse_stack.pop_back();
            st->reset(code);
            return st;
        }
    }

    void cjsruntime::delete_stack(const cjs_function::ref& f) {
        if (f == permanents.default_stack) {
            f->name = "<default>";
            f->ret_value.reset();
            return;
        }
        f->clear();
        reuse_stack.push_back(f);
    }

    void cjsruntime::gc() {
        permanents.global_env->mark(0);
        std::for_each(objs.begin(), objs.end(), [](auto& x) { x->mark(0); });
        for (const auto& s : stack) {
            const auto& st = s->stack;
            const auto& ret = s->ret_value.lock();
            const auto& th = s->_this.lock();
            const auto& env = s->envs.lock();
            const auto& closure = s->closure.lock();
            const auto& tr = s->trys;
            for (const auto& s2 : st) {
                if (s2.lock())
                    s2.lock()->mark(1);
            }
            env->mark(2);
            if (closure)
                closure->mark(2);
            if (th)
                th->mark(3);
            if (ret)
                th->mark(4);
            for (const auto& s2 : tr) {
                if (s2->obj.lock())
                    s2->obj.lock()->mark(7);
            }
        }
        for (const auto& s : timeout.ids) {
            s.second->func->mark(5);
            for (const auto& s2 : s.second->args) {
                s2.lock()->mark(6);
            }
        }
#if DUMP_STEP && DUMP_GC
        dump_step3();
#endif
        for (auto i = objs.begin(); i != objs.end();) {
            if ((*i)->marked == 0) {
                if (!((*i)->attr & js_value::at_const)) {
#if DUMP_STEP && DUMP_GC
                    fprintf(stdout, " GC  | [%p] Reuse, ", (*i).get());
                    print(*i, 0, std::cout);
#endif
                    reuse_value(*i);
                }
                i = objs.erase(i);
            }
            else
                i++;
        }
#if DUMP_STEP
        std::cout << std::setfill('#') << std::setw(60) << "" << std::endl;
#endif
    }

    jsv_number::ref cjsruntime::_new_number(double n, uint32_t attr) {
        auto s = std::make_shared<jsv_number>(n);
        if (attr & js_value::at_refs) {
            attr &= (uint32_t)~js_value::at_refs;
            permanents.refs.push_back(s);
        }
        if (attr > 0U) s->attr = attr;
        s->__proto__ = permanents._proto_number;
        return s;
    }

    jsv_string::ref cjsruntime::_new_string(const std::string& str, uint32_t attr) {
        auto s = std::make_shared<jsv_string>(str);
        if (attr & js_value::at_refs) {
            attr &= (uint32_t)~js_value::at_refs;
            permanents.refs.push_back(s);
        }
        if (attr > 0U) s->attr = attr;
        s->__proto__ = permanents._proto_string;
        return s;
    }

    jsv_boolean::ref cjsruntime::_new_boolean(bool b, uint32_t attr) {
        auto s = std::make_shared<jsv_boolean>(b);
        if (attr & js_value::at_refs) {
            attr &= (uint32_t)~js_value::at_refs;
            permanents.refs.push_back(s);
        }
        if (attr > 0U) s->attr = attr;
        s->__proto__ = permanents._proto_boolean;
        return s;
    }

    jsv_object::ref cjsruntime::_new_object(uint32_t attr) {
        auto s = std::make_shared<jsv_object>();
        if (attr & js_value::at_refs) {
            attr &= (uint32_t)~js_value::at_refs;
            permanents.refs.push_back(s);
        }
        if (attr > 0U) s->attr = attr;
        s->__proto__ = permanents._proto_object;
        return s;
    }

    jsv_function::ref cjsruntime::_new_function(jsv_object::ref proto, uint32_t attr) {
        auto s = std::make_shared<jsv_function>();
        if (attr & js_value::at_refs) {
            attr &= (uint32_t)~js_value::at_refs;
            permanents.refs.push_back(s);
        }
        if (attr > 0U) s->attr = attr;
        s->__proto__ = permanents._proto_function;
        if (proto) {
            proto->add("constructor", s);
            s->add("prototype", proto);
        }
        else {
            auto prototype = _new_object(js_value::at_refs);
            prototype->add("constructor", s);
            s->add("prototype", prototype);
        }
        return s;
    }

    jsv_null::ref cjsruntime::_new_null(uint32_t attr) {
        auto s = std::make_shared<jsv_null>();
        if (attr & js_value::at_refs) {
            attr &= (uint32_t)~js_value::at_refs;
            permanents.refs.push_back(s);
        }
        if (attr > 0U) s->attr = attr;
        s->__proto__ = permanents._proto_root;
        return s;
    }

    jsv_undefined::ref cjsruntime::_new_undefined(uint32_t attr) {
        auto s = std::make_shared<jsv_undefined>();
        if (attr & js_value::at_refs) {
            attr &= (uint32_t)~js_value::at_refs;
            permanents.refs.push_back(s);
        }
        if (attr > 0U) s->attr = attr;
        s->__proto__ = permanents._proto_root;
        return s;
    }

    jsv_regexp::ref cjsruntime::_new_regex(uint32_t attr) {
        auto s = std::make_shared<jsv_regexp>();
        if (attr & js_value::at_refs) {
            attr &= (uint32_t)~js_value::at_refs;
            permanents.refs.push_back(s);
        }
        if (attr > 0U) s->attr = attr;
        s->__proto__ = permanents._proto_regexp;
        return s;
    }

    js_value::ref cjsruntime::binop(int code, const js_value::ref& _op1, const js_value::ref& _op2, int* r) {
        assert(r);
        auto conv = js_value::conv_number;
        switch (code) {
        case COMPARE_EQUAL:
        case COMPARE_FEQUAL:
        case COMPARE_NOT_EQUAL:
        case COMPARE_FNOT_EQUAL: {
            if (!_op1->is_primitive() && !_op2->is_primitive()) {
                switch (code) {
                case COMPARE_EQUAL:
                    return new_boolean(_op1 == _op2);
                case COMPARE_NOT_EQUAL:
                    return new_boolean(_op1 != _op2);
                case COMPARE_FEQUAL:
                    return new_boolean(_op1 == _op2);
                case COMPARE_FNOT_EQUAL:
                    return new_boolean(_op1 != _op2);
                default:
                    break;
                }
            }
        }
                               break;
        default:
            break;
        }
        auto op1 = _op1->to_primitive(*this, js_value::conv_default, r);
        assert(op1);
        auto op2 = _op2->to_primitive(*this, js_value::conv_default, r);
        assert(op2);
        if (code == BINARY_ADD) {
            if (op1->get_type() == r_string || op2->get_type() == r_string)
                conv = js_value::conv_string;
        }
        else {
            switch (code) {
            case COMPARE_LESS:
            case COMPARE_LESS_EQUAL:
            case COMPARE_GREATER:
            case COMPARE_GREATER_EQUAL:
                if (op1->get_type() == r_string && op2->get_type() == r_string)
                    conv = js_value::conv_string;
                break;
            default:
                break;
            }
        }
        if (conv == js_value::conv_string) {
            auto s1 = op1->to_string(this, op1->get_type() == r_number ? 1 : 0);
            auto s2 = op2->to_string(this, op2->get_type() == r_number ? 1 : 0);
            switch (code) {
            case COMPARE_LESS:
                return new_boolean(s1 < s2);
            case COMPARE_LESS_EQUAL:
                return new_boolean(s1 <= s2);
            case COMPARE_GREATER:
                return new_boolean(s1 > s2);
            case COMPARE_GREATER_EQUAL:
                return new_boolean(s1 >= s2);
            case BINARY_ADD:
                return new_string(s1 + s2);
            default:
                assert(!"invalid binop type");
                break;
            }
        }
        else {
            double s1, s2;
            switch (code) {
            case COMPARE_LESS:
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                return new_boolean(s1 < s2);
            case COMPARE_LESS_EQUAL:
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                return new_boolean(s1 <= s2);
            case COMPARE_GREATER:
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                return new_boolean(s1 > s2);
            case COMPARE_GREATER_EQUAL:
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                return new_boolean(s1 >= s2);
            case COMPARE_EQUAL: {
                if (op1->get_type() != op2->get_type()) {
                    if (op1->get_type() == r_null) {
                        return new_boolean(op2->get_type() == r_undefined);
                    }
                    if (op2->get_type() == r_null) {
                        return new_boolean(op1->get_type() == r_undefined);
                    }
                    return new_boolean(op1->to_number(this) == op2->to_number(this));
                }
                switch (op1->get_type()) {
                case r_string:
                    return new_boolean(JS_STR(op1) == JS_STR(op2));
                case r_number:
                    return new_boolean(JS_NUM(op1) == JS_NUM(op2));
                default:
                    return new_boolean(op1 == op2);
                }
            }
            case COMPARE_NOT_EQUAL: {
                if (op1->get_type() != op2->get_type()) {
                    if (op1->get_type() == r_null) {
                        return new_boolean(!(op2->get_type() == r_undefined));
                    }
                    if (op2->get_type() == r_null) {
                        return new_boolean(!(op1->get_type() == r_undefined));
                    }
                    return new_boolean(op1->to_number(this) != op2->to_number(this));
                }
                switch (op1->get_type()) {
                case r_string:
                    return new_boolean(JS_STR(op1) != JS_STR(op2));
                case r_number:
                    return new_boolean(JS_NUM(op1) != JS_NUM(op2));
                default:
                    return new_boolean(op1 != op2);
                }
            }
            case COMPARE_FEQUAL: {
                if (_op1->get_type() != _op2->get_type()) {
                    return new_boolean(false);
                }
                switch (op1->get_type()) {
                case r_string:
                    return new_boolean(JS_STR(op1) == JS_STR(op2));
                case r_number:
                    return new_boolean(JS_NUM(op1) == JS_NUM(op2));
                default:
                    return new_boolean(op1 == op2);
                }
            }
            case COMPARE_FNOT_EQUAL: {
                if (_op1->get_type() != _op2->get_type()) {
                    return new_boolean(true);
                }
                switch (op1->get_type()) {
                case r_string:
                    return new_boolean(JS_STR(op1) != JS_STR(op2));
                case r_number:
                    return new_boolean(JS_NUM(op1) != JS_NUM(op2));
                default:
                    return new_boolean(op1 != op2);
                }
            }
            case BINARY_POWER:
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                if (s2 == 0)
                    return new_number(1.0);
                if ((s1 == 1.0 || s1 == -1.0) && std::isinf(s2))
                    return new_number(NAN);
                return new_number(pow(s1, s2));
            case BINARY_MULTIPLY:
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                return new_number(s1 * s2);
            case BINARY_MODULO:
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                if (std::isinf(s1) || s2 == 0)
                    return new_number(NAN);
                if (std::isinf(s2))
                    return new_number(s1);
                if (s1 == 0)
                    return new_number(std::isnan(s2) ? NAN : s1);
                return new_number(fmod(s1, s2));
            case BINARY_ADD:
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                return new_number(s1 + s2);
            case BINARY_SUBTRACT:
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                return new_number(s1 - s2);
            case BINARY_TRUE_DIVIDE:
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                return new_number(s1 / s2);
            case BINARY_LSHIFT: {
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                if (s2 == 0.0)
                    return new_number(fix(s1) == 0.0 ? 0.0 : fix(s1));
                auto a = int(fix(s1));
                auto b = fix(s2);
                auto c = b > 0 ? (uint32_t(b) % 32) : uint32_t(int(fmod(b, 32)) + 32);
                return new_number(double(int(a << c)));
            }
            case BINARY_RSHIFT: {
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                if (s2 == 0.0)
                    return new_number(fix(s1) == 0.0 ? 0.0 : fix(s1));
                auto a = int(fix(s1));
                auto b = fix(s2);
                auto c = b > 0 ? (uint32_t(b) % 32) : uint32_t(int(fmod(b, 32)) + 32);
                return new_number(double(int(a >> c)));
            }
            case BINARY_URSHIFT: {
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                if (s2 == 0.0)
                    return new_number(fix(s1) == 0.0 ? 0.0 : uint32_t(fix(s1)));
                auto a = uint32_t(fix(s1));
                auto b = fix(s2);
                auto c = b > 0 ? (uint32_t(b) % 32) : uint32_t(int(fmod(b, 32)) + 32);
                return new_number(double(uint32_t(a >> c)));
            }
            case BINARY_AND: {
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                auto a = uint32_t(fix(s1));
                auto b = uint32_t(fix(s2));
                return new_number(double(int(a & b)));
            }
            case BINARY_XOR: {
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                auto a = uint32_t(fix(s1));
                auto b = uint32_t(fix(s2));
                return new_number(double(int(a ^ b)));
            }
            case BINARY_OR: {
                s1 = op1->to_number(this);
                s2 = op2->to_number(this);
                auto a = uint32_t(fix(s1));
                auto b = uint32_t(fix(s2));
                return new_number(double(int(a | b)));
            }
            default:
                assert(!"invalid binop type");
                break;
            }
        }
        assert(!"invalid binop type");
        return new_number(NAN);
    }

    void cjsruntime::print(const js_value::ref& value, int level, std::ostream& os) {
        if (value == nullptr) {
            os << "undefined" << std::endl;
            return;
        }
        if (level > 10) {
            os << "too many lines" << std::endl;
            return;
        }
        auto type = value->get_type();
        os << std::setfill(' ') << std::setw(level) << "";
        switch (type) {
        case r_number: {
            auto n = std::dynamic_pointer_cast<jsv_number>(value);
            os << "number: " << std::fixed << n->number << std::endl;
        }
                     break;
        case r_string: {
            auto n = std::dynamic_pointer_cast<jsv_string>(value);
            os << "string: " << n->str << std::endl;
        }
                     break;
        case r_boolean: {
            auto n = std::dynamic_pointer_cast<jsv_boolean>(value);
            os << "boolean: " << std::boolalpha << n->b << std::endl;
        }
                      break;
        case r_object: {
            auto n = std::dynamic_pointer_cast<jsv_object>(value);
            if (!n->special.empty()) {
                os << "object: [[primitive]] " << n->to_string(nullptr, 0) << std::endl;
            }
            else {
                os << "object: " << std::endl;
                for (const auto& s : n->get_keys()) {
                    os << std::setfill(' ') << std::setw(level) << "";
                    os << s << ": " << std::endl;
                    print(n->get(s), level + 1, os);
                }
            }
        }
                     break;
        case r_function: {
            auto n = std::dynamic_pointer_cast<jsv_function>(value);
            if (n->builtin)
                os << "function: builtin " << n->name << std::endl;
            else if (n->code) {
                os << "function: " << n->code->debugName << " ";
                os << n->code->text << std::endl;;
                if (n->closure.lock()) {
                    print(n->closure.lock(), level + 1, os);
                }
            }
            else {
                os << "function: (cleaned) " << n->name << std::endl;
            }
        }
                       break;
        case r_null: {
            os << "null" << std::endl;
        }
                   break;
        case r_undefined: {
            os << "undefined" << std::endl;
        }
                        break;
        default:
            break;
        }
    }
}
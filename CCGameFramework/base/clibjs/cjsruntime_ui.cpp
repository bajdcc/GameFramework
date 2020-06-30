//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include "cjsruntime.h"


namespace clib {

    static void set_location(const js_ui_base::ref &u, const jsv_object::ref& _this, const jsv_object::ref& obj, js_value_new* n) {
        auto o = obj->get("left", n);
        if (o && o->get_type() == r_number) {
            _this->add("left", o);
            auto num = JS_NUM(o);
            if (!std::isinf(num) && !std::isnan(num)) {
                u->left = num;
            }
        }
        o = obj->get("right", n);
        if (o && o->get_type() == r_number) {
            _this->add("right", o);
            auto num = JS_NUM(o);
            if (!std::isinf(num) && !std::isnan(num)) {
                u->left = num;
            }
        }
        o = obj->get("width", n);
        if (o && o->get_type() == r_number) {
            _this->add("width", o);
            auto num = JS_NUM(o);
            if (!std::isinf(num) && !std::isnan(num)) {
                u->left = num;
            }
        }
        o = obj->get("height", n);
        if (o && o->get_type() == r_number) {
            _this->add("height", o);
            auto num = JS_NUM(o);
            if (!std::isinf(num) && !std::isnan(num)) {
                u->left = num;
            }
        }
    }

    bool jsv_ui::init(const jsv_object::ref& obj, js_value_new* n)
    {
        auto v = obj->get("type", n);
        if (v && v->get_type() == r_string) {
            add("length", n->new_number(0));
            auto type = JS_STR(v);
            if (type == "label") {
                add("type", v);
                auto u = std::make_shared<js_ui_label>();
                set_location(u, JS_O(shared_from_this()), obj, n);
                auto o = obj->get("content", n);
                if (o && o->get_type() == r_string) {
                    add("content", o);
                    u->set_content(JS_S(o)->wstr);
                }
                element = u;
                return true;
            }
            if (type == "root") {
                add("type", v);
                return true;
            }
        }
        return false;
    }

    void cjsruntime::send_signal(const std::string& s)
    {
        if (!global_ui.signals.empty() && global_ui.signals.back() == s)
            return;
        global_ui.signals.push_back(s);
    }

    int cjsruntime::get_frame() const
    {
        return global_ui.frames;
    }

    void cjsruntime::clear_frame()
    {
        global_ui.frames = 0;
    }

    void cjsruntime::eval_ui() {
        if (current_stack || !stack.empty())
            return;
        if (global_ui.signals.empty()) {
            global_ui.frames++;
            static auto name = std::string("<signal::render>");
            if (is_cached(name)) {
                exec(name, "", false, false);
                return;
            }
            static auto code = std::string("sys.send_signal(\"render\");");
            exec(name, code, false, false);
        }
        else {
            auto s = global_ui.signals.front();
            std::stringstream ss;
            ss << "<signal::" << s << ">";
            auto name = ss.str();
            if (is_cached(name)) {
                exec(name, "", false, false);
                return;
            }
            ss.str("");
            ss << "sys.send_signal(\"" << s << "\");";
            exec(name, ss.str(), false, false);
            global_ui.signals.pop_front();
        }
    }
}
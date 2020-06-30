//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include "cjsruntime.h"
#include "cjsgui.h"

namespace clib {

    static void set_location(const js_ui_base::ref &u, const jsv_object::ref& _this, const jsv_object::ref& obj, js_value_new* n) {
        auto o = obj->get("left", n);
        if (o && o->get_type() == r_number) {
            _this->add("left", o);
            auto num = JS_NUM(o);
            if (!std::isinf(num) && !std::isnan(num)) {
                u->left = (LONG)num;
            }
        }
        o = obj->get("top", n);
        if (o && o->get_type() == r_number) {
            _this->add("top", o);
            auto num = JS_NUM(o);
            if (!std::isinf(num) && !std::isnan(num)) {
                u->top = (LONG)num;
            }
        }
        o = obj->get("width", n);
        if (o && o->get_type() == r_number) {
            _this->add("width", o);
            auto num = JS_NUM(o);
            if (!std::isinf(num) && !std::isnan(num)) {
                u->width = (LONG)num;
            }
        }
        o = obj->get("height", n);
        if (o && o->get_type() == r_number) {
            _this->add("height", o);
            auto num = JS_NUM(o);
            if (!std::isinf(num) && !std::isnan(num)) {
                u->height = (LONG)num;
            }
        }
    }

    int jsv_ui::get_object_type() const
    {
        return jsv_object::T_UI;
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

    void jsv_ui::render()
    {
        if (element)
            element->render();
    }

    static LONG obj2num(const js_value::weak_ref& obj) {
        auto o = obj.lock();
        if (o->get_type() != r_number) {
            return 0;
        }
        auto num = JS_NUM(o);
        if (std::isinf(num) || std::isnan(num)) {
            return 0;
        }
        return (LONG)num;
    }

    void jsv_ui::add(const std::string& s, const js_value::weak_ref& obj)
    {
        jsv_object::add(s, obj);
        if (element) {
            if (s == "left")
                element->left = obj2num(obj);
            if (s == "top")
                element->top = obj2num(obj);
            if (s == "width")
                element->width = obj2num(obj);
            if (s == "height")
                element->height = obj2num(obj);
        }
    }

    void jsv_ui::remove(const std::string& s)
    {
        jsv_object::remove(s);
        if (element) {
            if (s == "left")
                element->left = 0;
            if (s == "top")
                element->top = 0;
            if (s == "width")
                element->width = 0;
            if (s == "height")
                element->height = 0;
        }

    }

    void jsv_ui::change_target()
    {
        if (element)
            element->change_target();
    }

    void cjsruntime::send_signal(const std::string& s)
    {
        if (!global_ui.signals.empty() && global_ui.signals.back() == s)
            return;
        global_ui.signals.push_back(s);
    }

    void cjsruntime::change_target()
    {
        for (auto& s : global_ui.elements) {
            s->change_target();
        }
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
            global_ui.signals.pop_front();
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
        }
    }

    js_ui_label::js_ui_label()
    {
        label = SolidLabelElement::Create();
    }

    int js_ui_label::get_type()
    {
        return js_ui_base::label;
    }

    const char* js_ui_label::get_type_str() const
    {
        return "label";
    }

    void js_ui_label::set_content(const std::wstring& s)
    {
        label->SetText(CString(s.c_str()));
    }

    void js_ui_label::render()
    {
        auto bounds = cjsgui::singleton().get_global().bound;
        bounds.left += left;
        bounds.top += top;
        bounds.right -= left + width;
        bounds.bottom -= top + height;
        //label->SetRenderRect(bounds);
        label->GetRenderer()->Render(bounds);
    }

    void js_ui_label::change_target()
    {
        label->GetRenderer()->SetRenderTarget(cjsgui::singleton().get_global().renderTarget.lock());
    }
}
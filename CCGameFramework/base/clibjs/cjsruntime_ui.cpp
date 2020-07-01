//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include "cjsruntime.h"
#include "cjsgui.h"

namespace clib {

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

    static void set_location(const js_ui_base::ref &u, const jsv_object::ref& _this, const jsv_object::ref& obj, js_value_new* n) {
        auto o = obj->get("left", n);
        if (o && o->get_type() == r_number) {
            _this->add("left", o);
            u->left = obj2num(o);
        }
        o = obj->get("top", n);
        if (o && o->get_type() == r_number) {
            _this->add("top", o);
            u->top = obj2num(o);
        }
        o = obj->get("width", n);
        if (o && o->get_type() == r_number) {
            _this->add("width", o);
            u->width = obj2num(o);
        }
        o = obj->get("height", n);
        if (o && o->get_type() == r_number) {
            _this->add("height", o);
            u->height = obj2num(o);
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
                element = u;
                set_location(u, JS_O(shared_from_this()), obj, n);
                auto o = obj->get("color", n);
                if (o && o->get_type() == r_string) {
                    add("color", o);
                    u->set_color(JS_STR(o));
                }
                o = obj->get("content", n);
                if (o && o->get_type() == r_string) {
                    add("content", o);
                    u->set_content(JS_S(o)->wstr);
                }
                o = obj->get("font", n);
                if (o && !o->is_primitive()) {
                    add("font", o);
                    auto font = JS_O(o);
                    auto f = u->get_font();
                    auto o = font->get("family", n);
                    if (o && o->get_type() == r_string) {
                        f.fontFamily = CStringA(JS_STR(o).c_str());
                        if (f.fontFamily.IsEmpty()) {
                            f.fontFamily = _T("Microsoft Yahei");
                        }
                    }
                    o = font->get("size", n);
                    if (o && o->get_type() == r_number) {
                        f.size = max(1, min(256, (cint)JS_NUM(o)));
                    }
                    o = font->get("bold", n);
                    if (o && o->get_type() == r_boolean) {
                        f.bold = JS_BOOL(o);
                    }
                    o = font->get("italic", n);
                    if (o && o->get_type() == r_boolean) {
                        f.italic = JS_BOOL(o);
                    }
                    o = font->get("underline", n);
                    if (o && o->get_type() == r_boolean) {
                        f.underline = JS_BOOL(o);
                    }
                    o = font->get("strikeline", n);
                    if (o && o->get_type() == r_boolean) {
                        f.strikeline = JS_BOOL(o);
                    }
                    o = font->get("antialias", n);
                    if (o && o->get_type() == r_boolean) {
                        f.antialias = JS_BOOL(o);
                    }
                    o = font->get("verticalAntialias", n);
                    if (o && o->get_type() == r_boolean) {
                        f.verticalAntialias = JS_BOOL(o);
                    }
                    u->set_font(f);
                }
                o = obj->get("align", n);
                if (o && o->get_type() == r_string) {
                    add("align", o);
                    auto a = JS_STR(o);
                    if (a == "left")
                        u->set_align(Alignment::StringAlignmentNear);
                    else if (a == "center")
                        u->set_align(Alignment::StringAlignmentCenter);
                    else if (a == "right")
                        u->set_align(Alignment::StringAlignmentFar);
                }
                o = obj->get("valign", n);
                if (o && o->get_type() == r_string) {
                    add("valign", o);
                    auto a = JS_STR(o);
                    if (a == "top")
                        u->set_valign(Alignment::StringAlignmentNear);
                    else if (a == "center")
                        u->set_valign(Alignment::StringAlignmentCenter);
                    else if (a == "bottom")
                        u->set_valign(Alignment::StringAlignmentFar);
                }
                return true;
            }
            if (type == "rect") {
                add("type", v);
                auto u = std::make_shared<js_ui_rect>();
                element = u;
                set_location(u, JS_O(shared_from_this()), obj, n);
                auto o = obj->get("color", n);
                if (o && o->get_type() == r_string) {
                    add("color", o);
                    u->set_color(JS_STR(o));
                }
                o = obj->get("fill", n);
                if (o && o->get_type() == r_boolean) {
                    add("color", o);
                    u->set_fill(JS_BOOL(o));
                }
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
        if (element && cjsgui::singleton().get_global().drawing)
            element->render();
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
        permanents.last = 4;
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

    // ---------------------- LABEL ----------------------

    js_ui_label::js_ui_label()
    {
        label = SolidLabelElement::Create();
        change_target();
    }

    int js_ui_label::get_type()
    {
        return js_ui_base::label;
    }

    const char* js_ui_label::get_type_str() const
    {
        return "label";
    }

    void js_ui_label::set_color(const std::string& s)
    {
        label->SetColor(CColor::Parse(CStringA(s.c_str())));
    }

    void js_ui_label::set_content(const std::wstring& s)
    {
        label->SetText(CString(s.c_str()));
    }

    void js_ui_label::set_font(const Font& f)
    {
        label->SetFont(f);
    }

    Font js_ui_label::get_font() const
    {
        return label->GetFont();
    }

    void js_ui_label::set_align(Alignment a)
    {
        label->SetHorizontalAlignment(a);
    }

    void js_ui_label::set_valign(Alignment a)
    {
        label->SetVerticalAlignment(a);
    }

    void js_ui_label::render()
    {
        auto bounds = CRect(left, top, left + width, top + height);
        label->SetRenderRect(bounds);
        label->GetRenderer()->Render(bounds, cjsgui::singleton().get_global().renderTarget);
    }

    void js_ui_label::clear()
    {
        label->GetRenderer()->Finalize();
    }

    void js_ui_label::change_target()
    {
        label->GetRenderer()->SetRenderTarget(cjsgui::singleton().get_global().canvas.lock());
    }

    // ---------------------- RECT ----------------------

    js_ui_rect::js_ui_rect()
    {
        rect = SolidBackgroundElement::Create();
        change_target();
    }

    int js_ui_rect::get_type()
    {
        return js_ui_base::rect;
    }

    const char* js_ui_rect::get_type_str() const
    {
        return "rect";
    }

    void js_ui_rect::set_color(const std::string& s)
    {
        rect->SetColor(CColor::Parse(CStringA(s.c_str())));
    }

    void js_ui_rect::set_fill(bool b)
    {
        rect->SetFill(b);
    }

    void js_ui_rect::render()
    {
        auto bounds = CRect(left, top, left + width, top + height);
        rect->SetRenderRect(bounds);
        rect->GetRenderer()->Render(bounds, cjsgui::singleton().get_global().renderTarget);
    }

    void js_ui_rect::clear()
    {
        rect->GetRenderer()->Finalize();
    }

    void js_ui_rect::change_target()
    {
        rect->GetRenderer()->SetRenderTarget(cjsgui::singleton().get_global().canvas.lock());
    }
}
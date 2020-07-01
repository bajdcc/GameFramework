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
        _this->add("left", obj->get("left", n));
        _this->add("top", obj->get("top", n));
        _this->add("width", obj->get("width", n));
        _this->add("height", obj->get("height", n));
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
                add("color", obj->get("color", n));
                add("content", obj->get("content", n));
                add("font", obj->get("font", n));
                add("size", obj->get("size", n));
                add("bold", obj->get("bold", n));
                add("italic", obj->get("italic", n));
                add("underline", obj->get("underline", n));
                add("strikeline", obj->get("strikeline", n));
                add("antialias", obj->get("antialias", n));
                add("verticalAntialias", obj->get("verticalAntialias", n));
                add("align", obj->get("align", n));
                add("valign", obj->get("valign", n));
                return true;
            }
            if (type == "rect") {
                add("type", v);
                auto u = std::make_shared<js_ui_rect>();
                element = u;
                set_location(u, JS_O(shared_from_this()), obj, n);
                add("color", obj->get("color", n));
                add("fill", obj->get("fill", n));
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
        if (!obj.lock())
            return;
        jsv_object::add(s, obj);
        if (element) {
            if (s == "left")
                element->left = obj2num(obj);
            else if (s == "top")
                element->top = obj2num(obj);
            else if (s == "width")
                element->width = obj2num(obj);
            else if (s == "height")
                element->height = obj2num(obj);
            else
                element->add(s, obj.lock());
        }
    }

    void jsv_ui::remove(const std::string& s)
    {
        jsv_object::remove(s);
        if (element) {
            if (s == "left")
                element->left = 0;
            else if (s == "top")
                element->top = 0;
            else if (s == "width")
                element->width = 0;
            else if (s == "height")
                element->height = 0;
            else
                element->remove(s);
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

    void js_ui_label::add(const std::string& s, const js_value::ref& obj)
    {
        if (s == "color") {
            if (obj->get_type() == r_string)
                label->SetColor(CColor::Parse(CStringA(JS_STR(obj).c_str())));
        }
        else if (s == "content") {
            if (obj->get_type() == r_string)
                label->SetText(CString(JS_S(obj)->wstr.c_str()));
        }
        else if (s == "font") {
            if (obj && !obj->is_primitive()) {
                auto font = JS_O(obj);
                auto f = label->GetFont();
                auto o = font->get("family", nullptr);
                if (o && o->get_type() == r_string) {
                    f.fontFamily = CStringA(JS_STR(o).c_str());
                    if (f.fontFamily.IsEmpty()) {
                        f.fontFamily = _T("Microsoft Yahei");
                    }
                }
                o = font->get("size", nullptr);
                if (o && o->get_type() == r_number) {
                    f.size = max(1, min(256, (cint)JS_NUM(o)));
                }
                o = font->get("bold", nullptr);
                if (o && o->get_type() == r_boolean) {
                    f.bold = JS_BOOL(o);
                }
                o = font->get("italic", nullptr);
                if (o && o->get_type() == r_boolean) {
                    f.italic = JS_BOOL(o);
                }
                o = font->get("underline", nullptr);
                if (o && o->get_type() == r_boolean) {
                    f.underline = JS_BOOL(o);
                }
                o = font->get("strikeline", nullptr);
                if (o && o->get_type() == r_boolean) {
                    f.strikeline = JS_BOOL(o);
                }
                o = font->get("antialias", nullptr);
                if (o && o->get_type() == r_boolean) {
                    f.antialias = JS_BOOL(o);
                }
                o = font->get("verticalAntialias", nullptr);
                if (o && o->get_type() == r_boolean) {
                    f.verticalAntialias = JS_BOOL(o);
                }
                label->SetFont(f);
            }
        }
        else if (s == "align") {
            if (obj->get_type() == r_string) {
                auto a = JS_STR(obj);
                if (a == "left")
                    label->SetHorizontalAlignment(Alignment::StringAlignmentNear);
                else if (a == "center")
                    label->SetHorizontalAlignment(Alignment::StringAlignmentCenter);
                else if (a == "right")
                    label->SetHorizontalAlignment(Alignment::StringAlignmentFar);
            }
        }
        else if (s == "valign") {
            if (obj->get_type() == r_string) {
                auto a = JS_STR(obj);
                if (a == "top")
                    label->SetHorizontalAlignment(Alignment::StringAlignmentNear);
                else if (a == "center")
                    label->SetHorizontalAlignment(Alignment::StringAlignmentCenter);
                else if (a == "bottom")
                    label->SetHorizontalAlignment(Alignment::StringAlignmentFar);
            }
        }
    }

    void js_ui_label::remove(const std::string& s)
    {
        if (s == "color") {
            label->SetColor(CColor());
        }
        else if (s == "content") {
            label->SetText(CString());
        }
        else if (s == "font") {
            Font f;
            f.fontFamily = _T("Microsoft Yahei");
            f.size = 12;
            label->SetFont(f);
        }
        else if (s == "align") {
            label->SetHorizontalAlignment(Alignment::StringAlignmentNear);
        }
        else if (s == "valign") {
            label->SetVerticalAlignment(Alignment::StringAlignmentNear);
        }
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

    void js_ui_rect::add(const std::string& s, const js_value::ref& obj)
    {
        if (s == "color") {
            if (obj->get_type() == r_string)
                rect->SetColor(CColor::Parse(CStringA(JS_STR(obj).c_str())));
        }
        else if (s == "fill") {
            if (obj->get_type() == r_boolean)
                rect->SetFill(JS_BOOL(obj));
        }
    }

    void js_ui_rect::remove(const std::string& s)
    {
        if (s == "color") {
            rect->SetColor(CColor());
        }
        else if (s == "fill") {
            rect->SetFill(true);
        }
    }
}
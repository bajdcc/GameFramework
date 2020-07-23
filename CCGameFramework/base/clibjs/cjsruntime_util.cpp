//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include "cjsruntime.h"
#include "cjsgui.h"

namespace clib {

    bool js_trans(const js_value::ref& obj, std::string& s) {
        if (!obj || obj->get_type() != r_string)
            return false;
        s = JS_STR(obj);
        return true;
    }

    bool js_trans(const js_value::ref& obj, double& d) {
        if (!obj || obj->get_type() != r_number)
            return false;
        d = JS_NUM(obj);
        return true;
    }

    bool js_trans(const js_value::ref& obj, jsv_object::ref& o) {
        if (!obj || obj->is_primitive())
            return false;
        o = JS_OBJ(obj);
        return true;
    }

    bool js_trans(const js_value::ref& obj) {
        if (!obj)
            return false;
        return obj->to_bool();
    }

    bool js_trans(const js_value::ref& obj, std::vector<js_value::ref>& v) {
        if (!obj || obj->is_primitive())
            return false;
        auto o = JS_OBJ(obj);
        auto l = 0.0;
        if (!js_trans(o->get("length"), l))
            return false;
        auto len = o->get("length");
        if (!std::isinf(l) && !std::isnan(l)) {
            auto i = 0, j = 0;
            auto L = (int)l;
            std::stringstream ss;
            while (i < L) {
                ss.str("");
                ss << j++;
                auto ff = o->get(ss.str());
                if (ff) {
                    v.push_back(ff);
                }
                i++;
            }
        }
        return true;
    }

    bool js_trans(const js_value::ref& obj, CRect& r) {
        if (!obj || obj->is_primitive())
            return false;
        auto o = JS_OBJ(obj);
        auto d = 0.0;
        if (js_trans(o->get("left"), d))
            r.left = (LONG)d;
        if (js_trans(o->get("right"), d))
            r.right = (LONG)d;
        if (js_trans(o->get("top"), d))
            r.top = (LONG)d;
        if (js_trans(o->get("bottom"), d))
            r.bottom = (LONG)d;
        return true;
    }

    extern int helper_Klotski_1(js_value_new* js, int size, const std::vector<CRect>& blocks, int red, js_value::ref& out);

    int cjsruntime::call_helper(std::vector<js_value::weak_ref>& args, js_value::ref& out)
    {
        js_value::ref ret = new_undefined();
        do {
            if (args.size() < 1)
                break;
            std::string type;
            if (!js_trans(args.front().lock(), type))
                break;
            if (type == "puzzle") {
                if (args.size() < 2)
                    break;
                jsv_object::ref data;
                if (!js_trans(args[1].lock(), data))
                    break;
                std::string t;
                if (!js_trans(data->get("type"), t))
                    break;
                if (t == "Klotski-1") {
                    double _size;
                    if (!js_trans(data->get("size"), _size))
                        break;
                    if (std::isinf(_size) || std::isnan(_size) || _size < 0 || _size >= 10) {
                        ret = new_string("invalid size");
                        break;
                    }
                    auto size = (int)_size;
                    std::vector<js_value::ref> blocks;
                    if (!js_trans(data->get("blocks"), blocks))
                        break;
                    std::vector<CRect> rects;
                    CRect r;
                    auto idx = 0, target = -1;
                    for (const auto& b : blocks) {
                        if (js_trans(b, r)) {
                            rects.push_back(r);
                            if (js_trans(JS_OBJ(b)->get("main"))) {
                                if(target == -1)
                                target = idx;
                                else {
                                    ret = new_string("multiple target");
                                    break;
                                }
                            }
                            if (r.left <= 0 || r.left > size) {
                                ret = new_string("invalid left");
                                break;
                            }
                            if (r.right <= 0 || r.right > size) {
                                ret = new_string("invalid right");
                                break;
                            }
                            if (r.top <= 0 || r.top > size) {
                                ret = new_string("invalid top");
                                break;
                            }
                            if (r.bottom <= 0 || r.bottom > size) {
                                ret = new_string("invalid bottom");
                                break;
                            }
                            if (r.Width() < 0) {
                                ret = new_string("invalid width");
                                break;
                            }
                            if (r.Height() < 0) {
                                ret = new_string("invalid height");
                                break;
                            }
                            if (r.Width() > 0 && r.Height() > 0) {
                                ret = new_string("invalid rect");
                                break;
                            }
                        }
                        idx++;
                    }
                    if (target == -1) {
                        ret = new_string("no target");
                        break;
                    }
                    return helper_Klotski_1(this, (int)size, rects, target, out);
                }
                ret = new_string("invalid type");
            }
            else if (type == "debugger") {
#ifdef DEBUG
                _asm int 3;
#endif
            }
        } while (0);
        push(ret);
        return 0;
    }
}
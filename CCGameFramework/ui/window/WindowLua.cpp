#include "stdafx.h"
#include "Window.h"

int ui_clear_scene(lua_State *L)
{
    for (auto& timer : window->setTimer)
    {
        ::KillTimer(window->handle, timer);
    }
    window->ptrEle = 0;
    window->mapEle.clear();
    window->setTimer.clear();
    window->root->GetRenderer()->SetRenderTarget(nullptr);
    window->root->GetChildren().clear();
    return 0;
}

int ui_add_obj(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, -1, "type");
    auto type = ElementId(cint(luaL_checkinteger(L, -1)));
    lua_pop(L, 1);
    window->ptrEle++;
    auto ptr = window->ptrEle;
    while (window->mapEle.find(ptr) != window->mapEle.end())
    {
        ptr++;
        if (ptr > 0x10000000)
            ptr = 0;
    }
    window->ptrEle = ptr;
    lua_getfield(L, -1, "parent");
    auto parent = ElementId(cint(luaL_checkinteger(L, -1)));
    decltype(window->root) p;
    if (parent == -1)
        p = window->root;
    else if (window->mapEle.find(parent) != window->mapEle.end())
        p = window->mapEle[parent].lock();
    else
        p = window->root;
    std::shared_ptr<IGraphicsElement> obj;
    switch (type)
    {
    case Empty:
        obj = EmptyElement::Create();
        break;
    case SolidBackground:
        obj = SolidBackgroundElement::Create();
        break;
    case SolidLabel:
        obj = SolidLabelElement::Create();
        break;
    case GradientBackground:
        obj = GradientBackgroundElement::Create();
        break;
    case RoundBorder:
        obj = RoundBorderElement::Create();
        break;
    case QRImage:
        obj = QRImageElement::Create();
        break;
    case Base64Image:
        obj = Base64ImageElement::Create();
        break;
    case WireworldAutomaton:
        obj = WireworldAutomatonImageElement::Create();
        break;
    case Edit:
        obj = EditElement::Create();
        break;
    default:
        return luaL_argerror(L, 1, "Invalid obj id");
    }
    obj->GetFlags().parent = p;
    p->GetChildren().push_back(obj);
    window->mapEle.insert(std::pair<decltype(ptr), decltype(obj)>(ptr, obj));
    lua_pushinteger(L, ptr);
    return 1;
}

int ui_update_obj(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, -1, "type");
    auto type = ElementId(cint(luaL_checkinteger(L, -1))); lua_pop(L, 1);
    lua_getfield(L, -1, "handle");
    auto handle = cint(luaL_checkinteger(L, -1)); lua_pop(L, 1);
    if (window->mapEle.find(handle) == window->mapEle.end())
        return luaL_argerror(L, 1, "Invalid obj id");
    auto o = window->mapEle[handle].lock();
    if (o->GetTypeId() != type)
        return luaL_argerror(L, 1, "Invalid obj type");
    {
        CRect rt;
        lua_getfield(L, -1, "left");
        rt.left = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        lua_getfield(L, -1, "top");
        rt.top = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        lua_getfield(L, -1, "right");
        rt.right = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        lua_getfield(L, -1, "bottom");
        rt.bottom = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        o->SetRenderRect(rt);
    }
    {
        lua_getfield(L, -1, "show_self");
        o->GetFlags().self_visible = cint(luaL_checkinteger(L, -1)) != 0; lua_pop(L, 1);
        lua_getfield(L, -1, "show_children");
        o->GetFlags().children_visible = cint(luaL_checkinteger(L, -1)) != 0; lua_pop(L, 1);
    }
    switch (type)
    {
    case Empty:
        break;
    case SolidBackground:
    {
        auto obj = static_cast<SolidBackgroundElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(color));
            lua_getfield(L, -1, "fill");
            auto fill = luaL_checknumber(L, -1) != 0; lua_pop(L, 1);
            obj->SetFill(fill);
        }
    }
    break;
    case SolidLabel:
    {
        auto obj = static_cast<SolidLabelElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(color));
        }
        {
            Font font;
            lua_getfield(L, -1, "size");
            font.size = cint(luaL_checkinteger(L, -1)); lua_pop(L, 1);
            lua_getfield(L, -1, "family");
            font.fontFamily = CString(luaL_checkstring(L, -1)); lua_pop(L, 1);
            lua_getfield(L, -1, "italic");
            font.italic = cint(luaL_checkinteger(L, -1)) != 0; lua_pop(L, 1);
            lua_getfield(L, -1, "bold");
            font.bold = cint(luaL_checkinteger(L, -1)) != 0; lua_pop(L, 1);
            lua_getfield(L, -1, "underline");
            font.underline = cint(luaL_checkinteger(L, -1)) != 0; lua_pop(L, 1);
            lua_getfield(L, -1, "strikeline");
            font.strikeline = cint(luaL_checkinteger(L, -1)) != 0; lua_pop(L, 1);
            obj->SetFont(font);
        }
        {
            lua_getfield(L, -1, "text");
            obj->SetText(CString(luaL_checkstring(L, -1))); lua_pop(L, 1);
        }
        {
            lua_getfield(L, -1, "align");
            obj->SetHorizontalAlignment(Alignment(cint(luaL_checkinteger(L, -1)))); lua_pop(L, 1);
            lua_getfield(L, -1, "valign");
            obj->SetVerticalAlignment(Alignment(cint(luaL_checkinteger(L, -1)))); lua_pop(L, 1);
        }
    }
    break;
    case GradientBackground:
    {
        auto obj = static_cast<GradientBackgroundElement*>(o.get());
        {
            lua_getfield(L, -1, "color1");
            auto color1 = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetColor1(CColor::Parse(color1));
            lua_getfield(L, -1, "color2");
            auto color2 = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetColor2(CColor::Parse(color2));
        }
        {
            lua_getfield(L, -1, "direction");
            obj->SetDirection(GradientBackgroundElement::Direction(cint(luaL_checkinteger(L, -1)))); lua_pop(L, 1);
        }
    }
    break;
    case RoundBorder:
    {
        auto obj = static_cast<RoundBorderElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(color));
            lua_getfield(L, -1, "radius");
            auto radius = luaL_checknumber(L, -1); lua_pop(L, 1);
            obj->SetRadius((FLOAT)radius);
            lua_getfield(L, -1, "fill");
            auto fill = luaL_checknumber(L, -1) != 0; lua_pop(L, 1);
            obj->SetFill(fill);
        }
    }
    break;
    case QRImage:
    {
        auto obj = static_cast<QRImageElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(color));
            lua_getfield(L, -1, "text");
            auto text = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetText(text);
            lua_getfield(L, -1, "opacity");
            auto opacity = (FLOAT)luaL_checknumber(L, -1); lua_pop(L, 1);
            obj->SetOpacity(opacity);
        }
    }
    break;
    case Base64Image:
    {
        auto obj = static_cast<Base64ImageElement*>(o.get());
        {
            lua_getfield(L, -1, "text");
            auto text = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetText(text);
            lua_getfield(L, -1, "url");
            auto url = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetUrl(url);
            lua_getfield(L, -1, "opacity");
            auto opacity = (FLOAT)luaL_checknumber(L, -1); lua_pop(L, 1);
            obj->SetOpacity(opacity);
        }
    }
    break;
    case WireworldAutomaton:
    {
        auto obj = static_cast<WireworldAutomatonImageElement*>(o.get());
        {
            lua_getfield(L, -1, "text");
            auto text = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetText(text);
            lua_getfield(L, -1, "opacity");
            auto opacity = (FLOAT)luaL_checknumber(L, -1); lua_pop(L, 1);
            obj->SetOpacity(opacity);
        }
    }
    break;
    case Edit:
    {
        auto obj = static_cast<EditElement*>(o.get());
        lua_getfield(L, -1, "text");
        auto text = luaL_checkstring(L, -1); lua_pop(L, 1);
        obj->SetText(CString(text));
        lua_getfield(L, -1, "multiline");
        auto multiline = (cint)luaL_checkinteger(L, -1) == 0; lua_pop(L, 1);
        obj->SetMultiline(multiline);
        {
            Font font;
            lua_getfield(L, -1, "size");
            font.size = cint(luaL_checkinteger(L, -1)); lua_pop(L, 1);
            lua_getfield(L, -1, "family");
            font.fontFamily = CString(luaL_checkstring(L, -1)); lua_pop(L, 1);
            lua_getfield(L, -1, "bold");
            font.bold = luaL_checkinteger(L, -1) != 0; lua_pop(L, 1);
            lua_getfield(L, -1, "italic");
            font.italic = luaL_checkinteger(L, -1) != 0; lua_pop(L, 1);
            lua_getfield(L, -1, "underline");
            font.underline = luaL_checkinteger(L, -1) != 0; lua_pop(L, 1);
            obj->SetFont(font);
        }
    }
        break;
    default:
        break;
    }
    return 0;
}

int ui_refresh_obj(lua_State *L)
{
    auto arg = (cint)luaL_checkinteger(L, 2); lua_pop(L, 1);
    luaL_checktype(L, -1, LUA_TTABLE);
    lua_getfield(L, -1, "type");
    auto type = ElementId(cint(luaL_checkinteger(L, -1))); lua_pop(L, 1);
    lua_getfield(L, -1, "handle");
    auto handle = cint(luaL_checkinteger(L, -1)); lua_pop(L, 1);
    if (window->mapEle.find(handle) == window->mapEle.end())
        return luaL_argerror(L, 1, "Invalid obj id");
    auto o = window->mapEle[handle].lock();
    if (o->GetTypeId() != type)
        return luaL_argerror(L, 1, "Invalid obj type");
    switch (type)
    {
    case Empty:
        break;
    case SolidBackground:
        break;
    case SolidLabel:
        break;
    case GradientBackground:
        break;
    case RoundBorder:
        break;
    case QRImage:
        break;
    case Base64Image:
        break;
    case WireworldAutomaton:
    {
        auto obj = static_cast<WireworldAutomatonImageElement*>(o.get());
        {
            obj->Refresh(arg);
        }
    }
    break;
    case Edit:
        break;
    default:
        break;
    }
    return 0;
}

int ui_info(lua_State *L)
{
    lua_newtable(L);
    auto size = window->GetClientWindowSize();
    lua_pushstring(L, "width");
    lua_pushinteger(L, size.cx);
    lua_settable(L, -3);
    lua_pushstring(L, "height");
    lua_pushinteger(L, size.cy);
    lua_settable(L, -3);
    return 1;
}

int ui_win_set_minsize(lua_State* L)
{
    auto w = cint(luaL_checkinteger(L, 1));
    auto h = cint(luaL_checkinteger(L, 2));
    window->SetMinSize(CSize(w, h));
    return 0;
}

int ui_paint(lua_State *L)
{
    window->Redraw();
    return 0;
}

int ui_set_timer(lua_State *L)
{
    auto id = cint(luaL_checkinteger(L, 1));
    auto elapse = cint(luaL_checkinteger(L, 2));
    window->SetTimer(id, elapse);
    return 0;
}

int ui_kill_timer(lua_State *L)
{
    auto id = cint(luaL_checkinteger(L, 1));
    window->KillTimer(id);
    return 0;
}

int ui_quit(lua_State* L)
{
    auto code = cint(luaL_checkinteger(L, 1));
    PostQuitMessage(code);
    return 0;
}
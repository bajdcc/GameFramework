#include "stdafx.h"
#include "Window.h"

int ui_clear_scene(lua_State *L)
{
    for (auto& timer : window->setTimer)
    {
        ::KillTimer(window->handle, timer);
    }
    window->setTimer.clear();
    window->layers.clear();
    return 0;
}

int ui_add_obj(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, -1, "type");
    auto type = ElementId(cint(luaL_checknumber(L, -1)));
    lua_pop(L, 1);
    switch (type)
    {
    case SolidBackground:
    {
        RefPtr<SolidBackgroundElement> obj = SolidBackgroundElement::Create();
        window->layers.push_back(obj);
        lua_pushnumber(L, window->layers.size());
    }
    break;
    case SolidLabel:
    {
        RefPtr<SolidLabelElement> obj = SolidLabelElement::Create();
        window->layers.push_back(obj);
        lua_pushnumber(L, window->layers.size());
    }
    break;
    case GradientBackground:
    {
        RefPtr<GradientBackgroundElement> obj = GradientBackgroundElement::Create();
        window->layers.push_back(obj);
        lua_pushnumber(L, window->layers.size());
    }
    break;
    default:
        return luaL_argerror(L, 1, "Invalid obj id");
    }
    return 1;
}

int ui_update_obj(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, -1, "type");
    auto type = ElementId(cint(luaL_checknumber(L, -1))); lua_pop(L, 1);
    lua_getfield(L, -1, "handle");
    auto handle = cint(luaL_checknumber(L, -1)); lua_pop(L, 1);
    if (handle <= 0 || handle > cint(window->layers.size()))
        return luaL_argerror(L, 1, "Invalid obj id");
    auto o = window->layers[handle - 1];
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
    switch (type)
    {
    case SolidBackground:
    {
        auto obj = static_cast<SolidBackgroundElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checklstring(L, -1, NULL); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(color));
        }
    }
    break;
    case SolidLabel:
    {
        auto obj = static_cast<SolidLabelElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checklstring(L, -1, NULL); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(color));
        }
        {
            Font font;
            lua_getfield(L, -1, "size");
            font.size = cint(luaL_checknumber(L, -1)); lua_pop(L, 1);
            lua_getfield(L, -1, "family");
            font.fontFamily = CString(luaL_checklstring(L, -1, NULL)); lua_pop(L, 1);
            obj->SetFont(font);
        }
        {
            lua_getfield(L, -1, "text");
            obj->SetText(CString(luaL_checklstring(L, -1, NULL))); lua_pop(L, 1);
        }
        {
            lua_getfield(L, -1, "align");
            obj->SetHorizontalAlignment(Alignment(cint(luaL_checknumber(L, -1)))); lua_pop(L, 1);
            lua_getfield(L, -1, "valign");
            obj->SetVerticalAlignment(Alignment(cint(luaL_checknumber(L, -1)))); lua_pop(L, 1);
        }
    }
    break;
    case GradientBackground:
    {
        auto obj = static_cast<GradientBackgroundElement*>(o.get());
        {
            lua_getfield(L, -1, "color1");
            auto color1 = luaL_checklstring(L, -1, NULL); lua_pop(L, 1);
            obj->SetColor1(CColor::Parse(color1));
            lua_getfield(L, -1, "color2");
            auto color2 = luaL_checklstring(L, -1, NULL); lua_pop(L, 1);
            obj->SetColor2(CColor::Parse(color2));
        }
        {
            lua_getfield(L, -1, "direction");
            obj->SetDirection(GradientBackgroundElement::Direction(cint(luaL_checknumber(L, -1)))); lua_pop(L, 1);
        }
    }
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
    lua_pushnumber(L, size.cx);
    lua_settable(L, -3);
    lua_pushstring(L, "height");
    lua_pushnumber(L, size.cy);
    lua_settable(L, -3);
    return 1;
}

int ui_paint(lua_State *L)
{
    window->Redraw();
    return 0;
}

int ui_set_timer(lua_State *L)
{
    auto id = cint(luaL_checknumber(L, 1));
    auto elapse = cint(luaL_checknumber(L, 2));
    window->SetTimer(id, elapse);
    return 0;
}

int ui_kill_timer(lua_State *L)
{
    auto id = cint(luaL_checknumber(L, 1));
    window->KillTimer(id);
    return 0;
}
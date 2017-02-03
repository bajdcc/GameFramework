#include "stdafx.h"
#include "UI.h"
#include "ext.h"

extern CTraceCategoryEx<CTraceCategoryEx<>::TraceUser> atlTraceLua(_T("atlTraceLua"));

static int ui_trace(lua_State *L)
{
    auto s = luaL_checkstring(L, 1);
    ATLTRACE(atlTraceLua, 0, "%s\n", s);
    return 0;
}

static const luaL_Reg ui_lib[] = {
    { "trace", ui_trace },
    { "clear_scene", ui_clear_scene },
    { "add_obj", ui_add_obj },
    { "update", ui_update_obj },
    { "info", ui_info },
    { nullptr, nullptr }
};

static int luaopen_UI(lua_State *L) {
    luaL_newlib(L, ui_lib);
    return 1;
}

void lua_ext_register_UI(lua_State *L)
{
    luaL_requiref(L, "UIExt", luaopen_UI, 1);
}
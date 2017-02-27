#include "stdafx.h"
#include "ext.h"
#include "UI.h"
#include "Web.h"

void lua_ext_register_all(lua_State *L)
{
    lua_ext_register_UI(L);
    lua_ext_register_web(L);
}
#include "stdafx.h"
#include "ext.h"
#include "UI.h"


void lua_ext_register_all(lua_State *L)
{
    lua_ext_register_UI(L);
}
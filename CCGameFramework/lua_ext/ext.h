#ifndef LUA_EXT
#define LUA_EXT

extern CTraceCategoryEx<CTraceCategoryEx<>::TraceUser> atlTraceLua;

void lua_ext_register_all(lua_State *L);

#endif
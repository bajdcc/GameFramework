#ifndef LUA_EXT
#define LUA_EXT

extern CTraceCategoryEx<CTraceCategoryEx<>::TraceUser> atlTraceLua;
extern std::map<std::string, lua_Integer> g_ui_map;

void lua_ext_register_all(lua_State *L);

#endif
#ifndef LUA_EXT_WEB
#define LUA_EXT_WEB

void lua_ext_register_web(lua_State *L);

extern int web_http_get(lua_State *L);
extern int web_http_get_b64(lua_State *L);

#endif
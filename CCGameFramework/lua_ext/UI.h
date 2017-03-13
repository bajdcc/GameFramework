#ifndef LUA_EXT_UI
#define LUA_EXT_UI

void lua_ext_register_UI(lua_State *L);

extern int ui_clear_scene(lua_State *L);
extern int ui_add_obj(lua_State *L);
extern int ui_update_obj(lua_State *L);
extern int ui_refresh_obj(lua_State *L);
extern int ui_info(lua_State *L);

extern int ui_win_set_minsize(lua_State *L);
extern int ui_paint(lua_State *L);

extern int ui_set_timer(lua_State *L);
extern int ui_kill_timer(lua_State *L);
extern int ui_quit(lua_State *L);

extern int ui_helper_hsl2rgb(lua_State *L);
extern int ui_helper_rgb2hsl(lua_State *L);

extern int ui_isprintchar(lua_State *L);
extern int ui_importchar(lua_State *L);
extern int ui_removechar(lua_State *L);
extern int ui_limitstr(lua_State *L);

extern int ui_play_song(lua_State *L);
extern int ui_music_ctl(lua_State *L);

extern int ui_parse_lyric(lua_State *L);

#endif
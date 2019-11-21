#include "/include/gui"
#include "/include/io"
#include "/include/memory"
#include "/include/xtoa_atoi"
#include "/include/arg"
#include "/include/shell"
int main(int argc, char **argv) {
    if (argc < 3) {
        set_fg(240, 0, 0);
        put_string("[ERROR] Invalid argument.");
        restore_fg();
        return 0;
    }
    int id = atoi32(argv[1]);
    if (id == 0) {
        char* name = arg_string(2, argc, argv);
        put_string("Play ID = ");
        put_string(name);
        put_string("\n");
        shell("sleep 1");
        char* code = malloc(200);
        strcpy(code, "MusicSceneName = '");
        strcat(code, name);
        strcat(code, "'; MusicSceneId = ");
        strcat(code, argv[1]);
        strcat(code, "; MusicSceneReturn = 'Parser2d'; FlipScene('Music');");
        gui_lua(code);
        return 0;
    }
    if (id < 1 || id > 4) {
        set_fg(240, 0, 0);
        put_string("[ERROR] Invalid id.");
        restore_fg();
        return 0;
    }
    char* name = arg_string(2, argc, argv);
    put_string("播放：");
    put_string(name);
    put_string("\n");
    shell("sleep 1");
    char* code = malloc(200);
    strcpy(code, "MusicSceneName = '");
    strcat(code, name);
    strcat(code, "'; MusicSceneId = ");
    strcat(code, argv[1]);
    strcat(code, "; MusicSceneReturn = 'Parser2d'; FlipScene('Music');");
    gui_lua(code);
    return 0;
}
#include "/include/shell"
#include "/include/xtoa_atoi"
int main(int argc, char **argv) {
    int i = 0;
    if (argc > 1) {
        i = atoi32(argv[1]);
    }
    path_add("/usr");
    switch (i) {
        case 0: shell("test_command");
        case 1: shell("test_rec");
        case 2: shell("test_fork");
        case 3: shell("test_input");
        case 4: shell("test_resize");
        case 5: shell("test_malloc");
        case 6: shell("test_struct");
        case 7: shell("test_xtoa");
        case 8: shell("test_vector");
        case 9: shell("test_gui");
        case 10: shell("test_map");
        case 11: shell("test_pipe");
        case 12: shell("test_lua");
        case 13: shell("test_mutex");
        case 14: shell("test_semaphore");
        case 15: shell("test_window");
        case 16: shell("test_stack");
        case 17: shell("test_readfile");
        case 18: shell("test_goto");
        case 19: shell("test_bf");
        case 20: shell("test_json");
    }
    return 0;
}
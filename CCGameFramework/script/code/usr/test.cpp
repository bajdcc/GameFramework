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
    }
    return 0;
}
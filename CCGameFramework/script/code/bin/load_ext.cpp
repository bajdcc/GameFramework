#include "/include/io"
#include "/include/fs"
#include "/include/arg"
#include "/include/sys"
int main(int argc, char **argv) {
    if (argc > 1) {
        char* path = arg_string(1, argc, argv);
        int handle = load_ext(path);
        switch (handle) {
            default:
                // put_string("[INFO] Success.");
                break;
            case 1:
                set_fg(240, 0, 0);
                put_string("[ERROR] Ext already loaded.");
                restore_fg();
                break;
            case 2:
                set_fg(240, 0, 0);
                put_string("[ERROR] Ext load error.");
                restore_fg();
                break;
            case 3:
                set_fg(240, 0, 0);
                put_string("[ERROR] Invalid ext name.");
                restore_fg();
                break;
            case 4:
                set_fg(240, 0, 0);
                put_string("[ERROR] Missing load function.");
                restore_fg();
                break;
        }
        free(path);
    }
    return 0;
}
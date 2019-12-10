#include "/include/io"
#include "/include/fs"
#include "/include/arg"
#include "/include/shell"
#include "/include/format"
int link(int handle, char* path, char* to, int show) {
    int ret = mklink(handle, to);
    switch (ret) {
    case -1:
        set_fg(240, 0, 0);
        put_string("[ERROR] File not exists.");
        restore_fg();
        break;
    case -2:
        set_fg(240, 0, 0);
        put_string("[ERROR] Need file type.");
        restore_fg();
        break;
    case -3:
        set_fg(240, 0, 0);
        put_string("[ERROR] Need empty file.");
        restore_fg();
        break;
    }
    close(handle);
    if (show == 1) {
        char* cmd = format("Created link: %s <=> %s", path, to);
        put_string(cmd);
        free(cmd);
    }
}
int main(int argc, char** argv) {
    int show = 1;
    if (argc > 3 && strcmp(argv[3], "hide") == 0) {
        show = 0;
        argc--;
    }
    if (argc > 2) {
        char* path = arg_string(1, 2, argv);
        char* to = arg_string(2, argc, argv);
        char* cmd = format("touch %s", path);
        shell(cmd);
        free(cmd);
        newline();
        int handle = open(path);
        switch (handle) {
        default:
            // put_string("[INFO] Success.");
            link(handle, path, to, show);
            break;
        case -1:
            set_fg(240, 0, 0);
            put_string("[ERROR] File not exists.");
            restore_fg();
            break;
        case -2:
            set_fg(240, 0, 0);
            put_string("[ERROR] Path is not file.");
            restore_fg();
            break;
        case -3:
            set_fg(240, 0, 0);
            put_string("[ERROR] File is locked.");
            restore_fg();
            break;
        }
        free(path);
    }
    return 0;
}
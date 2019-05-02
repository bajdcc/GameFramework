#include "/include/io"
#include "/include/fs"
int main(int argc, char **argv) {
    if (argc == 1) { // rm
        set_fg(240, 0, 0);
        put_string("[Error] Need path.");
        restore_fg();
    } else if (argc == 2) { // rm XX
        switch (rm(argv[1])) {
            case -1:
                set_fg(240, 0, 0);
                put_string("[ERROR] File not exists.");
                restore_fg();
                break;
            case -2:
                set_fg(240, 0, 0);
                put_string("[ERROR] File is locked.");
                restore_fg();
                break;
            case -3:
                set_fg(240, 0, 0);
                put_string("[ERROR] Delete failed.");
                restore_fg();
                break;
        }
    } else {
        set_fg(240, 0, 0);
        put_string("[Error] Invalid argument.");
        restore_fg();
    }
    return 0;
}
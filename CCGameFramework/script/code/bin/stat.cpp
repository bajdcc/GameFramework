#include "/include/io"
#include "/include/fs"
#include "/include/memory"
#include "/include/shell"
#include "/include/string"
int main(int argc, char** argv) {
    char* s = malloc(1024);
    pwd(s);
    if (argc == 2) {
        char* cmd = malloc(1024);
        strcpy(cmd, "cat ");
        strcat(cmd, argv[1]);
        strcat(cmd, ":stat");
        shell(cmd);
        free((int)cmd);
    }
    else {
        set_fg(240, 0, 0);
        put_string("[Error] Invalid argument.");
        restore_fg();
    }
    free((int)s);
    return 0;
}
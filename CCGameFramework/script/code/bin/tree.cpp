#include "/include/io"
#include "/include/fs"
#include "/include/memory"
#include "/include/shell"
#include "/include/string"
int main(int argc, char **argv) {
    if (argc == 1) { // ll
        shell("ls -tree");
    } else if (argc == 2) { // ll XX
        char *cmd = malloc(1024);
        strcpy(cmd, "ls -tree ");
        strcat(cmd, argv[1]);
        shell(cmd);
        free((int) cmd);
    } else {
        set_fg(240, 0, 0);
        put_string("[Error] Invalid argument.");
        restore_fg();
    }
    return 0;
}
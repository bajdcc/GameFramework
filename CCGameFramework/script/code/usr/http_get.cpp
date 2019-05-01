#include "/include/io"
#include "/include/fs"
#include "/include/memory"
#include "/include/shell"
#include "/include/string"
int main(int argc, char **argv) {
    if (argc == 2) { // http_get XX
        char *cmd = malloc(1024);
        strcpy(cmd, "cat /http/");
        strcat(cmd, argv[1]);
        shell(cmd);
        free((int) cmd);
    } else {
        set_fg(240, 0, 0);
        put_string("[Error] Invalid argument. Usage: http_get [URL]");
        restore_fg();
    }
    return 0;
}
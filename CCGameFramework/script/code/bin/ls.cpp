#include "/include/io"
#include "/include/fs"
#include "/include/memory"
#include "/include/shell"
#include "/include/string"
int main(int argc, char **argv) {
    char *s = malloc(1024);
    pwd(s);
    if (argc == 1) { // ls
        char *cmd = malloc(1024);
        strcpy(cmd, "cat ");
        strcat(cmd, s);
        strcat(cmd, ":ls");
        shell(cmd);
        free((int) cmd);
    } else if (argc >= 2 && strcmp(argv[1], "-l") == 0) { // ls -l
        char *cmd = malloc(1024);
        strcpy(cmd, "cat ");
        if (argc > 2)
            strcat(cmd, argv[2]);
        else
            strcat(cmd, s);
        strcat(cmd, ":ll");
        shell(cmd);
        free((int) cmd);
    } else if (argc >= 2 && strcmp(argv[1], "-tree") == 0) { // ls -tree
        char *cmd = malloc(1024);
        strcpy(cmd, "cat ");
        if (argc > 2)
            strcat(cmd, argv[2]);
        else
            strcat(cmd, s);
        strcat(cmd, ":tree");
        shell(cmd);
        free((int) cmd);
    } else if (argc == 2) {
        char *cmd = malloc(1024);
        strcpy(cmd, "cat ");
        strcat(cmd, argv[1]);
        strcat(cmd, ":ls");
        shell(cmd);
        free((int) cmd);
    } else {
        set_fg(240, 0, 0);
        put_string("[Error] Invalid argument.");
        restore_fg();
    }
    free((int) s);
    return 0;
}
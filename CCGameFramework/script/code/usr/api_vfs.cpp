#include "/include/io"
#include "/include/shell"
#include "/include/format"
#include "/include/fs"
void run(char* cmd) {
    // put_string(cmd); put_string("\n");
    shell(cmd);
    free(cmd);
}
int main(int argc, char** argv) {
    if (argc < 2) {
        set_fg(240, 0, 0);
        put_string("[ERROR] Missing argument.");
        restore_fg();
        return 0;
    }
    char* path = format("/ext/fs/func/file%s", argv[2]);
    if (strcmp("read", argv[1]) == 0) {
        if (fsize(path) > 0) {
            run(format("cat %s", path));
        }
        else {
            run(format("touch %s", path));
            run(format("cat %s:fs:load", path));
        }
    }
    else if (strcmp("load", argv[1]) == 0) {
        if (fsize(path) < 0) {
            run(format("touch %s", path));
        }
        run(format("cat %s:fs:load:no_output", path));
        if (fsize(path) > 0) {
            run(format("echo %s | copy %s", path, argv[2]));
        }
        run(format("rm %s", path));
    }
    else if (strcmp("write", argv[1]) == 0) {
        if (fsize(argv[2]) >= 0) {
            run(format("echo %s | copy %s", argv[2], path));
            run(format("cat %s:fs:save", path));
        }
    }
    else if (strcmp("save", argv[1]) == 0) {
        if (fsize(argv[2]) >= 0) {
            run(format("echo %s | copy %s", argv[2], path));
            run(format("cat %s:fs:save:no_output", path));
        }
    }
    else if (strcmp("mkdir", argv[1]) == 0) {
        run(format("mkdir %s", argv[1]));
        run(format("mkdir %s", path));
        run(format("cat %s:fs:save", path));
    }
    else {
        set_fg(240, 0, 0);
        put_string("[ERROR] Invalid argument.");
        restore_fg();
    }
    free(path);
    return 0;
}
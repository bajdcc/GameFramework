#include "/include/io"
#include "/include/shell"
#include "/include/format"
int main(int argc, char** argv) {
    if (argc < 3) {
        set_fg(240, 0, 0);
        put_string("[ERROR] Missing argument.");
        restore_fg();
        return 0;
    }
    char* buf = "";
    int i;
    for (i = 2; i < argc; i++) {
        char* old_buf = buf;
        buf = format("%s:%s", buf, argv[i]);
        free(old_buf);
    }
    char* fmt = format("ipc ext_%s file/__call__%s", argv[1], buf);
    free(buf);
    shell(fmt);
    free(fmt);
    return 0;
}
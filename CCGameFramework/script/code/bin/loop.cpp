#include "/include/shell"
#include "/include/proc"
#include "/include/arg"
int main(int argc, char **argv) {
    if (argc <= 1) {
        set_fg(240, 0, 0);
        put_string("[Error] Missing argument.");
        restore_fg();
        return;
    }
    for (;;) {
        shell(arg_string(1, argc, argv));
        sleep(500);
    }
    return 0;
}
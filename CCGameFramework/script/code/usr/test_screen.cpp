#include "/include/io"
#include "/include/shell"
#include "/include/format"
void print(char* s) {
    put_string(s);
    free(s);
}
// WELCOME
int welcome(int n) {
    put_string("\f");
    set_fg(0, 240, 240);
    set_bg(10, 10, 10);
    shell("cat /usr/logo.txt");
    restore_fg();
    restore_bg();
    put_string("\n\n");
    set_fg(160, 160, 160);
    put_string("Welcome to @clibos system by bajdcc!\n");
    print(format("欢迎来到脚本操作系统！现在是屏幕（%d）\n\n", n));
    put_string("# Type \"help\" for help.\n\n");
    restore_fg();
}
int main(int argc, char **argv) {
    put_string("========== [#27 TEST SCREEN] ==========\n");
    if (fork() != -1) {
        wait();
    }
    else {
        int i = 1, r = 0;
        for (;;) {
            r = switch_screen(i);
            if (r <= 1) {
                break;
            }
            i++;
            if (r == 2) {
                continue;
            }
        }
        if (r >= 1) {
            set_fg(240, 0, 0);
            put_string("Not enough screen.\n");
            restore_fg();
        }
        else if (r == 0) {
            welcome(i);
            shell("sh");
        }
        exit(0);
    }
    put_string("========== [#27 TEST SCREEN] ==========\n");
    return 0;
}
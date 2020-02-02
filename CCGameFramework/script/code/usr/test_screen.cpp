#include "/include/io"
#include "/include/shell"
int main(int argc, char **argv) {
    put_string("========== [#27 TEST SCREEN] ==========\n");
    if (fork() != -1) {
        wait();
    }
    else {
        switch_screen(1);
        shell("sh");
        exit(0);
    }
    put_string("========== [#27 TEST SCREEN] ==========\n");
    return 0;
}
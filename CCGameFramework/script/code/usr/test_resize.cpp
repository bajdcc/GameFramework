#include "/include/io"
#include "/include/proc"
int main(int argc, char **argv) {
    int i;
    put_string("========== [#4 TEST RESIZE] ==========\n");
    put_string("Command:");
    for (i = 0; i < argc; ++i) {
        put_string(" ");
        put_string(argv[i]);
    }
    put_string("\n");
    resize(30, 120);
    sleep(1000);
    resize(20, 20);
    sleep(1000);
    resize(30, 84);
    put_string("========== [#4 TEST RESIZE] ==========\n");
    return 0;
}
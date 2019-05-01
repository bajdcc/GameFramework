#include "/include/io"
#include "/include/proc"
int main(int argc, char **argv) {
    int i;
    put_string("========== [#2 TEST FORK] ==========\n");
    put_string("Command:");
    for (i = 0; i < argc; ++i) {
        put_string(" ");
        put_string(argv[i]);
    }
    put_string("\n");
    i = fork();
    if (i == -1) {
        put_string("Child: fork return "); put_int(i); put_string("\n");
    } else {
        wait();
        sleep(500);
        put_string("Parent: fork return "); put_int(i); put_string("\n");
        put_string("========== [#2 TEST FORK] ==========\n");
    }
    return 0;
}
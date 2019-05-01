#include "/include/io"
#include "/include/memory"
#include "/include/proc"
#include "/include/string"
int waiting(int second) {
    while (second > 0) {
        put_string("Waiting... ");
        put_int(second--);
        put_string("s");
        put_string("\r");
        sleep(1000);
    }
    put_string("\n");
}
int main(int argc, char **argv) {
    int i;
    put_string("========== [#3 TEST INPUT] ==========\n");
    put_string("Command:");
    for (i = 0; i < argc; ++i) {
        put_string(" ");
        put_string(argv[i]);
    }
    put_string("\n");
    put_string("Input: ");
    char *text = malloc(100);
    *text = 0;
    input((char *) text, 100);
    put_string("Output: ");
    put_string((char *) text);
    put_string("\n");
    put_string("Length: ");
    put_int(strlen((char *) text));
    put_string("\n");
    waiting(1);
    return 0;
}
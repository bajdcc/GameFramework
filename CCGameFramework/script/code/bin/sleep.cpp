#include "/include/proc"
#include "/include/xtoa_atoi"
#include "/include/string"
#include "/include/io"
int waiting(int second) {
    while (second > 0) {
        put_string("Waiting... ");
        put_int(second--);
        put_string("s");
        put_string("    \r");
        sleep(1000);
    }
    put_string("\n");
}
int main(int argc, char **argv) {
    int i = 0, show = 0;
    if (argc > 1) {
        i = atoi32(argv[1]);
    }
    if (i == 0) {
        return 0;
    }
    if (argc > 2) {
        if (strcmp(argv[2], "show") == 0) {
            waiting(i);
            return 0;
        }
    }
    sleep(i * 1000);
    return 0;
}
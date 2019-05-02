#include "/include/io"
#include "/include/xtoa_itoa"
#include "/include/xtoa_atoi"
int main(int argc, char **argv) {
    if (argc == 3) {
        long start = atoi64(argv[1]);
        long end = atoi64(argv[2]);
        long i;
        char text[128];
        if (start > end) {
            for (i = start; i >= end; --i) {
                i64toa(i, (char *) &text);
                put_string((char *) &text);
                put_string("\n");
            }
        } else {
            for (i = start; i <= end; ++i) {
                i64toa(i, (char *) &text);
                put_string((char *) &text);
                put_string("\n");
            }
        }
    } else {
        set_fg(240, 0, 0);
        put_string("[Error] Invalid argument.");
        restore_fg();
    }
    return 0;
}
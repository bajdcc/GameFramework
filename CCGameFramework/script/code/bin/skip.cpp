#include "/include/io"
#include "/include/shell"
#include "/include/xtoa_atoi"
void skip(int n) {
    int c, i = 1, cmd = 0, start = 0;
    input_lock();
    while ((c = input_valid()) != -1) {
        c = input_char();
        if (start) {
            put_char(c);
        }
        else if (((char) c) == '\033') {
            cmd = 1 - cmd;
        }
        else if (cmd == 0) {
            if (((char)c) == '\n') {
                if (i >= n) {
                    start = 1;
                }
                else {
                    i++;
                }
            }
        }
    }
    input_unlock();
}
int main(int argc, char **argv) {
    if (argc == 1) { // skip
        shell("pipe");
    } else if (argc == 2) { // skip XX
        int n = atoi32(argv[1]);
        if (n <= 0) {
            shell("pipe");
            return 0;
        }
        skip(n);
    } else {
        set_fg(240, 0, 0);
        put_string("[Error] Invalid argument.");
        restore_fg();
    }
    return 0;
}
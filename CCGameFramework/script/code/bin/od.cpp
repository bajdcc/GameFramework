#include "/include/io"
char *hex = "0123456789ABCDEF";
int pipe_x() {
    int c;
    input_lock();
    while ((c = input_valid()) != -1) {
        c = input_char();
        put_char(hex[c >> 4]);
        put_char(hex[c & 0xF]);
    }
    input_unlock();
    return 0;
}
int main(int argc, char **argv) {
    pipe_x();
    return 0;
}
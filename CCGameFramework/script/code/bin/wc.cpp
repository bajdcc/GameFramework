#include "/include/io"
int wc(int *lines, int *chars) {
    int c;
    input_lock();
    while ((c = input_valid()) != -1) {
        c = input_char();
        (*chars)++;
        if (((char) c) == '\n')
            (*lines)++;
    }
    input_unlock();
    return 0;
}
int main(int argc, char **argv) {
    int lines = 0;
    int chars = 0;
    wc(&lines, &chars);
    put_int(lines);
    put_string(" ");
    put_int(chars);
    return 0;
}
#include "/include/io"
int pipe() {
    int c;
    input_lock();
    while ((c = input_valid()) != -1) {
        put_char((char) input_char());
    }
    input_unlock();
    return 0;
}
int main(int argc, char **argv) {
    pipe();
    return 0;
}
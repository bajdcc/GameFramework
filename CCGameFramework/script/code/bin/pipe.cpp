#include "/include/io"
int pipe() {
    int c;
    input_lock();
    while ((c = input_char()) != -1) {
        put_char((char) c);
    }
    input_unlock();
    return 0;
}
int main(int argc, char **argv) {
    pipe();
    return 0;
}
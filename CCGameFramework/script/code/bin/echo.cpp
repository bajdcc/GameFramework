#include "/include/io"
int main(int argc, char **argv) {
    int i;
    for (i = 1; i < argc; ++i) {
        put_string(argv[i]);
        if (i < argc - 1)
            put_string(" ");
    }
    return 0;
}
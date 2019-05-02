#include "/include/io"
#include "/include/fs"
#include "/include/memory"
int main(int argc, char **argv) {
    char *s = malloc(1024);
    pwd(s);
    put_string(s);
    free((int) s);
    return 0;
}
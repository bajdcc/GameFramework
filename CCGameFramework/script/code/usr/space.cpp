#include "/include/io"
#include "/include/xtoa_atoi"
int main(int argc, char** argv) {
    if (argc != 2) {
        return 0;
    }
    int n = atoi32(argv[1]), i;
    for (i = 0; i < n; i++)
        put_char(' ');
    return 0;
}
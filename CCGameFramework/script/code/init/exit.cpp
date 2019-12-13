#include "/include/shell"
int main(int argc, char** argv) {
    path_add("/init");
    shell("ps | skip 1 | grep /sys/ | col 3 | signal 9");
    return 0;
}
#include "/include/shell"
int main(int argc, char** argv) {
    path_add("/init");
    shell("ps | skip 1 | grep /ipc/ | col 3 | signal 9");
    shell("ps | skip 1 | grep /init/ | col 3 | signal 9");
    return 0;
}
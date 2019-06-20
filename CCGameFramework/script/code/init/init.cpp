#include "/include/shell"
int main(int argc, char** argv) {
    path_add("/init");
    shell("cat /init/init.txt | bat async");
    return 0;
}
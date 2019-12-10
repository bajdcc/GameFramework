#include "/include/exec"
#include "/include/shell"
#include "/include/format"
int main(int argc, char** argv) {
    path_add("/ipc");
    shell("cat ipc.txt | bat async");
    return 0;
}
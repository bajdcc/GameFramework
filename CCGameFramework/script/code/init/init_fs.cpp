#include "/include/shell"
int main(int argc, char** argv) {
    shell("mkdir tmp");
    shell("mkdir ipc");
    shell("mkdir bat");
    shell("/ipc/ipc_start");
    return 0;
}
#include "/include/shell"
int main(int argc, char** argv) {
    int i;
    for (i = 0; i < 2; i++) {
        shell("ipc sys sleep 1");
        shell("ps | skip 1 | grep /init/startup | col 3 | signal 9");
    }
    shell("touch /pipe/sys_entry_shell_start");
    shell("echo Boot complete > /pipe/sys_entry_shell_start");
    return 0;
}
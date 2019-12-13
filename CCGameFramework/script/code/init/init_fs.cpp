#include "/include/shell"
int main(int argc, char** argv) {
    shell("mkdir tmp");
    shell("mkdir ipc");
    shell("mkdir bat");
    shell("echo [*] 启动IPC服务！ > /fifo/sys_entry_console");
    shell("/ipc/ipc_start");
    return 0;
}
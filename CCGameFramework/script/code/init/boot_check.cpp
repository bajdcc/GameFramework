#include "/include/shell"
int main(int argc, char** argv) {
    int j = fork();
    if (j == -1) {
        shell("touch /pipe/sys_entry_shell_start");
        int handle = open("/pipe/sys_entry_shell_start");
        if (handle >= 0) {
            while (recv_signal() != 9) {
                shell("cat /fifo/sys_entry_console >> /pipe/sys_entry_shell_start");
                shell("/usr/newline >> /pipe/sys_entry_shell_start");
            }
            close(handle);
        }
        exit(0);
    }
    sleep(3000);
    send_signal(j, 9);
    wait();
    shell("ps | skip 1 | grep /init/startup | col 3 | signal 9");
    shell("echo Boot complete > /pipe/sys_entry_shell_start");
    return 0;
}
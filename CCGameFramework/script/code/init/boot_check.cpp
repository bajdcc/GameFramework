#include "/include/shell"
#include "/include/readfile"
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
            shell("echo Boot complete >> /pipe/sys_entry_shell_start");
            shell("/usr/newline >> /pipe/sys_entry_shell_start");
            shell("echo Boot ok > /pipe/sys_entry_shell_complete");
            close(handle);
        }
        exit(0);
    }
    if (*readfile_fast("/dev/debug") == '1')
        sleep(8000);
    else
        sleep(3000);
    send_signal(j, 9);
    wait();
    return 0;
}
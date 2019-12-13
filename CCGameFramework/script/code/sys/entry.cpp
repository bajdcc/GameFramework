#include "/include/io"
#include "/include/fs"
#include "/include/shell"
// WELCOME
int welcome() {
    put_string("\f");
    set_fg(0, 240, 240);
    set_bg(10, 10, 10);
    shell("cat /usr/logo.txt");
    restore_fg();
    restore_bg();
    put_string("\n\n");
    set_fg(160, 160, 160);
    put_string("Welcome to @clibos system by bajdcc!\n");
    put_string("欢迎来到脚本操作系统！\n\n");
    put_string("# Type \"help\" for help.\n\n");
    restore_fg();
}
int main(int argc, char** argv) {
    put_string("\f");
    put_string("Starting OS...\n");
    path_add("/bin");
    put_string("Starting service...\n");
    shell("touch /fifo/sys_entry_console");
    exec_service("/init/init");
    int i;
    newline();
    for (i = 0;; i++) {
        int handle = open("/pipe/sys_entry_shell_start");
        if (handle >= 0) {
            close(handle);
            shell("cat /pipe/sys_entry_shell_start");
            handle = open("/pipe/sys_entry_shell_complete");
            if (handle >= 0) {
                close(handle);
                shell("cat /pipe/sys_entry_shell_complete");
                shell("rm /pipe/sys_entry_shell_complete");
                break;
            }
        }
        sleep(100);
    }
    shell("ps | skip 1 | grep /init/startup | col 3 | signal 9");
    shell("rm /fifo/sys_entry_console");
    sleep(1000);
    welcome();
    exec("sh"); wait();
    exec("/init/exit"); wait();
    return 0;
}
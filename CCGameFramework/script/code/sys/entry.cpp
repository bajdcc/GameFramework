#include "/include/io"
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
    shell("touch ");
    exec_service("/init/init");
    int i;
    for (i = 0;; i++) {
        if (fempty("/pipe/sys_entry_shell_start") > 0) {
            newline();
            shell("cat /pipe/sys_entry_shell_start");
            shell("rm /pipe/sys_entry_shell_start");
            sleep(100);
            break;
        }
        sleep(200);
        put_string("Waiting... ");
        put_int(i / 5); put_string("\r");
    }
    welcome();
    exec("sh"); wait();
    exec("/init/exit"); wait();
    return 0;
}
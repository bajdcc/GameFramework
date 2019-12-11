#include "/include/shell"
void run(char* cmd) {
    put_string("# "); put_string(cmd); put_char('\n');
    shell(cmd);
    shell("ipc sys sleep 1");
}
int main(int argc, char **argv) {
    int i;
    put_string("========== [#25 TEST IPC] ==========\n");
    run("ipc api ip all");
    run("ipc sys cat /sys/time");
    run("ipc sys ipc sys cat /sys/time");
    put_string("========== [#25 TEST IPC] ==========\n");
    return 0;
}
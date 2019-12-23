#include "/include/shell"
void run(char* cmd) {
    put_string("# "); put_string(cmd); put_char('\n');
    shell(cmd);
    shell("ipc sys sleep 1");
}
int main(int argc, char **argv) {
    int i;
    put_string("========== [#25 TEST IPC] ==========\n");
    for (i = 0; i < 10; i++) {
        if (fork() == -1) {
            shell("ipc sys sleep 1");
            exit(0);
        }
    }
    for (i = 0; i < 10; i++) {
        wait();
    }
    run("ipc api ip all");
    run("ipc api hitokoto");
    run("ipc api lyric 111111");
    run("ipc sys cat /sys/time");
    run("ipc sys ipc sys cat /sys/time");
    put_string("========== [#25 TEST IPC] ==========\n");
    return 0;
}
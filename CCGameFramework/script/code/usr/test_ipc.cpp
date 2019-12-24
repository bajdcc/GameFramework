#include "/include/shell"
#include "/include/format"
void run(char* cmd) {
    put_string("# "); put_string(cmd); put_char('\n');
    shell(cmd);
    shell("ipc sys sleep 1");
}
void test_sleep(int n) {
    int i;
    for (i = 0; i < n; i++) {
        if (fork() == -1) {
            run(format("ipc sys sleep %d", i + 2));
            exit(0);
        }
    }
    for (i = 0; i < n; i++) {
        wait();
    }
}
void test_ip(int n) {
    int i;
    for (i = 0; i < n; i++) {
        if (fork() == -1) {
            run("ipc api hitokoto");
            exit(0);
        }
    }
    for (i = 0; i < n; i++) {
        wait();
    }
}
int main(int argc, char **argv) {
    put_string("========== [#25 TEST IPC] ==========\n");
    run("ipc sys whoami");
    test_sleep(10);
    test_ip(10);
    run("ipc api ip all");
    run("ipc api hitokoto");
    run("ipc api lyric 111111");
    run("ipc sys cat /sys/time");
    run("ipc sys ipc sys cat /sys/time");
    put_string("========== [#25 TEST IPC] ==========\n");
    return 0;
}
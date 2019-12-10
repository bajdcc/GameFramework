#include "/include/io"
#include "/include/proc"
#include "/include/shell"
#include "/include/format"
int main(int argc, char **argv) {
    put_string("========== [#24 TEST FIFO] ==========\n");
    int i;
    shell("touch /fifo/__test_fifo__");
    shell("mklink /tmp/__test_FIFO__ /fifo/__test_fifo__");
    newline();
    if (fork() != -1) {
        // IPC SERVICE
        for (i = 0; i < 10; i++) {
            char* fmt = format("echo %d hello world, fifo! > /tmp/__test_fifo_file__", i);
            shell(fmt);
            shell("newline >> /tmp/__test_fifo_file__");
            shell("cat /tmp/__test_fifo_file__ > /tmp/__test_FIFO__");
            free(fmt);
        }
        sleep(1000);
    }
    else {
        // IPC CLIENT
        for (i = 0; i < 10; i++) {
            if (fork() == -1) {
                shell("cat /tmp/__test_FIFO__");
                exit(0);
            }
        }
        exit(0);
    }
    shell("rm /tmp/__test_FIFO__");
    shell("rm /fifo/__test_fifo__");
    shell("rm /tmp/__test_fifo_file__");
    put_string("========== [#24 TEST FIFO] ==========\n");
    return 0;
}
//
// Project: clibparser
// Created by bajdcc
//

#include "/include/exec"
#include "/include/proc"
#include "/include/io"

int intern_pipe() {
    int c;
    input_lock();
    while ((c = input_valid()) != -1) {
        put_char((char) input_char());
    }
    input_unlock();
    return 0;
}
int shell(char *path) {
    int pid = exec_sleep(path);
    if (pid < 0)
        return pid;
    exec_connect(pid, get_pid());
    exec_wakeup(pid);
    intern_pipe();
    wait();
}

//
// Project: clibparser
// Created by bajdcc
//

// 进程控制
int sleep(int ms) {
    ms;
    interrupt 100;
}
int wait() {
    interrupt 52;
}
int fork() {
    interrupt 55;
}
int get_pid() {
    0;
    interrupt 50;
}
int get_parent() {
    1;
    interrupt 50;
}
int switch_task() {
    interrupt 58;
}
int exit(int n) {
    n;
    interrupt 40;
}
int send_signal(int pid, int signal) {
    pid << 16 | signal;
    interrupt 42;
}
int recv_signal() {
    interrupt 43;
}

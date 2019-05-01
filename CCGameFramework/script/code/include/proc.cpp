//
// Project: clibparser
// Created by bajdcc
//

// 进程控制
int sleep(int ms) {
    ms;
    interrupt 100;
    interrupt 101;
}
int wait() {
    interrupt 52;
}
int fork() {
    interrupt 55;
}
int get_pid() {
    interrupt 50;
}
int switch_task() {
    interrupt 58;
}
int exit(int n) {
    n;
    interrupt 40;
}

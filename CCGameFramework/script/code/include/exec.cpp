//
// Project: clibparser
// Created by bajdcc
//

#include "/include/string"

// 启动进程
int exec(char *path) {
    path;
    interrupt 51;
}
int exec_sleep(char *path) {
    path;
    interrupt 53;
}
int exec_wakeup(int pid) {
    pid;
    interrupt 54;
}
int exec_connect(int left, int right) {
    (left << 16) | right;
    interrupt 56;
}
int exec_kill_children() {
    interrupt 57;
}

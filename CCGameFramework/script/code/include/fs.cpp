//
// Project: clibparser
// Created by bajdcc
//

// 文件系统

int pwd(char *s) {
    s;
    interrupt 60;
}
int whoami(char *s) {
    s;
    interrupt 61;
}
int cd(char *s) {
    s;
    interrupt 62;
}
int mkdir(char *s) {
    s;
    interrupt 63;
}
int touch(char *s) {
    s;
    interrupt 64;
}
int open(char *s) {
    s;
    interrupt 65;
}
int read(int handle) {
    handle;
    interrupt 66;
}
int close(int handle) {
    handle;
    interrupt 67;
}
int rm(char *s) {
    s;
    interrupt 68;
}
int write(int handle, char c) {
    handle << 16 | (c + 0x1000);
    interrupt 69;
}
int truncate(int handle) {
    handle;
    interrupt 70;
}
int exists(char*s) {
    int fd = open(s);
    if (fd < 0)return 0;
    close(fd);
    return 1;
}
struct __copy_struct__ {
    int from, to;
};
int copy(int from, int to) {
    __copy_struct__ s;
    s.from = from;
    s.to = to;
    &s;
    interrupt 74;
}
int load(int handle) {
    handle;
    interrupt 75;
}
int fsize(char* path) { // ERR:<0  OK:>=0
    int handle = open(path);
    if (handle >= 0) {
        int n = 0; int c;
        while (c = read(handle), c < 0x1000) {
            n++;
        }
        close(handle);
        return n;
    }
    else {
        return handle;
    }
}
int fempty(char* path) { // ERR:<0  YES:1  NO:0
    int handle = open(path);
    if (handle >= 0) {
        int val = read(handle) < 0x1000 ? 0 : 1;
        close(handle);
        return val;
    }
    else {
        return handle;
    }
}
//
// Project: clibparser
// Created by bajdcc
//

#include "/include/exec"
#include "/include/proc"
#include "/include/fs"
#include "/include/io"
#include "/include/string"
#include "/include/memory"

int process(char* text) {
    char* tmp = malloc(256), c;
    int i = 0, j = 0;
    while (true) {
        c = text[i];
        if (c == '>') {
            i++;
            if (text[i] == '>') { // append
                i++;
                strcpy(tmp + j, "| append ");
                j += 9;
            }
            else { // truncate
                strcpy(tmp + j, "| write ");
                j += 8;
            }
        }
        tmp[j++] = text[i++];
        if (c == (char)0)
            break;
    }
    strcpy(text, tmp);
}
int exec_single(char* text, int* total, int right) {
    int pid;
    while (*text == ' ')
        text++;
    if (strncmp(text, "/sys/", 5) == 0) {
        return -3;
    }
    if (strncmp(text, "/", 1) != 0) {
        char* path = malloc(200);
        pwd(path);
        if (strlen(path) != 1)
            strcat(path, "/");
        strcat(path, text);
        pid = exec_sleep(path);
        free(path);
        if (pid >= 0) {
            exec_connect(pid, get_pid());
            (*total)++;
            return pid;
        }
    }
    (*total)++;
    pid = exec_sleep(text);
    exec_connect(pid, get_pid());
    return pid;
}
int exec_start(char* text, int* total) {
    char* c = strchr(text, '|');
    if (c == (char*)0) {
        return exec_single(text, total, 1);
    }
    else {
        *c++ = '\0';
        if (*c == '\0')
            return exec_single(text, total, 1);
        int right = exec_start(c, total);
        if (right < 0)
            return right;
        int left = exec_single(text, total, 0);
        exec_connect(left, right);
        exec_wakeup(right);
        return left;
    }
}
void output() {
    int i, c;
    int state = input_lock();
    while ((c = input_valid()) != -1 && c > INPUT_BEGIN) {
        put_char(input_char());
    }
    input_unlock();
}
int shell(char *path) {
    int len = strlen(path), total = 0, i;
    char* new_path = malloc(len + 1);
    strcpy(new_path, path);
    process(new_path);
    int pid = exec_start(new_path, &total);
    free(new_path);
    if (pid >= 0) {
        exec_wakeup(pid);
        output();
        for (i = 0; i < total; ++i) {
            wait();
        }
        return 0;
    }
    else {
        exec_kill_children();
        return pid;
    }
}

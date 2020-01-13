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
#include "/include/vector"

int process(char* text) {
    char* tmp = malloc(256), c;
    int i = 0, j = 0;
    while (true) {
        c = text[i];
        if (c == '>') {
            i++;
            if (text[i] == '>') { // append
                i++;
                strcpy(tmp + j, "| append");
                j += 8;
                if (text[i] != ' ')
                    tmp[j++] = ' ';
            }
            else { // truncate
                strcpy(tmp + j, "| write");
                j += 7;
                if (text[i] != ' ')
                    tmp[j++] = ' ';
            }
        }
        tmp[j++] = text[i++];
        if (c == (char)0)
            break;
    }
    strcpy(text, tmp);
    free(tmp);
}
int exec_single(char* text, int* total, void* arr) {
    int pid;
    while (*text == ' ')
        text++;
    if (strncmp(text, "/sys/", 5) == 0) {
        return -3;
    }
    (*total)++;
    pid = exec_sleep(text);
    vector_push(arr, &pid);
    exec_connect(pid, get_pid());
    return pid;
}
int exec_start(char* text, int* total, void* arr) {
    char* c = strchr(text, '|');
    if (c == (char*)0) {
        return exec_single(text, total, arr);
    }
    else {
        *c++ = '\0';
        if (*c == '\0')
            return exec_single(text, total, arr);
        int right = exec_start(c, total, arr);
        if (right < 0)
            return right;
        int left = exec_single(text, total, arr);
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
int shell(char* path) {
    int len = strlen(path), total = 0, i;
    char* new_path = malloc(256);
    strcpy(new_path, path);
    process(new_path);
    void* arr = vector_new(sizeof(int));
    int pid = exec_start(new_path, &total, arr);
    free(new_path);
    if (pid >= 0) {
        exec_wakeup(pid);
        output();
        for (i = 0; i < total; ++i) {
            wait();
        }
        vector_del(arr);
        return 0;
    }
    else {
        for (i = 0; i < total; ++i) {
            int p = *(int*)vector_get(arr, i);
            if (p >= 0)
                send_signal(pid, 99);
        }
        vector_del(arr);
        return pid;
    }
}

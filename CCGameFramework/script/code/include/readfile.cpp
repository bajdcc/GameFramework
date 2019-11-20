//
// Project: clibparser
// Created by bajdcc
//

#include "/include/fs"
#include "/include/memory"

struct __intern_string__ {
    char* text;
    int capacity;
    int length;
};
__intern_string__ __intern_new_string__(int n) {
    __intern_string__ s;
    s.text = malloc(n);
    s.capacity = n;
    s.length = 0;
    return s;
}
void __intern_append_byte__(__intern_string__* s, char c) {
    if (s->length >= s->capacity - 1) {
        s->capacity <<= 1;
        char* new_text = malloc(s->capacity);
        memmove(new_text, s->text, s->length);
        free(s->text);
        s->text = new_text;
    }
    (s->text)[s->length++] = c;
}

int near2(int n) {
    int s = 16;
    if (n >= s) {
        n |= (n >> 1);
        n |= (n >> 2);
        n |= (n >> 4);
        n |= (n >> 8);
        n |= (n >> 16);
        n++;
        if (n < 0)
            n >>= 1;
        return n;
    }
    else {
        return s;
    }
}

void __intern_read_file__(int handle, char** out, int* len) {
    int cache = near2(flen(handle) + 1);
    __intern_string__ s = __intern_new_string__(cache);
    int n = 0; int c;
    while (c = read(handle), c < 0x1000) {
        __intern_append_byte__(&s, (char)c);
        n++;
    }
    __intern_append_byte__(&s, (char)0);
    close(handle);
    *out = s.text;
    *len = n;
}

// 读取文件（成功返回零，失败为负）
int readfile(char* path, char** out, int* len) {
    int handle = open(path);
    if (handle >= 0) {
        load(handle);
        __intern_read_file__(handle, out, len);
        return 0;
    }
    return handle;
}

char* readfile_fast(char* path) {
    int handle = open(path);
    if (handle >= 0) {
        load(handle);
        char* data = fread(handle);
        close(handle);
        return data;
    }
    return (char*)0;
}

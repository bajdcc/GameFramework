//
// Project: clibparser
// Created by bajdcc
//

#include "/include/fs"
#include "/include/memory"

int __intern_write_file__(int handle, char* _input, int len) {
    int i, r = 0;
    for (i = 0; i < len; i++) {
        r = write(handle, _input[i]);
        if (r < 0) { break; }
    }
    close(handle);
    return r;
}

// 写入文件（成功返回零，失败为负）
int writefile(char* path, char* _input, int len, int trun) {
    touch(path);
    int handle = open(path);
    if (handle >= 0) {
        if (trun) truncate(handle);
        return __intern_write_file__(handle, _input, len);
    }
    else {
        return handle;
    }
}

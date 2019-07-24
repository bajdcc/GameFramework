#include "/include/io"
#include "/include/readfile"
#include "/include/sys"
void test_read(char* path) {
    put_string("Read file: ");
    put_string(path);
    char* out;
    int n;
    put_string(" waiting...");
    long now = timestamp();
    if (readfile(path, &out, &n) == 0) {
        put_string("\b\b\b\b\b\b\b\b\b\b\b");
        put_string(", ptr= ");
        put_hex(out);
        put_string(", len= ");
        put_int(n);
        put_string(", time= ");
        put_long((timestamp() - now) / 1000);
        put_string("ms.\n");
        free(out);
    }
    else {
        put_string("\b\b\b\b\b\b\b\b\b\b\b");
        put_string(" failed.\n");
    }
}
int main(int argc, char** argv) {
    put_string("========== [#17 TEST READFILE] ==========\n");
    test_read("/usr/file-dont-exists");
    test_read("/proc/0/dep");
    test_read("/http/www.bing.cn");
    test_read("/http/www.baidu.com");
    test_read("/http/www.gitee.com");
    test_read("/http/www.github.com");
    test_read("/http/www.stackoverflow.com");
    test_read("/http/www.csdn.net");
    test_read("/http/www.codewars.com");
    put_string("========== [#17 TEST READFILE] ==========\n");
    return 0;
}
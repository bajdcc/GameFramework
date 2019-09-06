#include "/include/io"
#include "/include/format"
void print(char* s) {
    put_string(s);
    free(s);
}
void test() {
    print(format("1 2 %l %d %c %f %% %D %D %l %L %s\n",
        3L, -4, '5', 6.0, -1, 0XFFFFFFFF, 100000000000L, 0xFFFFFFFFF, "hello"));
}
int main(int argc, char** argv) {
    put_string("========== [#23 TEST FORMAT] ==========\n");
    test();
    put_string("========== [#23 TEST FORMAT] ==========\n");
    return 0;
}
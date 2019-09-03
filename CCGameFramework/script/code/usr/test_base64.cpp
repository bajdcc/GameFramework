#include "/include/io"
#include "/include/memory"
#include "/include/base64"
#include "/include/readfile"
#include "/include/shell"
void test() {
    char* data; int L;
    if (readfile("/usr/logo.txt", &data, &L) == 0) {
        char* m = malloc(L * 2); int l;
        if (base64_encode(data, L, m, &l) != 0) {
            set_fg(240, 0, 0);
            put_string("[ERROR] Encode error.\n");
            restore_fg();
            return;
        }
        m[l] = 0C;
        put_string(m);
        newline();
        char* m2 = malloc(L + 1); int l2;
        if (base64_decode(m, l, m2, &l2) != 0) {
            set_fg(240, 0, 0);
            put_string("[ERROR] Decode error.\n");
            restore_fg();
            return;
        }
        m2[l2] = 0C;
        put_string(m2);
        newline();
    }
    else {
        set_fg(240, 0, 0);
        put_string("[ERROR] Read error.\n");
        restore_fg();
    }
}
void test_shell() {
    shell("cat /usr/logo.txt | base64_encode | base64_decode");
    newline();
}
int main(int argc, char **argv) {
    put_string("========== [#22 TEST BASE64] ==========\n");
    test();
    test_shell();
    put_string("========== [#22 TEST BASE64] ==========\n");
    return 0;
}
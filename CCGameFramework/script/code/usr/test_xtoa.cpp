#include "/include/io"
#include "/include/memory"
#include "/include/string"
#include "/include/xtoa_itoa"
#include "/include/xtoa_dtoa"
#include "/include/proc"
int waiting(int second) {
    while (second > 0) {
        put_string("Waiting... ");
        put_int(second--);
        put_string("s");
        put_string("\r");
        sleep(1000);
    }
    put_string("\n");
}
int case_1() {
    put_string("-- CASE #1 [i32toa] --\n");
    char *str = malloc(256);
    int i, a = 1;
    for (i = 0; i < 10; ++i, a *= 10) {
        put_string("Input: ");
        put_int(--a);
        put_string(", Output: ");
        i32toa(a++, str);
        put_string(str);
        put_string("\n");
    }
    for (i = 0, a = -1; i < 10; ++i, a *= 10) {
        put_string("Input: ");
        put_int(++a);
        put_string(", Output: ");
        i32toa(a--, str);
        put_string(str);
        put_string("\n");
    }
}
int case_2() {
    put_string("-- CASE #2 [u32toa] --\n");
    char *str = malloc(256);
    int i;
    unsigned a = 1U;
    for (i = 0; i < 10; ++i, a *= 10U) {
        put_string("Input: ");
        put_int(--a);
        put_string(", Output: ");
        u32toa(a++, str);
        put_string(str);
        put_string("\n");
    }
}
int case_3() {
    put_string("-- CASE #3 [i64toa] --\n");
    char *str = malloc(256);
    int i;
    long a = 1L;
    for (i = 0; i < 19; ++i, a *= 10L) {
        put_string("Input: ");
        put_long(--a);
        put_string(", Output: ");
        i64toa(a++, str);
        put_string(str);
        put_string("\n");
    }
    waiting(1);
    for (i = 0, a = -1L; i < 19; ++i, a *= 10L) {
        put_string("Input: ");
        put_long(++a);
        put_string(", Output: ");
        i64toa(a--, str);
        put_string(str);
        put_string("\n");
    }
}
int case_4() {
    put_string("-- CASE #4 [u64toa] --\n");
    char *str = malloc(256);
    int i;
    unsigned long a = 1UL;
    for (i = 0; i < 19; ++i, a *= 10UL) {
        put_string("Input: ");
        put_long(--a);
        put_string(", Output: ");
        u64toa(a++, str);
        put_string(str);
        put_string("\n");
    }
}
int case_5() {
    put_string("-- CASE #5 [dtoa] --\n");
    char *str = malloc(256);
    int i;
    double a = 1.1;
    for (i = 0; i < 17; ++i, a *= a) {
        put_string("Input: ");
        put_double(a);
        put_string(", Output: ");
        xtoa_dtoa(a, str);
        put_string(str);
        put_string("\n");
    }
    waiting(1);
    for (i = 0, a = 1.1; i < 17; ++i, a *= a) {
        put_string("Input: ");
        put_double(-a);
        put_string(", Output: ");
        xtoa_dtoa(-a, str);
        put_string(str);
        put_string("\n");
    }
}
int main(int argc, char **argv) {
    put_string("========== [#7 TEST XTOA] ==========\n");
    case_1(); waiting(2);
    case_2(); waiting(2);
    case_3(); waiting(2);
    case_4(); waiting(2);
    case_5(); waiting(2);
    put_string("========== [#7 TEST XTOA] ==========\n");
    return 0;
}
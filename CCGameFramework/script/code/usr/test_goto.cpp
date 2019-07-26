#include "/include/io"
int fun1() {
    int a = 0;
    int n = 10;
A:
    a++;
    n--;
    if (a % 2 == 0)
        goto C;
B:
    a *= 2;
C:
    if (n > 0) goto A;
    return a;
}
int fun2() {
    int a = 0, b = 0;
    int n = 10;
A:
B:
    a++;
    b += a;
    if (--n > 0) goto A;
    return b;
}
void fun3(int i) {
    // Duff's device
    // URL: https://www.zhihu.com/question/27417946/answer/36572141
    int n = (i + 7) / 8;
    switch (i % 8) {
    case 0:	do { put_char('*');
    case 7:      put_char('*');
    case 6:      put_char('*');
    case 5:      put_char('*');
    case 4:      put_char('*');
    case 3:      put_char('*');
    case 2:      put_char('*');
    case 1:      put_char('*');
    } while (--n > 0); }
}
int co_1_i, co_1_state = 0;
int co_1(int n) {
    // Coroutine: https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html
    switch (co_1_state) {
    case 0: /* start of function */
        for (co_1_i = 0; co_1_i < n; co_1_i++) {
            co_1_state = 1; /* so we will come back to "case 1" */
            put_char('#');
            return co_1_i;
    case 1: /* resume control straight after the return */
            put_char('*');
        }
    }
    return n;
}
void fun4(int n) {
    int i;
    for (i = 0; i < n; i++) {
        put_int(co_1(n)); put_char(' ');
    }
}
int main(int argc, char **argv) {
    put_string("========== [#18 TEST GOTO] ==========\n");
    put_int(fun1()); put_string("\n");
    put_int(fun2()); put_string("\n");
    fun3(10); put_string("\n");
    fun4(10); put_string("\n");
    put_string("========== [#18 TEST GOTO] ==========\n");
    return 0;
}
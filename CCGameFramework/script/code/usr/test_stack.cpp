#include "/include/io"
#include "/include/sys"
#include "/include/memory"
int output = 0;
int fun_b(int n);
int fun_a(int n) {
    if (n < 1 && !output) {
        output = 1;
        char* s = malloc(1024);
        put_string(stacktrace(s));
    }
    return n < 1 ? 1 : 2 * fun_b(n - 1) + fun_b(n - 2);
}
int fun_b(int n) {
    return n < 1 ? 1 : fun_a(n - 2) + 2 * fun_b(n - 1);
}
int main(int argc, char **argv) {
    put_string("========== [#16 TEST STACK] ==========\n");
    int answer = fun_a(5);
    put_string("Answer: ");
    put_int(answer);
    put_string("\n");
    put_string("========== [#16 TEST STACK] ==========\n");
    return 0;
}
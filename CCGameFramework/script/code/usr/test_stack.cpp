#include "/include/io"
int fun_b();
int fun_a() {
    fun_b();
}
int fun_b() {
    fun_a();
}
int main(int argc, char **argv) {
    put_string("========== [#16 TEST STACK] ==========\n");
    fun_a();
    put_string("========== [#16 TEST STACK] ==========\n");
    return 0;
}
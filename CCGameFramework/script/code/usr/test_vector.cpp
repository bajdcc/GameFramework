#include "/include/io"
#include "/include/memory"
#include "/include/string"
#include "/include/vector"
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
    put_string("-- CASE #1 --\n");
    void *v = vector_new(sizeof(int));
    int i;
    for (i = 0; i < 1000; ++i) {
        vector_push(v, &i);
    }
    for (i = 0; i < 10; ++i) {
        put_int(*(int *)vector_get(v, i * 100 + 99));
        put_string("\n");
    }
    vector_del(v);
}
int case_2() {
    put_string("-- CASE #2 --\n");
    void *v = vector_new(sizeof(int));
    int i, j;
    for (i = 1, j = -1; i < 100; ++i, --j) {
        vector_push(v, &i);
        if (i % 3 == 0 || i % 7 == 0)
            vector_pop(v);
        if (i % 5 == 0 || i % 9 == 0)
            vector_insert(v, i % 6, &j);
        put_int(*(int *)vector_get(v, vector_size(v) - 1));
        put_string(" ");
        put_int(*(int *)vector_get(v, 0));
        put_string("\n");
    }
    vector_del(v);
}
int main(int argc, char **argv) {
    put_string("========== [#8 TEST VECTOR] ==========\n");
    case_1(); waiting(1);
    case_2(); waiting(1);
    put_string("========== [#8 TEST VECTOR] ==========\n");
    return 0;
}
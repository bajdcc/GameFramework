#include "/include/json"
#include "/include/io"
#include "/include/memory"
void print_addr(int *addr, int len) {
    int i;
    for (i = 0; i < len; i++) {
        put_string(" [");
        put_hex(&addr[i]);
        put_string("] ");
        put_hex(addr[i]);
        if (i % 4 == 3)put_string("\n");
    }
}
void test_1() {
    json_object* obj = json_parse_obj("5");
    put_hex(obj); put_string(" ");
    put_int(obj->type); put_string(" ");
    put_int(obj->data.i);
    free(obj);
}
void test_2() {
    json_object* obj = json_parse_obj("5.6");
    put_hex(obj); put_string(" ");
    put_int(obj->type); put_string(" ");
    put_double(obj->data.d);
    free(obj);
}
void test_3() {
    json_object* obj = json_parse_obj("[4, 5.6]");
    print_addr(obj, 32); put_string("\n");
    put_hex(obj); put_string(" ");
    put_int(obj->type); put_string(" ");
    put_int(json_array_size(obj)); put_string(" ");
    put_int(json_array_get(obj, 0)->data.i); put_string(" ");
    put_double(json_array_get(obj, 1)->data.d); put_string(" ");
    free(obj);
}
int main(int argc, char **argv) {
    put_string("========== [#20 TEST JSON] ==========\n");
    test_1(); put_string("\n");
    test_2(); put_string("\n");
    test_3(); put_string("\n");
    put_string("========== [#20 TEST JSON] ==========\n");
    return 0;
}
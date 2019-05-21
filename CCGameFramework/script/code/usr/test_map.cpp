#include "/include/proc"
#include "/include/io"
#include "/include/map"
void* one(void* p) {
    return p;
}
int cmp(void* x, void* y) {
    return x - y;
}
int print(__rbt_node__* node) {
    if (node) {
        print(node->left);

        put_string("-> ");
        if (node->color == 0)
            put_string("[B] ");
        else
            put_string("[R] ");

        put_string("key= ");
        put_int(node->key);
        put_string(", value= ");
        put_int(node->value);
        put_string("\n");

        print(node->right);
    }
}
void case_1() {
    __rbt_info__ m = rbt_create(one, one, one, one, cmp);
    rbt_insert(&m, 1, 1);
    rbt_insert(&m, 5, 5);
    rbt_insert(&m, 3, 3);
    rbt_insert(&m, 4, 4);
    rbt_insert(&m, 2, 2);
    print(&m);
}
int main(int argc, char** argv) {
    put_string("========== [#10 TEST MAP] ==========\n");
    case_1();
    put_string("========== [#10 TEST MAP] ==========\n");
    return 0;
}
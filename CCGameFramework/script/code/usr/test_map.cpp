#include "/include/proc"
#include "/include/io"
#include "/include/map"
void* one(void* p) {
    return p;
}
int cmp(void* x, void* y) {
    return x - y;
}
int print(__rbt_node__* node, int n) {
    if (node) {
        print(node->left, n + 1);

        int i;
        for (i = 0; i <= n; ++i)
            put_string("-");
        put_string("> ");
        if (node->color == 1)
            put_string("[B] ");
        else
            put_string("[R] ");

        put_string("key= ");
        put_int(node->key);
        put_string(", value= ");
        put_int(node->value);
        put_string(", ptr= ");
        put_hex(node);
        put_string("\n");

        print(node->right, n + 1);
    }
}
int print_inorder(__rbt_node__* node) {
    if (node) {
        print_inorder(node->left);
        put_int(node->key); put_string(" ");
        print_inorder(node->right);
    }
}
int a[0] = { 1, 4, 3, 6, 9, 7, 2, 5, 8 };
void case_1() {
    __rbt_info__ m = rbt_create(one, one, one, one, cmp);
    int i;
    for (i = 0; i < 9; ++i)
        rbt_insert(&m, a[i] * 10, i);
    print(m.root, 0);
    print_inorder(m.root); put_string("\n");
    rbt_destroy(&m);
}
void case_2() {
    __rbt_info__ m = rbt_create(one, one, one, one, cmp);
    int i;
    for (i = 0; i < 9; ++i)
        rbt_insert(&m, a[i] * 10, i);
    print_inorder(m.root); put_string("\n");
    for (i = 0; i < 9; ++i) {
        put_string("Remove "); put_int(a[i] * 10); put_string(" -> ");
        rbt_remove(&m, a[i] * 10); print_inorder(m.root); put_string("\n");
    }
    rbt_destroy(&m);
}
int main(int argc, char** argv) {
    put_string("========== [#10 TEST MAP] ==========\n");
    case_1();
    case_2();
    put_string("========== [#10 TEST MAP] ==========\n");
    return 0;
}
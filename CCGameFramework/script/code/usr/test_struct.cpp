#include "/include/io"
#include "/include/memory"
#include "/include/string"
struct node {
    int value;
    node *next;
};
node *create(int n) {
    if (n <= 0)
        return (node *) 0;
    node *new_node = (node *) malloc(sizeof(node));
    new_node->value = n;
    new_node->next = create(n - 1);
    return new_node;
}
int print(node *head) {
    while (head) {
        put_int(head->value); put_string(" ");
        head = head->next;
    }
}
int destroy(node *head) {
    node *old;
    while (head) {
        old = head;
        head = head->next;
        free((int) old);
    }
}
int case_1() {
    put_string("-- CASE #1 --\n");
    node *head = create(10);
    print(head);
    destroy(head);
    put_string("\n");
}
struct node2 {
    int a, b;
};
union node3 {
    struct _1 {
        int a, b;
    } A;
    struct _2 {
        char a, b, c, d;
        char e, f, g, h;
    } B;
};
int case_2() {
    put_string("-- CASE #2 --\n");
    node2 *n1 = (node2 *) malloc(sizeof(node2));
    strncpy((char *) n1, "ABCD1234", 8);
    put_int(n1->a); put_string(" ");
    put_int(n1->b);
    free((int) n1);
    put_string("\n");
    node3 *n2 = (node3 *) malloc(sizeof(node3));
    strncpy((char *) n2, "1234ABCD", 8);
    put_int(n2->A.a); put_string(" ");
    put_int(n2->A.b); put_string(" ");
    put_char(n2->B.a); put_string(" ");
    put_char(n2->B.h);
    free((int) n2);
    put_string("\n");
}
int main(int argc, char **argv) {
    put_string("========== [#6 TEST STRUCT] ==========\n");
    case_1();
    case_2();
    put_string("========== [#6 TEST STRUCT] ==========\n");
    return 0;
}
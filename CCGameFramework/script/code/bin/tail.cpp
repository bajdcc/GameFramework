#include "/include/io"
#include "/include/fs"
#include "/include/memory"
#include "/include/shell"
#include "/include/string"
#include "/include/xtoa_atoi"
struct string {
    char *text;
    int capacity;
    int length;
};
string new_string() {
    string s;
    s.text = malloc(16);
    s.capacity = 16;
    s.length = 0;
    return s;
}
struct node {
    string text;
    node *prev;
    node *next;
};
void destroy(node *tmp) {
    free(tmp->text.text);
    free(tmp);
}
void push(node **head, node **last, string s, int not_remove) {
    node *new_node = (node *) malloc(sizeof(node));
    new_node->text = s;
    new_node->prev = 0;
    new_node->next = *head;
    if (new_node->next)
        new_node->next->prev = new_node;
    if (not_remove == 1) {
        if (!*last)
            *last = new_node;
    } else {
        node *tmp = *last;
        *last = (*last)->prev;
        (*last)->next = 0;
        destroy(tmp);
    }
    *head = new_node;
}
void append_char(string *s, char c) {
    if (s->length >= s->capacity - 1) {
        s->capacity <<= 1;
        char *new_text = malloc(s->capacity);
        strcpy(new_text, s->text);
        free(s->text);
        s->text = new_text;
    }
    (s->text)[s->length++] = c;
    (s->text)[s->length] = 0;
}
void print(node *last) {
    while (last) {
        put_string((last->text).text);
        put_string("\n");
        last = last->prev;
    }
}
void tail(int n) {
    int c, i = 0, cmd = 0;
    node *list = (node *) 0;
    node *last = (node *) 0;
    string s = new_string();
    input_lock();
    while ((c = input_valid()) != -1) {
        c = input_char();
        if (((char) c) == '\033') {
            append_char(&s, c);
            cmd = 1 - cmd;
        } else if (cmd == 0) {
            if (((char) c) == '\n') {
                push(&list, &last, s, i < n);
                s = new_string();
                ++i;
            } else {
                append_char(&s, c);
            }
        } else {
            append_char(&s, c);
        }
    }
    input_unlock();
    print(last);
}
int main(int argc, char **argv) {
    if (argc == 1) { // tail
        shell("pipe");
    } else if (argc == 2) { // tail XX
        int n = atoi32(argv[1]);
        if (n <= 0)
            return 1;
        tail(n);
    } else {
        set_fg(240, 0, 0);
        put_string("[Error] Invalid argument.");
        restore_fg();
    }
    return 0;
}
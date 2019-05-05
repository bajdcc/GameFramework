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
void push(node **head, string s) {
    node *new_node = (node *) malloc(sizeof(node));
    new_node->text = s;
    new_node->prev = 0;
    new_node->next = *head;
    if (new_node->next)
        new_node->next->prev = new_node;
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
void print(node *list) {
    node *prev = list;
    while (list) {
        prev = list;
        list = list->next;
    }
    while (prev) {
        put_string((prev->text).text);
        put_string("\n");
        prev = prev->prev;
    }
}
void head(int n) {
    int c, i = 0, cmd = 0;
    node *list = (node *) 0;
    string s = new_string();
    input_lock();
    while ((c = input_valid()) != -1) {
        c = input_char();
        if (i < n) {
            if (((char) c) == '\033') {
                append_char(&s, c);
                cmd = 1 - cmd;
            } else if (cmd == 0) {
                if (((char) c) == '\n') {
                    push(&list, s);
                    s = new_string();
                    ++i;
                } else {
                    append_char(&s, c);
                }
            } else {
                append_char(&s, c);
            }
        }
    }
    input_unlock();
    print(list);
}
int main(int argc, char **argv) {
    if (argc == 1) { // head
        shell("pipe");
    } else if (argc == 2) { // head XX
        int n = atoi32(argv[1]);
        if (n <= 0)
            return 1;
        head(n);
    } else {
        set_fg(240, 0, 0);
        put_string("[Error] Invalid argument.");
        restore_fg();
    }
    return 0;
}
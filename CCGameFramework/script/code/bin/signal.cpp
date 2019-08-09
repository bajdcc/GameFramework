#include "/include/io"
#include "/include/string"
#include "/include/memory"
#include "/include/shell"
#include "/include/proc"
#include "/include/xtoa_atoi"
struct string {
    char* text;
    int capacity;
    int length;
};
string new_string() {
    string s;
    s.text = malloc(16);
    s.text[0] = '\0';
    s.capacity = 16;
    s.length = 0;
    return s;
}
struct node {
    string text;
    node* prev;
    node* next;
};
void push(node** head, string s) {
    node* new_node = (node*)malloc(sizeof(node));
    new_node->text = s;
    new_node->prev = 0;
    new_node->next = *head;
    if (new_node->next)
        new_node->next->prev = new_node;
    *head = new_node;
}
void append_char(string* s, char c) {
    if (s->length >= s->capacity - 1) {
        s->capacity <<= 1;
        char* new_text = malloc(s->capacity);
        strcpy(new_text, s->text);
        free(s->text);
        s->text = new_text;
    }
    (s->text)[s->length++] = c;
    (s->text)[s->length] = 0;
}
void run(node* list, char* sig) {
    node* prev = list;
    while (list) {
        prev = list;
        list = list->next;
    }
    char* cmd = malloc(256);
    while (prev) {
        //put_string("[!] ");
        strcpy(cmd, "signal ");
        strcat(cmd, sig);
        strcat(cmd, " ");
        strcat(cmd, (prev->text).text);
        //put_string(cmd);
        //newline();
        shell(cmd);
        //newline();
        prev = prev->prev;
    }
}
void signal(char* sig) {
    int c, cmd = 0;
    node* list = (node*)0;
    string s = new_string();
    input_lock();
    while ((c = input_valid()) != -1) {
        c = input_char();
        if (((char)c) == '\033') {
            cmd = 1 - cmd;
        }
        else if (cmd == 0) {
            if (((char)c) == '\n') {
                push(&list, s);
                s = new_string();
            }
            else {
                append_char(&s, c);
            }
        }
        else {
            append_char(&s, c);
        }
    }
    input_unlock();
    if (s.length > 0)
        push(&list, s);
    run(list, sig);
}
int main(int argc, char **argv) {
    char* sig = "0";
    if (argc >= 2) { // signal SIG
        sig = argv[1];
    }
    if (argc == 3) { // signal SIG PID
        int pid = atoi32(argv[2]);
        send_signal(pid, atoi32(sig));
        return 0;
    }
    int direct_input = input_state();
    if (!direct_input) {
        signal(sig);
    }
    return 0;
}
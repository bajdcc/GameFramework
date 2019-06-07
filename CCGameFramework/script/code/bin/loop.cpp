#include "/include/shell"
#include "/include/memory"
#include "/include/proc"
struct string {
    char* text;
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
int main(int argc, char **argv) {
    if (argc <= 1) {
        set_fg(240, 0, 0);
        put_string("[Error] Missing argument.");
        restore_fg();
        return;
    }
    int i;
    string s = new_string();
    for (i = 1; i < argc; ++i) {
        char* c = argv[i];
        while (*c)
            append_char(&s, *c++);
        if (i < argc - 1)
            append_char(&s, ' ');
    }
    for (;;) {
        shell(s.text);
        sleep(500);
    }
    return 0;
}
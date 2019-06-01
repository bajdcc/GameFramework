#include "/include/proc"
#include "/include/fs"
#include "/include/io"
#include "/include/memory"
#include "/include/string"
#include "/include/json"
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
void pipe() {
    int c, cmd = 0;
    string s = new_string();
    input_lock();
    while ((c = input_valid()) != -1) {
        c = input_char();
        if (((char)c) == '\033') {
            cmd = 1 - cmd;
        }
        else if (cmd == 0) {
            append_char(&s, c);
        }
    }
    input_unlock();
    put_string(json_parse(s.text));
}
int main(int argc, char **argv) {
    pipe();
    return 0;
}
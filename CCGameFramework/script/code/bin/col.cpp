#include "/include/io"
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
void show(string* s, int n) {
    int i = 0; char* c = s->text;
    while (*c) {
        if (*c == ' ') { c++; continue; }
        i++;
        if (n == i) {
            while (*c) {
                if (*c == ' ') break;
                put_char(*c++);
            }
            put_char('\n');
            break;
        }
        else {
            while (*c) {
                if (*c != ' ') c++;
                else break;
            }
        }
    }
}
void col(int n) {
    int c, cmd = 0;
    string s = new_string();
    input_lock();
    while ((c = input_valid()) != -1) {
        c = input_char();
        if (((char) c) == '\033') {
            cmd = 1 - cmd;
        } else if (cmd == 0) {
            if (((char) c) == '\n') {
                show(&s, n);
                free(s.text);
                s = new_string();
            } else {
                append_char(&s, c);
            }
        }
    }
    input_unlock();
}
int main(int argc, char **argv) {
    if (argc == 1) { // col
        shell("pipe");
    } else if (argc == 2) { // col XX
        int n = atoi32(argv[1]);
        if (n <= 0) {
            shell("pipe");
            return 0;
        }
        col(n); // start from 1
    } else {
        set_fg(240, 0, 0);
        put_string("[Error] Invalid argument.");
        restore_fg();
    }
    return 0;
}
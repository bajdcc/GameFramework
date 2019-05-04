#include "/include/io"
#include "/include/memory"
#include "/include/string"
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
// KMP
int *build_next(char *rep) {
    int len = strlen(rep), i, u = 0, v = -1;
    int *arr = malloc(len * sizeof(int));
    for (i = 0; i < len; ++i) {
        arr[i] = -1;
    }
    while (u < len - 1) {
        if (v == -1 || (rep[u] == rep[v])) {
            u++;
            v++;
            arr[u] = v;
        } else {
            v = arr[v];
        }
    }
    return arr;
}
int match(char *str, int slen, char *rep, int len, int *arr) {
    int i = 0, j = 0, slen = strlen(str);
    while (i < slen && j < len) {
        if (j == -1 || (str[i] == rep[j])) {
            i++; j++;
        } else {
            j = arr[j];
        }
    }
    if (j == len) {
        return i - j;
    }
    return -1;
}
void grep(char *rep) {
    int c, cmd = 0;
    string s = new_string();
    string s2 = new_string();
    int *arr = build_next(rep), len = strlen(rep);
    input_lock();
    while ((c = input_valid()) != -1) {
        c = input_char();
        if (((char) c) == '\033') {
            append_char(&s, c);
            cmd = 1 - cmd;
        } else if (cmd == 0) {
            if (((char) c) == '\n') {
                if (match(s2.text, s2.length, rep, len, arr) != -1) {
                    put_string(s.text);
                    put_string("\n");
                } else {
                    free(s.text);
                }
                free(s2.text);
                s = new_string();
                s2 = new_string();
            } else {
                append_char(&s2, c);
                append_char(&s, c);
            }
        } else {
            append_char(&s, c);
        }
    }
    input_unlock();
}
int main(int argc, char **argv) {
    char *cmd = malloc(1024);
    int i;
    for (i = 1; i < argc; ++i) {
        strcat(cmd, argv[i]);
        if (i < argc - 1)
            strcat(cmd, " ");
    }
    grep(cmd);
    return 0;
}
//
// Project: clibparser
// Created by bajdcc
//

#include "/include/memory"
#include "/include/string"

struct __arg_string {
    char* text;
    int capacity;
    int length;
};
__arg_string __arg_new_string() {
    __arg_string s;
    s.text = malloc(16);
    s.capacity = 16;
    s.length = 0;
    return s;
}
void __arg_append_char(__arg_string* s, char c) {
    if (s->length >= s->capacity - 1) {
        s->capacity <<= 1;
        char* new_text = malloc(s->capacity);
        strncpy(new_text, s->text, s->length);
        free(s->text);
        s->text = new_text;
    }
    (s->text)[s->length++] = c;
    (s->text)[s->length] = 0;
}
char* arg_string(int start, int end, char** argv) {
    __arg_string s = __arg_new_string();
    int i;
    for (i = start; i < end; ++i) {
        char* c = argv[i];
        while (*c)
            __arg_append_char(&s, *c++);
        if (i < end - 1)
            __arg_append_char(&s, ' ');
    }
    return s.text;
}
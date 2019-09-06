//
// Project: clibparser
// Created by bajdcc
//

#include "/include/memory"
#include "/include/string"
#include "/include/xtoa_itoa"
#include "/include/xtoa_dtoa"

struct __varg_string {
    char* text;
    int capacity;
    int length;
};
__varg_string __varg_new_string() {
    __varg_string s;
    s.text = malloc(16);
    s.capacity = 16;
    s.length = 0;
    return s;
}
void __varg_append_char(__varg_string* s, char c) {
    if (s->length >= s->capacity - 2) {
        s->capacity <<= 1;
        char* new_text = malloc(s->capacity);
        strncpy(new_text, s->text, s->length);
        free(s->text);
        s->text = new_text;
    }
    (s->text)[s->length++] = c;
}
void __varg_append_string(__varg_string* s, char* str) {
    while (*str)
        __varg_append_char(s, *str++);
}
char* format(char* fmt, ...) {
    __varg_string s = __varg_new_string();
    int L = strlen(fmt), i;
    int format_state = 0;
    int escape_state = 0;
    char* args = (char*)(((int*)& fmt) + 1);
    for (i = 0; i < L; i++) {
        if (format_state == 1) {
            char c = fmt[i];
            switch (c) {
            case 'c':
            {
                __varg_append_char(&s, (char)*args);
                args += sizeof(int);
            }
            break;
            case 'd':
            {
                static char _d[16];
                i32toa(*((int*)args), _d);
                __varg_append_string(&s, _d);
                args += sizeof(int);
            }
            break;
            case 'D':
            {
                static char _D[16];
                u32toa(*((unsigned int*)args), _D);
                __varg_append_string(&s, _D);
                args += sizeof(unsigned int);
            }
            break;
            case 'l':
            {
                static char _l[32];
                i64toa(*((long*)args), _l);
                __varg_append_string(&s, _l);
                args += sizeof(long);
            }
            break;
            case 'L':
            {
                static char _L[32];
                u64toa(*((unsigned long*)args), _L);
                __varg_append_string(&s, _L);
                args += sizeof(unsigned long);
            }
            break;
            case 's':
            {
                __varg_append_string(&s, *(char**)args);
                args += sizeof(char*);
            }
            break;
            case 'f':
            {
                static char _f[256];
                xtoa_dtoa(*((double*)args), _f);
                __varg_append_string(&s, _f);
                args += sizeof(double);
            }
            break;
            default:
            {
                __varg_append_char(&s, fmt[i]);
            }
            break;
            }
            format_state = 0;
        }
        else {
            if (fmt[i] == '%')
                format_state = 1;
            else
                __varg_append_char(&s, fmt[i]);
        }
    }
    (s.text)[s.length++] = 0;
    return s.text;
}
//
// Project: clibparser
// Created by bajdcc
//

// 字符串操作

int strlen(char *text) {
    int i = 0;
    while (*text++)
        i++;
    return i;
}
int strcpy(char *dst, char *src) {
    while (*dst++ = *src++);
}
int strncpy(char *dst, char *src, int n) {
    if (dst < src) while (n-- > 0 && (*dst++ = *src++));
    else {
        src += n - 1;
        dst += n - 1;
        while (n-- > 0 && (*dst-- = *src--));
    }
}
int strcmp(char *a, char *b) {
    int len = 0;
    while(*a && *b && (*a == *b)) {
        a++, b++;
    }
    return *a - *b;
}
int strncmp(char *a, char *b, int n) {
    while(n-- && *a && *b) {
        if (*a != *b) {
            return *a - *b;
        } else {
            a++, b++;
        }
    }
    if (n < 0)
        return 0;
    return *a - *b;
}
char *strchr(char *text, char c) {
    for (; *text; ++text)
        if (*text == c)
            return text;
    return (char *) 0;
}
char *strcat(char *dst, char *src) {
    char *s = dst;
    while (*s++); s--;
    while (*s++ = *src++);
    return dst;
}

//
// Project: clibparser
// Created by bajdcc
//

// 内存管理
char *malloc(int size) {
    size;
    interrupt 30;
}
int free(int addr) {
    addr;
    interrupt 31;
}
void memmove(char *dst, char *src, int n) {
    if (dst < src) while (n-- > 0) *dst++ = *src++;
    else {
        src += n - 1;
        dst += n - 1;
        while (n-- > 0) *dst-- = *src--;
    }
}
void memset(char *src, char c, int n) {
    for (; 0 < n; ++src, --n)
        *src = c;
}

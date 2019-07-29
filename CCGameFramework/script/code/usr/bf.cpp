#include "/include/io"
#include "/include/shell"
#include "/include/memory"
#include "/include/readfile"
// REFER: https://blog.csdn.net/sm9sun/article/details/53896962
void error(char* txt) {
    set_fg(240, 0, 0);
    put_string(txt);
    restore_fg();
}
void bf(char* code, int len) {
    char* s = malloc(30000);
    memset(s, 0, 30000);
    int stack[100];
    memset(s, 0, sizeof(stack));
    int stack_len = 0;
    int i = 0, j, k, x = 0, t;
    char* p = s + 10000;
    while (i < len) {
        switch (code[i]) {
        case '+':
            (*p)++;
            break;
        case '-':
            (*p)--;
            break;
        case '>':
            p++;
            break;
        case '<':
            p--;
            break;
        case '.':
            put_char((char)(*p));
            break;
        case ',':
            t = get_char();
            if (t != -1)
                * p = (char)t;
            else {
                error("[ERROR] Input error.");
                return 1;
            }
            break;
        case '[':
            if (*p) {
                stack[stack_len++] = i;
            }
            else {
                for (k = i, j = 0; k < len; k++) {
                    code[k] == '[' && j++;
                    code[k] == ']' && j--;
                    if (j == 0)break;
                }
                if (j == 0)
                    i = k;
                else {
                    error("[ERROR] Running error.");
                    return 2;
                }
            }
            break;
        case ']':
            i = stack[stack_len-- - 1] - 1;
            break;
        default:
            break;
        }
        i++;
    }
}
int main(int argc, char** argv) {
    if (argc > 1) {
        char* code;
        int len;
        if (readfile(argv[1], &code, &len) == 0) {
            bf(code, len);
        }
        else {
            set_fg(240, 0, 0);
            put_string("[ERROR] File not exists.");
            restore_fg();
        }
    }
    else {
        set_fg(240, 0, 0);
        put_string("[Error] Invalid argument.");
        restore_fg();
    }
    return 0;
}
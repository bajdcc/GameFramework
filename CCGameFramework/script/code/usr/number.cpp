#include "/include/proc"
#include "/include/memory"
#include "/include/string"
#include "/include/io"
void error(char *s) {
    set_fg(240, 0, 0);
    put_string(s);
    restore_fg();
}
struct big_number {
    int negative;
    char *text;
    int capacity;
    int length;
};
big_number new_number() {
    big_number s;
    s.negative = false;
    s.text = malloc(16);
    s.capacity = 16;
    s.length = 0;
    return s;
}
big_number new_number2(int n) {
    big_number s;
    s.negative = false;
    s.text = malloc(n);
    memset(s.text, 0, n);
    s.capacity = n;
    s.length = n;
    return s;
}
void append_number(big_number *s, char c) {
    if (s->length >= s->capacity - 1) {
        s->capacity <<= 1;
        char *new_text = malloc(s->capacity);
        memmove(new_text, s->text, s->length);
        free(s->text);
        s->text = new_text;
    }
    (s->text)[s->length++] = c;
}
big_number get_number(char *s) {
    big_number n = new_number();
    char *str = s;
    while (*s) {
        if (*s >= '0' && *s <= '9') {
            append_number(&n, *s - '0');
        } else {
            error("[ERROR] Invalid number: ");
            error(str);
            error("\n");
            exit(3);
        }
        s++;
    }
    return n;
}
void print_number(big_number s) {
    int i;
    if (s.negative)
        put_char('-');
    for (i = 0; i < s.length && s.text[i] == 0; ++i);
    if (i == s.length) {
        put_char('0');
        return;
    }
    for (; i < s.length; ++i) {
        put_char('0' + s.text[i]);
    }
}
void align2(big_number *a, big_number *b) {
    char *new_text = malloc(b->capacity);
    memset(new_text, 0, b->length - a->length);
    memmove(new_text + b->length - a->length, a->text, a->length);
    free(a->text);
    a->text = new_text;
    a->length = b->length;
    a->capacity = b->capacity;
}
void align(big_number *a, big_number *b) {
    if (a->length == b->length)
        return;
    if (a->length < b->length)
        align2(a, b);
    else
        align2(b, a);
}
int compare(big_number *a, big_number *b) {
    int i, j;
    for (i = 0; i < a->length && a->text[i] == 0; ++i);
    for (j = 0; j < b->length && b->text[j] == 0; ++j);
    int nA = a->length - i;
    int nB = b->length - j;
    if (nA > nB)
        return 1;
    if (nA < nB)
        return -1;
    for (; i < a->length && j < b->length; ++i, ++j) {
        if (a->text[i] > b->text[j])
            return 1;
        if (a->text[i] < b->text[j])
            return -1;
    }
    return 0;
}
big_number plus(big_number *a, big_number *b) {
    big_number c = new_number2(a->length + 1);
    int i, acc = 0;
    for (i = a->length - 1; i >= 0; --i) {
        c.text[i + 1] = a->text[i] + b->text[i] + acc;
        if (c.text[i + 1] >= 10) {
            c.text[i + 1] -= 10;
            acc = 1;
        } else {
            acc = 0;
        }
    }
    c.text[0] = acc;
    return c;
}
big_number minus(big_number *a, big_number *b) {
    int cmp = compare(a, b);
    if (cmp == 0) {
        big_number c = new_number2(1);
        c.text[0] = 0;
        return c;
    }
    if (cmp < 0) {
        big_number c = minus(b, a);
        c.negative = true;
        return c;
    }
    big_number c = new_number2(a->length);
    int i, acc = 0;
    for (i = a->length - 1; i >= 0; --i) {
        c.text[i] = a->text[i] - b->text[i] - acc;
        if (c.text[i] < 0) {
            c.text[i] += 10;
            acc = 1;
        } else {
            acc = 0;
        }
    }
    return c;
}
big_number times(big_number *a, big_number *b) {
    int i, j;
    for (i = 0; i < a->length && a->text[i] == 0; ++i);
    for (j = 0; j < b->length && b->text[j] == 0; ++j);
    int nA = a->length - i;
    int nB = b->length - j;
    big_number c = new_number2(nA + nB);
    int m, n, acc;
    for (m = nA - 1; m >= 0; --m) {
        acc = 0;
        for (n = nB - 1; n >= 0; --n) {
            c.text[m + n + 1] += a->text[i + m] * b->text[j + n] + acc;
            acc = c.text[m + n + 1] / 10;
            c.text[m + n + 1] %= 10;
        }
        c.text[m] = acc;
    }
    return c;
}
struct divide_struct {
    big_number shang, yu;
};
int is_zero(big_number *a) {
    int i;
    for (i = 0; i < a->length && a->text[i] == 0; ++i);
    return i == a->length;
}
int numcmp(char *a, char *b, int n) {
    for (; n > 0; n--) {
        if (*a < *b) {
            return -1;
        } else if (*a > *b) {
            return 1;
        }
        a++, b++;
    }
    return 0;
}
divide_struct divide(big_number *a, big_number *b) {
    if (is_zero(b)) {
        error("[ERROR] Cannot divide by zero.\n");
        exit(4);
    }
    int cmp = compare(a, b);
    if (cmp == 0) {
        divide_struct ds;
        ds.shang = new_number2(1);
        ds.shang.text[0] = 1;
        ds.yu = new_number2(1);
        ds.yu.text[0] = 0;
        return ds;
    }
    if (cmp < 0) {
        divide_struct ds;
        ds.shang = new_number2(1);
        ds.shang.text[0] = 0;
        ds.yu = *a;
        return ds;
    }
    int i, j, k;
    for (i = 0; i < a->length && a->text[i] == 0; ++i);
    for (j = 0; j < b->length && b->text[j] == 0; ++j);
    int nA = a->length - i;
    int nB = b->length - j;
    big_number c = new_number2(nA - nB + 1);
    int m, n, acc;
    for (m = 0, n = 0; m + n <= nA - nB;) {
        if (numcmp(a->text + i + m, b->text + j - n, nB + n) >= 0) {
            do {
                c.text[m + n]++;
                acc = 0;
                for (k = nB + n - 1; k >= 0; --k) {
                    a->text[i + m + k] -= b->text[j + k - n] + acc;
                    if (a->text[i + m + k] < 0) {
                        a->text[i + m + k] += 10;
                        acc = 1;
                    } else {
                        acc = 0;
                    }
                }
            } while (numcmp(a->text + i + m, b->text + j - n, nB + n) >= 0);
            for (; m <= nA - nB && a->text[i + m] == 0; ++m);
            n = 0;
        } else {
            ++n;
        }
        if (m < nA - nB) {
        }
    }
    divide_struct ds;
    ds.shang = c;
    ds.yu = *a;
    return ds;
}
int main(int argc, char **argv) {
    if (argc != 4) {
        error("[ERROR] Invalid arguments. Required: [+-*/] BigNumber1 BigNumber2\n");
        exit(1);
    }
    big_number a = get_number(argv[2]);
    big_number b = get_number(argv[3]);
    align(&a, &b);
    switch (argv[1][0]) {
        default:
            error("[ERROR] Invalid operator. Required: [+-*/]\n");
            exit(2);
            break;
        case '+':
            print_number(plus(&a, &b));
            break;
        case '-':
            print_number(minus(&a, &b));
            break;
        case '*':
            print_number(times(&a, &b));
            break;
        case '/': {
            divide_struct ds = divide(&a, &b);
            print_number(ds.shang);
            put_char(' ');
            print_number(ds.yu);
        }
            break;
    }
    return 0;
}
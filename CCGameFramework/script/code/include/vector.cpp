//
// Project: clibparser
// Created by bajdcc
//

#include "/include/memory"
#include "/include/string"

int _vector_initial_size_ = 16;
struct _vector_struct_ {
    int _sizeof;
    char *data;
    int capacity;
    int size;
};

void *vector_new(int _sizeof) {
    _vector_struct_ *s = malloc(sizeof(_vector_struct_));
    s->_sizeof = _sizeof;
    s->capacity = _vector_initial_size_;
    s->size = 0;
    s->data = malloc(s->capacity * s->_sizeof);
    return s;
}

void vector_del(void *p) {
    _vector_struct_ *s = (_vector_struct_ *) p;
    free(s->data);
    free(s);
}

void vector_push(void *p, void *data) {
    _vector_struct_ *s = (_vector_struct_ *) p;
    if (s->size == s->capacity) {
        s->capacity <<= 1;
        void *new_data = malloc(s->capacity * s->_sizeof);
        memmove(new_data, s->data, s->size * s->_sizeof);
        free(s->data);
        s->data = new_data;
    }
    memmove(s->data + s->size++ * s->_sizeof, data, s->_sizeof);
}

void vector_pop(void *p) {
    _vector_struct_ *s = (_vector_struct_ *) p;
    if (s->size > 0) {
        s->size--;
    }
}

void vector_insert(void *p, int n, void *data) {
    _vector_struct_ *s = (_vector_struct_ *) p;
    if (n >= 0 && n <= s->size) {
        if (s->size == s->capacity) {
            s->capacity <<= 1;
            void *new_data = malloc(s->capacity * s->_sizeof);
            memmove(new_data, s->data, s->capacity * s->_sizeof);
            free(s->data);
            s->data = new_data;
        }
        memmove(s->data + (n + 1) * s->_sizeof,
                s->data + (n) * s->_sizeof,
                (s->size++ - n) * s->_sizeof);
        memmove(s->data + n * s->_sizeof, data, s->_sizeof);
    }
}

int vector_size(void *p) {
    _vector_struct_ *s = (_vector_struct_ *) p;
    return s->size;
}

void *vector_get(void *p, int index) {
    _vector_struct_ *s = (_vector_struct_ *) p;
    if (index >= 0 && index < s->size) {
        return s->data + index * s->_sizeof;
    }
    return 0;
}

void vector_set(void *p, int index, void *data) {
    _vector_struct_ *s = (_vector_struct_ *) p;
    if (index > 0 && index < s->size) {
        memmove(s->data + index * s->_sizeof, data, s->_sizeof);
    }
}

void vector_clear(void *p) {
    _vector_struct_ *s = (_vector_struct_ *) p;
    s->size = 0;
}
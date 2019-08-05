//
// Project: clibparser
// Created by bajdcc
//

// JSON解析

#include "/include/string"

char* json_parse(char* text) {
    text;
    interrupt 401;
}

enum json_type {
    j_none,
    j_object,
    j_array,
    j_char,
    j_uchar,
    j_short,
    j_ushort,
    j_int,
    j_uint,
    j_long,
    j_ulong,
    j_float,
    j_double,
    j_string,
};

struct json_object_obj_list {
    unsigned int name;
    unsigned int value;
};

struct json_object_obj {
    int len;
    unsigned int list;
};

struct json_object_arr {
    int len;
    unsigned int list;
};

struct json_object {
    int type;
    union _u_ {
        json_object_obj* obj;
        json_object_arr* arr;
        char c;
        unsigned char uc;
        short s;
        unsigned short us;
        int i;
        unsigned int ui;
        long l;
        unsigned long ul;
        float f;
        double d;
        char* str;
    } data;
};

json_object* json_parse_obj(char* text) {
    text;
    interrupt 402;
}

int json_array_size(json_object* obj) {
    return obj->data.arr->len;
}

int json_obj_size(json_object* obj) {
    return obj->data.obj->len;
}

json_object* json_array_get(json_object* obj, int index) {
    return (json_object*) * (unsigned int*)((int*)(&obj->data.arr->list) + index);
}

json_object* json_obj_get_1(json_object* obj, int index) {
    return (json_object*) * (unsigned int*)((int*)(&obj->data.obj->list) + (index * 2));
}

json_object* json_obj_get_2(json_object* obj, int index) {
    return (json_object*) * (unsigned int*)((int*)(&obj->data.obj->list) + (index * 2 + 1));
}

json_object* json_obj_get_int(json_object* obj, int key) {
    int i;
    int len = json_obj_size(obj);
    for (i = 0; i < len; i++) {
        json_object* k = json_obj_get_1(obj, i);
        if (k->type == j_int && k->data.i == key) {
            return json_obj_get_2(obj, i);
        }
    }
    return (json_object*)0;
}

json_object* json_obj_get_string(json_object* obj, char* key) {
    int i;
    int len = json_obj_size(obj);
    for (i = 0; i < len; i++) {
        json_object* k = json_obj_get_1(obj, i);
        if (k->type == j_string && strcmp(k->data.str, key) == 0) {
            return json_obj_get_2(obj, i);
        }
    }
    return (json_object*)0;
}
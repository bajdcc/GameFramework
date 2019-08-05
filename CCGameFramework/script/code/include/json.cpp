//
// Project: clibparser
// Created by bajdcc
//

// JSON解析

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

json_object* json_array_get(json_object* obj, int index) {
    return (json_object*) * (unsigned int*)((int*)(&obj->data.arr->list) + index);
}
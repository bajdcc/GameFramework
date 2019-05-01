//
// Project: clibparser
// Created by bajdcc
//

// 红黑树

// 红色节点=0
// 黑色节点=1

// 成功=1
// 失败=0

struct __rbt_node__ {
    unsigned char color;
    void *key;
    void *value;
    __rbt_node__ *left, *right, *parent;
};
struct __rbt_info__ {
    void *key_new, *value_new; // 构造回调
    void *key_del, *value_del; // 析构回调
    void *key_cmp;
    __rbt_node__ *root;
};

__rbt_info__ rbt_create(void *key_new, void *value_new, void *key_del, void *value_del, void *key_cmp) {
    __rbt_info__ info;
    info.key_new = key_new;
    info.value_new = value_new;
    info.key_del = key_del;
    info.value_del = value_del;
    info.key_cmp = key_cmp;
    return info;
}

int rbt_insert(__rbt_info__ *info, void *key, void *value) {

}
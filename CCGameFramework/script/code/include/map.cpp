//
// Project: clibparser
// Created by bajdcc
//

// 参考：https://www.cnblogs.com/skywang12345/p/3624291.html

// 红黑树

// 红色节点=0
// 黑色节点=1

// 成功=1
// 失败=0

#include "/include/memory"

struct __rbt_node__ {
    unsigned char color;
    void* key;
    void* value;
    __rbt_node__* left, * right, * parent;
};
struct __rbt_info__ {
    void* key_new, * value_new; // 构造回调
    void* key_del, * value_del; // 析构回调
    void* key_cmp;
    __rbt_node__* root;
};

__rbt_info__ rbt_create(void* key_new, void* value_new, void* key_del, void* value_del, void* key_cmp) {
    __rbt_info__ info;
    info.key_new = key_new;
    info.value_new = value_new;
    info.key_del = key_del;
    info.value_del = value_del;
    info.key_cmp = key_cmp;
    info.root = 0;
    return info;
}

__rbt_node__* rbt_search(__rbt_info__* info, __rbt_node__* x, void* key)
{
    if (!x)
        return x;
    int cmp = info->key_cmp(x->key, key);
    if (cmp == 0)
        return x;
    if (cmp > 0)
        return rbt_search(info, x->left, key);
    else
        return rbt_search(info, x->right, key);
}

/*
 * 对红黑树的节点(x)进行左旋转
 *
 * 左旋示意图(对节点x进行左旋)：
 *      px                              px
 *     /                               /
 *    x                               y
 *   /  \      --(左旋)-->           / \                #
 *  lx   y                          x  ry
 *     /   \                       /  \
 *    ly   ry                     lx  ly
 *
 *
 */
int rbt_left_rotate(__rbt_info__* info, __rbt_node__* x) {
    // 设置x的右孩子为y
    __rbt_node__* y = x->right;

    // 将 “y的左孩子” 设为 “x的右孩子”；
    // 如果y的左孩子非空，将 “x” 设为 “y的左孩子的父亲”
    x->right = y->left;
    if (y->left)
        y->left->parent = x;

    // 将 “x的父亲” 设为 “y的父亲”
    y->parent = x->parent;

    if (!x->parent) {
        info->root = y;            // 如果 “x的父亲” 是空节点，则将y设为根节点
    }
    else {
        if (x->parent->left == x)
            x->parent->left = y;    // 如果 x是它父节点的左孩子，则将y设为“x的父节点的左孩子”
        else
            x->parent->right = y;   // 如果 x是它父节点的左孩子，则将y设为“x的父节点的左孩子”
    }

    // 将 “x” 设为 “y的左孩子”
    y->left = x;
    // 将 “x的父节点” 设为 “y”
    x->parent = y;
}

/*
 * 对红黑树的节点(y)进行右旋转
 *
 * 右旋示意图(对节点y进行左旋)：
 *            py                               py
 *           /                                /
 *          y                                x
 *         /  \      --(右旋)-->            /  \                     #
 *        x   ry                           lx   y
 *       / \                                   / \                   #
 *      lx  rx                                rx  ry
 *
 */
int rbt_right_rotate(__rbt_info__* info, __rbt_node__* y) {
    // 设置y的左孩子为x
    __rbt_node__* x = y->left;

    // 将 “x的右孩子” 设为 “y的左孩子”；
    // 如果"x的右孩子"不为空的话，将 “y” 设为 “x的右孩子的父亲”
    y->left = x->right;
    if (x->right)
        x->right->parent = y;

    // 将 “y的父亲” 设为 “x的父亲”
    x->parent = y->parent;

    if (!y->parent) {
        info->root = x;            // 如果 “y的父亲” 是空节点，则将x设为根节点
    }
    else {
        if (y == y->parent->right)
            y->parent->right = x;    // 如果 y是它父节点的右孩子，则将x设为“y的父节点的右孩子”
        else
            y->parent->left = x;     // (y是它父节点的左孩子) 将x设为“x的父节点的左孩子”
    }

    // 将 “y” 设为 “x的右孩子”
    x->right = y;

    // 将 “y的父节点” 设为 “x”
    y->parent = x;
}

int rbt_insert_balance(__rbt_info__* info, __rbt_node__* node) {
    __rbt_node__* parent;

    // 若“父节点存在，并且父节点的颜色是红色”
    while ((parent = node->parent) != (void*)0 && parent->color == 0) {
        __rbt_node__* grandparent = parent->parent;

        //若“父节点”是“祖父节点的左孩子”
        if (parent == grandparent->left) {
            // Case 1条件：叔叔节点是红色
            {
                __rbt_node__* uncle = grandparent->right;
                if (uncle != (void*)0 && uncle->color == 0) {
                    uncle->color = 1;
                    parent->color = 1;
                    grandparent->color = 0;
                    node = grandparent;
                    continue;
                }
            }

            // Case 2条件：叔叔是黑色，且当前节点是右孩子
            if (parent->right == node) {
                __rbt_node__* tmp;
                rbt_left_rotate(info, parent);
                tmp = parent;
                parent = node;
                node = tmp;
            }

            // Case 3条件：叔叔是黑色，且当前节点是左孩子。
            parent->color = 1;
            grandparent->color = 0;
            rbt_right_rotate(info, grandparent);
        }
        else //若“父节点”是“祖父节点的右孩子”
        {
            // Case 1条件：叔叔节点是红色
            {
                __rbt_node__* uncle = grandparent->left;
                if (uncle != (void*)0 && uncle->color == 0) {
                    uncle->color = 1;
                    parent->color = 1;
                    grandparent->color = 0;
                    node = grandparent;
                    continue;
                }
            }

            // Case 2条件：叔叔是黑色，且当前节点是左孩子
            if (parent->left == node) {
                __rbt_node__* tmp;
                rbt_right_rotate(info, parent);
                tmp = parent;
                parent = node;
                node = tmp;
            }

            // Case 3条件：叔叔是黑色，且当前节点是右孩子。
            parent->color = 1;
            grandparent->color = 0;
            rbt_left_rotate(info, grandparent);
        }
    }

    // 将根节点设为黑色
    info->root->color = 1;
}

int rbt_insert(__rbt_info__* info, void* key, void* value) {
    __rbt_node__* x = info->root, * y = (void*)0;
    while (x) {
        y = x;
        if (info->key_cmp(x->key, key) > 0)
            x = x->left;
        else
            x = x->right;
    }
    __rbt_node__* node = malloc(sizeof(__rbt_node__));
    node->key = key;
    node->value = value;
    node->left = 0;
    node->right = 0;
    node->parent = y;
    node->color = 0;
    if (y) {
        if (info->key_cmp(node->key, y->key) < 0)
            y->left = node;
        else
            y->right = node;
    }
    else {
        info->root = node;
    }
    rbt_insert_balance(info, node);
}

int rbt_remove_balance(__rbt_info__* info, __rbt_node__* node, __rbt_node__* parent) {
    __rbt_node__* other;

    while ((!node || node->color == 1) && node != info->root)
    {
        if (parent->left == node)
        {
            other = parent->right;
            if (other->color == 0)
            {
                // Case 1: x的兄弟w是红色的  
                other->color == 1;
                parent->color = 0;
                rbt_left_rotate(info, parent);
                other = parent->right;
            }
            if ((!other->left || other->left->color == 1) &&
                (!other->right || other->right->color == 1))
            {
                // Case 2: x的兄弟w是黑色，且w的俩个孩子也都是黑色的  
                other->color = 0;
                node = parent;
                parent = node->parent;
            }
            else
            {
                if (!other->right || other->right->color == 1)
                {
                    // Case 3: x的兄弟w是黑色的，并且w的左孩子是红色，右孩子为黑色。  
                    other->left->color = 1;
                    other->color = 0;
                    rbt_right_rotate(info, other);
                    other = parent->right;
                }
                // Case 4: x的兄弟w是黑色的；并且w的右孩子是红色的，左孩子任意颜色。
                other->color = parent->color;
                parent->color = 1;
                other->right->color = 1;
                rbt_left_rotate(info, parent);
                node = info->root;
                break;
            }
        }
        else
        {
            other = parent->left;
            if (other->color == 0)
            {
                // Case 1: x的兄弟w是红色的  
                other->color == 1;
                parent->color = 0;
                rbt_right_rotate(info, parent);
                other = parent->left;
            }
            if ((!other->left || other->left->color == 1) &&
                (!other->right || other->right->color == 1))
            {
                // Case 2: x的兄弟w是黑色，且w的俩个孩子也都是黑色的  
                other->color = 0;
                node = parent;
                parent = node->parent;
            }
            else
            {
                if (!other->left || other->left->color == 1)
                {
                    // Case 3: x的兄弟w是黑色的，并且w的左孩子是红色，右孩子为黑色。  

                    other->right->color = 1;
                    other->color = 0;
                    rbt_left_rotate(info, other);
                    other = parent->left;
                }
                // Case 4: x的兄弟w是黑色的；并且w的右孩子是红色的，左孩子任意颜色。

                other->color = parent->color;
                parent->color = 1;
                other->left->color = 1;
                rbt_right_rotate(info, parent);
                node = info->root;
                break;
            }
        }
    }
    if (node)
        node->color = 1;
}

int rbt_remove_node(__rbt_info__* info, __rbt_node__* node) {
    __rbt_node__* child, * parent;
    int color;

    // 被删除节点的“左右孩子都不为空”的情况。
    if ((node->left) && (node->right)) {
        // 被删节点的后继节点。(称为“取代节点”)
        // 用它来取代“被删节点”的位置，然后再将“被删节点”去掉。
        __rbt_node__* replace = node;

        // 获取后继节点
        replace = replace->right;
        while (replace->left)
            replace = replace->left;

        // “node节点”不是根节点(只有根节点不存在父节点)
        if (node->parent) {
            if (node->parent->left == node)
                node->parent->left = replace;
            else
                node->parent->right = replace;
        }
        else
            // “node节点”是根节点，更新根节点。
            info->root = replace;

        // child是“取代节点”的右孩子，也是需要“调整的节点”。
        // “取代节点”肯定不存在左孩子！因为它是一个后继节点。
        child = replace->right;
        parent = replace->parent;
        // 保存“取代节点”的颜色
        color = replace->color;

        // “被删除节点”是“它的后继节点的父节点”
        if (parent == node) {
            parent = replace;
        }
        else {
            // child不为空
            if (child)
                child->parent = parent;
            parent->left = child;

            replace->right = node->right;
            node->right->parent = replace;
        }

        replace->parent = node->parent;
        replace->color = node->color;
        replace->left = node->left;
        node->left->parent = replace;
    }
    else {
        if (node->left)
            child = node->left;
        else
            child = node->right;

        parent = node->parent;
        // 保存"取代节点"的颜色
        color = node->color;

        if (child)
            child->parent = parent;

        // "node节点"不是根节点
        if (parent) {
            if (parent->left == node)
                parent->left = child;
            else
                parent->right = child;
        }
        else
            info->root = child;
    }
    if (color == 1)
        rbt_remove_balance(info, child, parent);
    free(node);
}

int rbt_remove(__rbt_info__* info, void* key) {
    __rbt_node__* node = rbt_search(info, info->root, key);
    if (node)
        rbt_remove_node(info, node);
}

int rbt_destroy_x(__rbt_node__* node) {
    if (node) {
        if (node->left) rbt_destroy_x(node->left);
        if (node->right) rbt_destroy_x(node->right);
        free(node);
    }
}

int rbt_destroy(__rbt_info__* info) {
    rbt_destroy_x(info->root);
    info->root = 0;
}

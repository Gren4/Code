#include "map.h"

#include <stdlib.h>
#include <string.h>
#include "util_funcs.h"

typedef enum node_color
{
    RED,
    BLACK
} node_color;

typedef struct map_node map_node;

typedef struct map_node
{
    map_node *parent;
    map_node *left;
    map_node *right;
    node_color color;
} map_node;

typedef struct map
{
    const size_t key_padding;
    const size_t key_size;
    const size_t data_size;
    map_node *root;
} map;

static inline void *at_key(const map *const m, void *const node)
{
    return node + sizeof(map_node) + m->data_size + m->key_padding;
}

static inline void *at_data(const map *const m, void *const node)
{
    return node + sizeof(map_node);
}

static inline int key_cmp(const void *const a, const void *const b, const size_t size)
{
    if (size == string_key_size)
        return strcmp(*(char **)a, b);
    else
        return memcmp(a, b, size);
}

static inline void key_cpy(void *const a, const void *const b, const size_t size)
{
    if (size == string_key_size)
    {
        char *string = strcpy(calloc(strlen(b) + 1, sizeof(char)), b);
        memcpy(a, &string, sizeof(char *));
    }
    else
        memcpy(a, b, size);
    return;
}

map_node *create_node(const map *const m, map_node *const parent, const node_color color, const void *const key, const void *const val)
{
    void *ptr = (void *)malloc(sizeof(map_node) + m->data_size + m->key_padding + (m->key_size != string_key_size ? m->key_size : sizeof(char *)));
    map_node *new_n = (map_node *)ptr;
    new_n->color = color;
    new_n->parent = parent;
    new_n->left = nullptr;
    new_n->right = nullptr;
    ptr += sizeof(map_node);
    memcpy(ptr, val, m->data_size);
    ptr += m->data_size;
    key_cpy(ptr, key, m->key_size);
    return new_n;
}

map *create_map(const size_t key_size, const size_t data_size)
{
    size_t real_key_size = key_size == string_key_size ? sizeof(char *) : key_size;
    map new_m = {
        .key_padding = (data_size + real_key_size) % real_key_size == 0 ? 0 : real_key_size - ((data_size + real_key_size) % real_key_size),
        .key_size = key_size,
        .data_size = data_size,
        .root = nullptr};
    return (map *)memcpy(malloc(sizeof(map)), &new_m, sizeof(map));
}

void free_map(map *const m)
{
    if (m->root != nullptr)
    {
        map_node *cur_node = m->root;
        for (; cur_node != nullptr;)
        {
            if (cur_node->left == nullptr && cur_node->right == nullptr)
            {
                if (cur_node->parent != nullptr)
                {
                    if (cur_node->parent->right == cur_node)
                    {
                        cur_node = cur_node->parent;
                        free(cur_node->right);
                        cur_node->right = nullptr;
                    }
                    else if (cur_node->parent->left == cur_node)
                    {
                        cur_node = cur_node->parent;
                        free(cur_node->left);
                        cur_node->left = nullptr;
                    }
                }
                else
                {
                    free(cur_node);
                    cur_node = nullptr;
                }
            }
            else if (cur_node->right != nullptr)
            {
                cur_node = cur_node->right;
            }
            else
            {
                cur_node = cur_node->left;
            }
        }
    }
    m->root = nullptr;
    return;
}

static void rotate_left(map_node *const node)
{
    map_node *pivot = node->right;
    pivot->parent = node->parent;
    if (node->parent != nullptr)
    {
        if (node->parent->left == node)
            node->parent->left = pivot;
        else
            node->parent->right = pivot;
    }
    node->right = pivot->left;
    if (pivot->left != nullptr)
        pivot->left->parent = node;
    node->parent = pivot;
    pivot->left = node;
    return;
}

static void rotate_right(map_node *const node)
{
    map_node *pivot = node->left;
    pivot->parent = node->parent;
    if (node->parent != nullptr)
    {
        if (node->parent->left == node)
            node->parent->left = pivot;
        else
            node->parent->right = pivot;
    }
    node->left = pivot->right;
    if (pivot->right != nullptr)
        pivot->right->parent = node;
    node->parent = pivot;
    pivot->right = node;
    return;
}

static map_node *get_grandparent(const map_node *const node)
{
    if (node != nullptr && node->parent != nullptr)
        return node->parent->parent;
    else
        return nullptr;
}

static map_node *get_uncle(const map_node *const node)
{
    map_node *g = get_grandparent(node);
    if (g == nullptr)
        return nullptr;
    if (node->parent == g->left)
        return g->right;
    else
        return g->left;
}

static map_node *get_sibling(const map_node *const node)
{
    if (node->parent == nullptr)
        return nullptr;
    if (node == node->parent->left)
        return node->parent->right;
    else
        return node->parent->left;
}

static void insert_case(map_node *node)
{
    for (;;)
    {
        if (node->parent == nullptr)
        {
            node->color = BLACK;
            return;
        }
        else
        {
            if (node->parent->color == BLACK)
                return;
            else
            {
                map_node *u = get_uncle(node);
                if ((u != nullptr) && (u->color == RED))
                {
                    node->parent->color = BLACK;
                    u->color = BLACK;
                    node = get_grandparent(node);
                    node->color = RED;
                }
                else
                {
                    map_node *g = get_grandparent(node);
                    if ((node == node->parent->right) && (node->parent == g->left))
                    {
                        rotate_left(node->parent);
                        node = node->left;
                    }
                    else if ((node == node->parent->left) && (node->parent == g->right))
                    {
                        rotate_right(node->parent);
                        node = node->right;
                    }
                    node->parent->color = BLACK;
                    g->color = RED;
                    if ((node == node->parent->left) && (node->parent == g->left))
                    {
                        rotate_right(g);
                    }
                    else
                    {
                        rotate_left(g);
                    }
                    return;
                }
            }
        }
    }
}

int set_map(map *const m, const void *const key, const void *const val)
{
    if (m->root == nullptr)
    {
        m->root = create_node(m, nullptr, BLACK, key, val);
        return 1;
    }
    else
    {
        map_node *cur_node = m->root;
        for (;;)
        {
            int key_cmp_res = key_cmp(at_key(m, (void *)cur_node), key, m->key_size);
            if (key_cmp_res == 0)
            {
                memcpy(at_data(m, (void *)cur_node), val, m->data_size);
                return 1;
            }
            else
            {
                if (key_cmp_res > 0)
                {
                    if (cur_node->left != nullptr)
                        cur_node = cur_node->left;
                    else
                    {
                        cur_node->left = create_node(m, cur_node, RED, key, val);
                        cur_node = cur_node->left;
                        insert_case(cur_node);
                        return 1;
                    }
                }
                else
                {
                    if (cur_node->right != nullptr)
                        cur_node = cur_node->right;
                    else
                    {
                        cur_node->right = create_node(m, cur_node, RED, key, val);
                        cur_node = cur_node->right;
                        insert_case(cur_node);
                        return 1;
                    }
                }
            }
        }
    }
}

static void replace_node(map_node *const node, map_node *const child)
{
    child->parent = node->parent;
    if (node == node->parent->left)
        node->parent->left = child;
    else
        node->parent->right = child;
    return;
}

static void delete_case(map_node *const node)
{
    map_node *child = node->right != nullptr ? node->left : node->right;
    replace_node(node, child);
    if (node->color == BLACK)
    {
        if (child->color == RED)
        {
            child->color = BLACK;
            free(node);
            return;
        }
        else
        {
            for (;;)
            {
                if (child->parent != nullptr)
                {
                    map_node *s = get_sibling(child);
                    if (s->color == RED)
                    {
                        child->parent->color = RED;
                        s->color = BLACK;
                        if (child == child->parent->left)
                            rotate_left(child->parent);
                        else
                            rotate_right(child->parent);
                    }
                    if (child->parent->color == BLACK && s->color == BLACK && s->left->color == BLACK && s->right->color == BLACK)
                    {
                        s->color = RED;
                        child = child->parent;
                    }
                    else
                    {
                        if (child->parent->color == RED && s->color == BLACK && s->left->color == BLACK && s->right->color == BLACK)
                        {
                            s->color = RED;
                            child->parent->color = BLACK;
                            free(node);
                            return;
                        }
                        else
                        {
                            if (s->color == BLACK)
                            {
                                if (child == child->parent->left && s->right->color == BLACK && s->left->color == RED)
                                {
                                    s->color = RED;
                                    s->left->color = BLACK;
                                    rotate_right(s);
                                }
                                else if (child == child->parent->right && s->left->color == BLACK && s->right->color == RED)
                                {
                                    s->color = RED;
                                    s->right->color = BLACK;
                                    rotate_left(s);
                                }
                            }
                            s->color = child->parent->color;
                            child->parent->color = BLACK;
                            if (child == child->parent->left)
                            {
                                s->right->color = BLACK;
                                rotate_left(child->parent);
                            }
                            else
                            {
                                s->left->color = BLACK;
                                rotate_right(child->parent);
                            }
                            free(node);
                            return;
                        }
                    }
                }
                else
                {
                    free(node);
                    return;
                }
            }
        }
    }
    else
    {
        free(node);
        return;
    }
}

int delete_map(map *const m, const void *const key, void *const val)
{
    if (m->root == nullptr)
    {
        return 0;
    }
    else
    {
        map_node *cur_node = m->root;
        for (;;)
        {
            int key_cmp_res = key_cmp(at_key(m, (void *)cur_node), key, m->key_size);
            if (key_cmp_res == 0)
            {
                if (val != nullptr)
                    memcpy(val, at_data(m, (void *)cur_node), m->data_size);
                delete_case(cur_node);
                return 1;
            }
            else
            {
                if (key_cmp_res > 0)
                {
                    if (cur_node->left != nullptr)
                        cur_node = cur_node->left;
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    if (cur_node->right != nullptr)
                        cur_node = cur_node->right;
                    else
                    {
                        return 0;
                    }
                }
            }
        }
    }
}

#include "rbt_map.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

#define RBT_MAP_MIN_SIZE 32

typedef enum rbt_node_color
{
    RED,
    BLACK
} rbt_node_color;

typedef struct rbt_node
{
    size_t left;
    size_t right;
    size_t parent;
    char *key;
    char *val;
    rbt_node_color color;
} rbt_node;

typedef struct rbt_node_pool
{
    size_t size;
    size_t count;
    rbt_node *nodes;
    char *used;
    char *keys;
    char *data;
    const type_func *keys_type;
    const type_func *data_type;
} rbt_node_pool;

typedef struct rbt_map
{
    rbt_node_pool pool;
    size_t root;
    size_t nil;
} rbt_map;

static int init_pool(rbt_node_pool *const pool, const size_t size)
{
    pool->nodes = (rbt_node *)calloc(size, sizeof(rbt_node));
    if (pool->nodes == NULL)
        return 0;
    pool->used = (char *)calloc(size, sizeof(char));
    if (pool->used == NULL)
    {
        free(pool->nodes);
        return 0;
    }
    pool->keys = (char *)calloc(size, pool->keys_type->t_size);
    if (pool->keys == NULL)
    {
        free(pool->nodes);
        free(pool->used);
        return 0;
    }
    pool->data = (char *)calloc(size, pool->data_type->t_size);
    if (pool->data == NULL)
    {
        free(pool->nodes);
        free(pool->used);
        free(pool->keys);
        return 0;
    }
    pool->size = size;
    pool->count = 1;
    return 1;
}

static int expand_pool(rbt_node_pool *const pool)
{
    size_t new_size = pool->size << 1;
    rbt_node *new_nodes = (rbt_node *)realloc(pool->nodes, new_size * sizeof(rbt_node));
    if (new_nodes == NULL)
        return 0;
    char *new_used = (char *)realloc(pool->used, new_size * sizeof(char));
    if (new_used == NULL)
    {
        free(new_nodes);
        return 0;
    }
    char *new_keys = (char *)realloc(pool->keys, new_size * pool->keys_type->t_size);
    if (new_keys == NULL)
    {
        free(new_nodes);
        free(new_used);
        return 0;
    }
    char *new_data = (char *)realloc(pool->data, new_size * pool->data_type->t_size);
    if (new_data == NULL)
    {
        free(new_nodes);
        free(new_used);
        free(new_keys);
        return 0;
    }
    memset(new_nodes + pool->size, 0, (new_size - pool->size) * sizeof(rbt_node));
    memset(new_used + pool->size, 0, (new_size - pool->size) * sizeof(char));
    memset(new_keys + pool->size * pool->keys_type->t_size, 0, (new_size - pool->size) * pool->keys_type->t_size);
    memset(new_data + pool->size * pool->data_type->t_size, 0, (new_size - pool->size) * pool->data_type->t_size);
    pool->nodes = new_nodes;
    pool->keys = new_keys;
    pool->data = new_data;
    pool->size = new_size;
    return 1;
}

static int shrink_pool(rbt_node_pool *const pool)
{
    pool->count--;
    return 1;
}

static size_t get_unused_node(rbt_node_pool *const pool)
{
    ssize_t i = 1;
    for (; i < pool->size; i++)
    {
        if (pool->used[i] == 0)
            return i; 
    }
    return pool->size;
}

static int allocate_node(rbt_node_pool *const pool, const size_t index, const void *const key, const void *const val)
{
    if (pool->count >= pool->size && expand_pool(pool) == 0)
        return 0;
    pool->count++;
    rbt_node *node = pool->nodes + index;
    node->left = 0;
    node->right = 0;
    node->parent = 0;
    node->val = pool->data_type->t_at(pool->data, index);
    node->key = pool->keys_type->t_at(pool->keys, index);
    pool->data_type->t_cpy(node->val, val);
    pool->keys_type->t_cpy(node->key, key);
    node->color = RED;
    pool->used[index] = 1;
    return 1;
}

static int free_node(rbt_node_pool *const pool, const size_t index, void* const val)
{
    if (index >= pool->size)
        return 0;
    if (val != NULL)
        pool->data_type->t_cpy(val, pool->nodes[index].val);
    if (pool->keys_type->t_free != NULL)
        pool->keys_type->t_free(pool->nodes[index].key);
    if (pool->data_type->t_free != NULL)
        pool->data_type->t_free(pool->nodes[index].val);
    pool->nodes[index].left = 0;
    pool->nodes[index].right = 0;
    pool->nodes[index].parent = 0;
    pool->nodes[index].key = NULL;
    pool->nodes[index].val = NULL;
    pool->nodes[index].color = RED;
    pool->used[index] = 0;
    return shrink_pool(pool);
}

rbt_map *create_rbt_map(const size_t size, const type_func *const keys_type, const type_func *const data_type)
{
    size_t mul_of_2_size = size < RBT_MAP_MIN_SIZE ? RBT_MAP_MIN_SIZE : next_power_of_2(size);
    rbt_map new_m = {
        .root = 0,
        .nil = 0,
        .pool = {
            .keys_type = keys_type,
            .data_type = data_type}};
    if (init_pool(&new_m.pool, mul_of_2_size) == 0)
        return NULL;
    rbt_node *nil_node = new_m.pool.nodes;
    nil_node->color = BLACK;
    nil_node->left = 0;
    nil_node->right = 0;
    nil_node->parent = 0;
    return (rbt_map *)memcpy(malloc(sizeof(rbt_map)), &new_m, sizeof(rbt_map));
}

// Очистка дерева
void free_rbt_map(rbt_map *m)
{
    if (m->pool.nodes != NULL)
        free(m->pool.nodes);
    if (m->pool.keys != NULL)
    {
        if (m->pool.keys_type->t_free != NULL && m->pool.count > 0)
        {
            ssize_t i = 0;
            for (; i < m->pool.size; i++)
            {
                if (m->pool.used[i] == 1)
                    m->pool.keys_type->t_free(m->pool.keys_type->t_at(m->pool.keys, i));
            }
        }
        free(m->pool.keys);
    }
    if (m->pool.data != NULL)
    {
        if (m->pool.data_type->t_free != NULL && m->pool.count > 0)
        {
            ssize_t i = 0;
            for (; i < m->pool.size; i++)
            {
                if (m->pool.used[i] == 1)
                    m->pool.data_type->t_free(m->pool.data_type->t_at(m->pool.data, i));
            }
        }
        free(m->pool.data);
    }
    free(m->pool.used);
    m->pool.nodes = NULL;
    m->pool.keys = NULL;
    m->pool.data = NULL;
    m->pool.size = 0;
    m->pool.count = 0;
    m->root = m->nil;
    return;
}

// Получение узла по индексу
static rbt_node *node_from_pool(const rbt_map *const m, const size_t index)
{
    return m->pool.nodes + index;
}

// Поиск узла по значению
size_t has_rbt_map(const rbt_map *const m, const void *const key)
{
    int current = m->root;
    while (current != m->nil)
    {
        rbt_node *node = node_from_pool(m, current);
        int cmp_result = m->pool.keys_type->t_cmp(node->key, key);
        if (cmp_result == 0)
        {
            return current;
        }
        else if (cmp_result == 1)
        {
            current = node->left;
        }
        else
        {
            current = node->right;
        }
    }
    return m->nil;
}

// Левое вращение
static void left_rotate(rbt_map *const m, const size_t x_index)
{
    rbt_node *x = node_from_pool(m, x_index);
    int y_index = x->right;
    rbt_node *y = node_from_pool(m, y_index);
    x->right = y->left;
    if (y->left != m->nil)
    {
        node_from_pool(m, y->left)->parent = x_index;
    }
    y->parent = x->parent;
    if (x->parent == m->nil)
    {
        m->root = y_index;
    }
    else if (x_index == node_from_pool(m, x->parent)->left)
    {
        node_from_pool(m, x->parent)->left = y_index;
    }
    else
    {
        node_from_pool(m, x->parent)->right = y_index;
    }
    y->left = x_index;
    x->parent = y_index;
}

// Правое вращение (симметрично)
static void right_rotate(rbt_map *const m, const size_t y_index)
{
    rbt_node *y = node_from_pool(m, y_index);
    int x_index = y->left;
    rbt_node *x = node_from_pool(m, x_index);
    y->left = x->right;
    if (x->right != m->nil)
    {
        node_from_pool(m, x->right)->parent = y_index;
    }
    x->parent = y->parent;
    if (y->parent == m->nil)
    {
        m->root = x_index;
    }
    else if (y_index == node_from_pool(m, y->parent)->right)
    {
        node_from_pool(m, y->parent)->right = x_index;
    }
    else
    {
        node_from_pool(m, y->parent)->left = x_index;
    }
    x->right = y_index;
    y->parent = x_index;
}

// Балансировка после вставки
static void insert_fixup(rbt_map *const m, size_t z_index)
{
    rbt_node *z = node_from_pool(m, z_index);
    while (node_from_pool(m, z->parent)->color == RED)
    {
        if (z->parent == node_from_pool(m, node_from_pool(m, z->parent)->parent)->left)
        {
            int uncle_index = node_from_pool(m, node_from_pool(m, z->parent)->parent)->right;
            if (node_from_pool(m, uncle_index)->color == RED)
            {
                node_from_pool(m, z->parent)->color = BLACK;
                node_from_pool(m, uncle_index)->color = BLACK;
                node_from_pool(m, node_from_pool(m, z->parent)->parent)->color = RED;
                z_index = node_from_pool(m, z->parent)->parent;
                z = node_from_pool(m, z_index);
            }
            else
            {
                if (z_index == node_from_pool(m, z->parent)->right)
                {
                    z_index = z->parent;
                    left_rotate(m, z_index);
                    z = node_from_pool(m, z_index);
                }
                node_from_pool(m, z->parent)->color = BLACK;
                node_from_pool(m, node_from_pool(m, z->parent)->parent)->color = RED;
                right_rotate(m, node_from_pool(m, z->parent)->parent);
            }
        }
        else
        {
            // Симметричный случай (правая сторона)
            int uncle_index = node_from_pool(m, node_from_pool(m, z->parent)->parent)->left;
            if (node_from_pool(m, uncle_index)->color == RED)
            {
                node_from_pool(m, z->parent)->color = BLACK;
                node_from_pool(m, uncle_index)->color = BLACK;
                node_from_pool(m, node_from_pool(m, z->parent)->parent)->color = RED;
                z_index = node_from_pool(m, z->parent)->parent;
                z = node_from_pool(m, z_index);
            }
            else
            {
                if (z_index == node_from_pool(m, z->parent)->left)
                {
                    z_index = z->parent;
                    right_rotate(m, z_index);
                    z = node_from_pool(m, z_index);
                }
                node_from_pool(m, z->parent)->color = BLACK;
                node_from_pool(m, node_from_pool(m, z->parent)->parent)->color = RED;
                left_rotate(m, node_from_pool(m, z->parent)->parent);
            }
        }
    }
    node_from_pool(m, m->root)->color = BLACK;
}

// Вставка узла
int set_rbt_map(rbt_map *const m, const void *const key, const void *const val)
{
    size_t z_index = get_unused_node(&m->pool);
    if (z_index == m->pool.size)
        return 0;
    size_t y = m->nil;
    size_t x = m->root;
    while (x != m->nil)
    {
        y = x;
        rbt_node *x_node = node_from_pool(m, x);
        int cmp_result = m->pool.keys_type->t_cmp(key, x_node->key);
        if (cmp_result == 0)
        {
            m->pool.data_type->t_cpy(x_node->val, val);
            return 1;
        }
        else
        {
            if (cmp_result == -1)
            {
                x = x_node->left;
            }
            else
            {
                x = x_node->right;
            }
        }
    }
    if (allocate_node(&m->pool, z_index, key, val) == 0)
        return 0;
    rbt_node *z = node_from_pool(m, z_index);
    z->parent = y;
    if (y == m->nil)
    {
        m->root = z_index;
    }
    else
    {
        rbt_node *y_node = node_from_pool(m, y);
        int cmp_result = m->pool.keys_type->t_cmp(z->key, y_node->key);
        if (cmp_result == -1)
        {
            y_node->left = z_index;
        }
        else
        {
            y_node->right = z_index;
        }
    }
    z->left = m->nil;
    z->right = m->nil;
    z->color = RED;
    insert_fixup(m, z_index);
    return 1;
}

// Поиск минимального узла в поддереве
static int minimum_node(rbt_map *const m, size_t x_index)
{
    rbt_node *x = node_from_pool(m, x_index);
    while (x->left != m->nil)
    {
        x_index = x->left;
        x = node_from_pool(m, x_index);
    }
    return x_index;
}

// Перенос узла (для удаления)
static void transplant_node(rbt_map *const m, const size_t u_index, const size_t v_index)
{
    rbt_node *u = node_from_pool(m, u_index);
    if (u->parent == m->nil)
    {
        m->root = v_index;
    }
    else if (u_index == node_from_pool(m, u->parent)->left)
    {
        node_from_pool(m, u->parent)->left = v_index;
    }
    else
    {
        node_from_pool(m, u->parent)->right = v_index;
    }
    node_from_pool(m, v_index)->parent = u->parent;
    return;
}

// Балансировка после удаления
static void delete_fixup(rbt_map *const m, size_t x_index)
{
    while (x_index != m->root && node_from_pool(m, x_index)->color == BLACK)
    {
        rbt_node *x = node_from_pool(m, x_index);
        if (x_index == node_from_pool(m, x->parent)->left)
        {
            int wIndex = node_from_pool(m, x->parent)->right;
            rbt_node *w = node_from_pool(m, wIndex);
            if (w->color == RED)
            {
                w->color = BLACK;
                node_from_pool(m, x->parent)->color = RED;
                left_rotate(m, x->parent);
                wIndex = node_from_pool(m, x->parent)->right;
                w = node_from_pool(m, wIndex);
            }
            if (node_from_pool(m, w->left)->color == BLACK && node_from_pool(m, w->right)->color == BLACK)
            {
                w->color = RED;
                x_index = x->parent;
            }
            else
            {
                if (node_from_pool(m, w->right)->color == BLACK)
                {
                    node_from_pool(m, w->left)->color = BLACK;
                    w->color = RED;
                    right_rotate(m, wIndex);
                    wIndex = node_from_pool(m, x->parent)->right;
                    w = node_from_pool(m, wIndex);
                }
                w->color = node_from_pool(m, x->parent)->color;
                node_from_pool(m, x->parent)->color = BLACK;
                node_from_pool(m, w->right)->color = BLACK;
                left_rotate(m, x->parent);
                x_index = m->root;
            }
        }
        else
        {
            // Симметричный случай (правая сторона)
            int wIndex = node_from_pool(m, x->parent)->left;
            rbt_node *w = node_from_pool(m, wIndex);
            if (w->color == RED)
            {
                w->color = BLACK;
                node_from_pool(m, x->parent)->color = RED;
                right_rotate(m, x->parent);
                wIndex = node_from_pool(m, x->parent)->left;
                w = node_from_pool(m, wIndex);
            }
            if (node_from_pool(m, w->right)->color == BLACK && node_from_pool(m, w->left)->color == BLACK)
            {
                w->color = RED;
                x_index = x->parent;
            }
            else
            {
                if (node_from_pool(m, w->left)->color == BLACK)
                {
                    node_from_pool(m, w->right)->color = BLACK;
                    w->color = RED;
                    left_rotate(m, wIndex);
                    wIndex = node_from_pool(m, x->parent)->left;
                    w = node_from_pool(m, wIndex);
                }
                w->color = node_from_pool(m, x->parent)->color;
                node_from_pool(m, x->parent)->color = BLACK;
                node_from_pool(m, w->left)->color = BLACK;
                right_rotate(m, x->parent);
                x_index = m->root;
            }
        }
    }
    node_from_pool(m, x_index)->color = BLACK;
    return;
}

// Удаление узла
int delete_rbt_map(rbt_map *m, const void *const key, void *const val)
{
    int z_index = has_rbt_map(m, key);
    if (z_index == m->nil)
        return 0;
    rbt_node *z = node_from_pool(m, z_index);
    int y_index = z_index;
    int x_index;
    rbt_node_color y_original_color = z->color;
    if (z->left == m->nil)
    {
        x_index = z->right;
        transplant_node(m, z_index, x_index);
    }
    else if (z->right == m->nil)
    {
        x_index = z->left;
        transplant_node(m, z_index, x_index);
    }
    else
    {
        y_index = minimum_node(m, z->right);
        y_original_color = node_from_pool(m, y_index)->color;
        x_index = node_from_pool(m, y_index)->right;
        if (node_from_pool(m, y_index)->parent == z_index)
        {
            node_from_pool(m, x_index)->parent = y_index;
        }
        else
        {
            transplant_node(m, y_index, x_index);
            node_from_pool(m, y_index)->right = z->right;
            node_from_pool(m, node_from_pool(m, y_index)->right)->parent = y_index;
        }
        transplant_node(m, z_index, y_index);
        node_from_pool(m, y_index)->left = z->left;
        node_from_pool(m, node_from_pool(m, y_index)->left)->parent = y_index;
        node_from_pool(m, y_index)->color = z->color;
    }
    if (y_original_color == BLACK)
        delete_fixup(m, x_index);
    free_node(&m->pool, z_index, val);
    return 1;
}

// // Вывод дерева (для отладки)
// void printTreeHelper(rbt_map *m, int nodeIndex, int indent)
// {
//     if (nodeIndex == m->nil)
//         return;

//     printTreeHelper(m, node_from_pool(m, nodeIndex)->right, indent + 4);

//     for (int i = 0; i < indent; i++)
//         printf(" ");
//     rbt_node *node = node_from_pool(m, nodeIndex);
//     printf("%d(%c)\n", node->data, (node->color == RED ? 'R' : 'B'));

//     printTreeHelper(m, node_from_pool(m, nodeIndex)->left, indent + 4);
// }

// void printTree(rbt_map *m)
// {
//     printTreeHelper(m, m->root, 0);
// }

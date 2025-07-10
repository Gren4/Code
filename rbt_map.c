#include "rbt_map.h"
#include "queue.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

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

static int init_pool(rbt_node_pool *pool, size_t size)
{
    pool->nodes = (rbt_node *)calloc(size, sizeof(rbt_node));
    pool->keys = (char *)calloc(size, pool->keys_type->t_size);
    pool->data = (char *)calloc(size, pool->data_type->t_size);
    pool->size = size;
    pool->count = 0;
    return 1;
}

static int expand_pool(rbt_node_pool *pool)
{
    size_t new_size = pool->size << 1;
    rbt_node *new_nodes = (rbt_node *)realloc(pool->nodes, new_size * sizeof(rbt_node));
    if (new_nodes == NULL)
        return 0;
    char *new_keys = (char *)realloc(pool->keys, new_size * pool->keys_type->t_size);
    if (new_keys == NULL)
    {
        free(new_nodes);
        return 0;
    }
    char *new_data = (char *)realloc(pool->data, new_size * pool->data_type->t_size);
    if (new_data == NULL)
    {
        free(new_nodes);
        free(new_keys);
        return 0;
    }
    memset(new_nodes + pool->size, 0, (new_size - pool->size) * sizeof(rbt_node));
    memset(new_keys + pool->size * pool->keys_type->t_size, 0, (new_size - pool->size) * pool->keys_type->t_size);
    memset(new_data + pool->size * pool->data_type->t_size, 0, (new_size - pool->size) * pool->data_type->t_size);
    pool->nodes = new_nodes;
    pool->keys = new_keys;
    pool->data = new_data;
    pool->size = new_size;
    return 1;
}

static int shrink_pool(rbt_node_pool *pool)
{
    pool->count--;
    return 1;
}

static int allocate_node(rbt_node_pool *pool, void *key, void *val)
{
    if (pool->count == pool->size && expand_pool(pool) == 0)
        return pool->size;
    ssize_t i = 1;
    for (; i < pool->size; i++)
    {
        if (pool->nodes[i].key == NULL || pool->nodes[i].val == NULL)
        {
            pool->count++;
            rbt_node *node = pool->nodes + i;
            node->val = pool->data + i;
            pool->data_type->t_cpy(node->val, val);
            pool->keys_type->t_cpy(node->key, key);
            node->color = RED;
            node->left = 0;
            node->right = 0;
            node->parent = 0;
            return i;
        }
    }
    return pool->size;
}

static int free_node(rbt_node_pool *pool, size_t index)
{
    if (index >= pool->size)
        return 0;
    pool->nodes[index].key = NULL;
    pool->nodes[index].val = NULL;
    return shrink_pool(pool);
}

rbt_map *create_rbt_map(const size_t size, const type_func *keys_type, const type_func *data_type)
{
    rbt_map tree = {
        .root = 0,
        .nil = 0,
        .pool = {
            .keys_type = keys_type,
            .data_type = data_type}};
    init_pool(&tree.pool, size);
    rbt_node *nilNode = &tree.pool.nodes[0];
    nilNode->color = BLACK;
    nilNode->left = 0;
    nilNode->right = 0;
    nilNode->parent = 0;
    return (rbt_map *)memcpy(malloc(sizeof(rbt_map)), &tree, sizeof(rbt_map));
}

// Получение узла по индексу
static rbt_node *node_from_pool(rbt_map *tree, int index)
{
    return tree->pool.nodes + index;
}

// Поиск узла по значению
size_t has_rbt_map(rbt_map *tree, int data)
{
    int current = tree->root;
    while (current != tree->nil)
    {
        rbt_node *node = node_from_pool(tree, current);
        if (data == node->val)
        {
            return current;
        }
        else if (data < node->val)
        {
            current = node->left;
        }
        else
        {
            current = node->right;
        }
    }
    return tree->nil;
}

// Левое вращение
static void left_rotate(rbt_map *tree, int xIndex)
{
    rbt_node *x = node_from_pool(tree, xIndex);
    int yIndex = x->right;
    rbt_node *y = node_from_pool(tree, yIndex);
    x->right = y->left;
    if (y->left != tree->nil)
    {
        node_from_pool(tree, y->left)->parent = xIndex;
    }
    y->parent = x->parent;
    if (x->parent == tree->nil)
    {
        tree->root = yIndex;
    }
    else if (xIndex == node_from_pool(tree, x->parent)->left)
    {
        node_from_pool(tree, x->parent)->left = yIndex;
    }
    else
    {
        node_from_pool(tree, x->parent)->right = yIndex;
    }
    y->left = xIndex;
    x->parent = yIndex;
}

// Правое вращение (симметрично)
static void right_rotate(rbt_map *tree, int yIndex)
{
    rbt_node *y = node_from_pool(tree, yIndex);
    int xIndex = y->left;
    rbt_node *x = node_from_pool(tree, xIndex);
    y->left = x->right;
    if (x->right != tree->nil)
    {
        node_from_pool(tree, x->right)->parent = yIndex;
    }
    x->parent = y->parent;
    if (y->parent == tree->nil)
    {
        tree->root = xIndex;
    }
    else if (yIndex == node_from_pool(tree, y->parent)->right)
    {
        node_from_pool(tree, y->parent)->right = xIndex;
    }
    else
    {
        node_from_pool(tree, y->parent)->left = xIndex;
    }
    x->right = yIndex;
    y->parent = xIndex;
}

// Балансировка после вставки
static void insert_fixup(rbt_map *tree, int zIndex)
{
    rbt_node *z = node_from_pool(tree, zIndex);
    while (node_from_pool(tree, z->parent)->color == RED)
    {
        if (z->parent == node_from_pool(tree, node_from_pool(tree, z->parent)->parent)->left)
        {
            int uncleIndex = node_from_pool(tree, node_from_pool(tree, z->parent)->parent)->right;
            if (node_from_pool(tree, uncleIndex)->color == RED)
            {
                node_from_pool(tree, z->parent)->color = BLACK;
                node_from_pool(tree, uncleIndex)->color = BLACK;
                node_from_pool(tree, node_from_pool(tree, z->parent)->parent)->color = RED;
                zIndex = node_from_pool(tree, z->parent)->parent;
                z = node_from_pool(tree, zIndex);
            }
            else
            {
                if (zIndex == node_from_pool(tree, z->parent)->right)
                {
                    zIndex = z->parent;
                    left_rotate(tree, zIndex);
                    z = node_from_pool(tree, zIndex);
                }
                node_from_pool(tree, z->parent)->color = BLACK;
                node_from_pool(tree, node_from_pool(tree, z->parent)->parent)->color = RED;
                right_rotate(tree, node_from_pool(tree, z->parent)->parent);
            }
        }
        else
        {
            // Симметричный случай (правая сторона)
            int uncleIndex = node_from_pool(tree, node_from_pool(tree, z->parent)->parent)->left;
            if (node_from_pool(tree, uncleIndex)->color == RED)
            {
                node_from_pool(tree, z->parent)->color = BLACK;
                node_from_pool(tree, uncleIndex)->color = BLACK;
                node_from_pool(tree, node_from_pool(tree, z->parent)->parent)->color = RED;
                zIndex = node_from_pool(tree, z->parent)->parent;
                z = node_from_pool(tree, zIndex);
            }
            else
            {
                if (zIndex == node_from_pool(tree, z->parent)->left)
                {
                    zIndex = z->parent;
                    right_rotate(tree, zIndex);
                    z = node_from_pool(tree, zIndex);
                }
                node_from_pool(tree, z->parent)->color = BLACK;
                node_from_pool(tree, node_from_pool(tree, z->parent)->parent)->color = RED;
                left_rotate(tree, node_from_pool(tree, z->parent)->parent);
            }
        }
    }
    node_from_pool(tree, tree->root)->color = BLACK;
}

// Вставка узла
int set_rbt_map(rbt_map *tree, void *key, void *val)
{
    int zIndex = allocate_node(&tree->pool, key, val);
    if (zIndex == tree->pool.size)
    {
        return 0;
    }
    rbt_node *z = node_from_pool(tree, zIndex);
    int y = tree->nil;
    int x = tree->root;
    while (x != tree->nil)
    {
        y = x;
        rbt_node *xNode = node_from_pool(tree, x);
        if (z->val < xNode->val)
        {
            x = xNode->left;
        }
        else
        {
            x = xNode->right;
        }
    }
    z->parent = y;
    if (y == tree->nil)
    {
        tree->root = zIndex;
    }
    else if (z->val < node_from_pool(tree, y)->val)
    {
        node_from_pool(tree, y)->left = zIndex;
    }
    else
    {
        node_from_pool(tree, y)->right = zIndex;
    }
    z->left = tree->nil;
    z->right = tree->nil;
    z->color = RED;
    insert_fixup(tree, zIndex);
    return 1;
}

// Поиск минимального узла в поддереве
static int minimum_node(rbt_map *tree, int xIndex)
{
    rbt_node *x = node_from_pool(tree, xIndex);
    while (x->left != tree->nil)
    {
        xIndex = x->left;
        x = node_from_pool(tree, xIndex);
    }
    return xIndex;
}

// Перенос узла (для удаления)
static void transplant_node(rbt_map *tree, int uIndex, int vIndex)
{
    rbt_node *u = node_from_pool(tree, uIndex);
    if (u->parent == tree->nil)
    {
        tree->root = vIndex;
    }
    else if (uIndex == node_from_pool(tree, u->parent)->left)
    {
        node_from_pool(tree, u->parent)->left = vIndex;
    }
    else
    {
        node_from_pool(tree, u->parent)->right = vIndex;
    }
    node_from_pool(tree, vIndex)->parent = u->parent;
    return;
}

// Балансировка после удаления
static void delete_fixup(rbt_map *tree, int xIndex)
{
    while (xIndex != tree->root && node_from_pool(tree, xIndex)->color == BLACK)
    {
        rbt_node *x = node_from_pool(tree, xIndex);
        if (xIndex == node_from_pool(tree, x->parent)->left)
        {
            int wIndex = node_from_pool(tree, x->parent)->right;
            rbt_node *w = node_from_pool(tree, wIndex);
            if (w->color == RED)
            {
                w->color = BLACK;
                node_from_pool(tree, x->parent)->color = RED;
                left_rotate(tree, x->parent);
                wIndex = node_from_pool(tree, x->parent)->right;
                w = node_from_pool(tree, wIndex);
            }
            if (node_from_pool(tree, w->left)->color == BLACK && node_from_pool(tree, w->right)->color == BLACK)
            {
                w->color = RED;
                xIndex = x->parent;
            }
            else
            {
                if (node_from_pool(tree, w->right)->color == BLACK)
                {
                    node_from_pool(tree, w->left)->color = BLACK;
                    w->color = RED;
                    right_rotate(tree, wIndex);
                    wIndex = node_from_pool(tree, x->parent)->right;
                    w = node_from_pool(tree, wIndex);
                }
                w->color = node_from_pool(tree, x->parent)->color;
                node_from_pool(tree, x->parent)->color = BLACK;
                node_from_pool(tree, w->right)->color = BLACK;
                left_rotate(tree, x->parent);
                xIndex = tree->root;
            }
        }
        else
        {
            // Симметричный случай (правая сторона)
            int wIndex = node_from_pool(tree, x->parent)->left;
            rbt_node *w = node_from_pool(tree, wIndex);
            if (w->color == RED)
            {
                w->color = BLACK;
                node_from_pool(tree, x->parent)->color = RED;
                right_rotate(tree, x->parent);
                wIndex = node_from_pool(tree, x->parent)->left;
                w = node_from_pool(tree, wIndex);
            }
            if (node_from_pool(tree, w->right)->color == BLACK && node_from_pool(tree, w->left)->color == BLACK)
            {
                w->color = RED;
                xIndex = x->parent;
            }
            else
            {
                if (node_from_pool(tree, w->left)->color == BLACK)
                {
                    node_from_pool(tree, w->right)->color = BLACK;
                    w->color = RED;
                    left_rotate(tree, wIndex);
                    wIndex = node_from_pool(tree, x->parent)->left;
                    w = node_from_pool(tree, wIndex);
                }
                w->color = node_from_pool(tree, x->parent)->color;
                node_from_pool(tree, x->parent)->color = BLACK;
                node_from_pool(tree, w->left)->color = BLACK;
                right_rotate(tree, x->parent);
                xIndex = tree->root;
            }
        }
    }
    node_from_pool(tree, xIndex)->color = BLACK;
    return;
}

// Удаление узла
int delete_rbt_map(rbt_map *tree, int data)
{
    int zIndex = has_rbt_map(tree, data);
    if (zIndex == tree->nil)
        return 0;
    rbt_node *z = node_from_pool(tree, zIndex);
    int yIndex = zIndex;
    int xIndex;
    rbt_node_color yOriginalColor = z->color;
    if (z->left == tree->nil)
    {
        xIndex = z->right;
        transplant_node(tree, zIndex, xIndex);
    }
    else if (z->right == tree->nil)
    {
        xIndex = z->left;
        transplant_node(tree, zIndex, xIndex);
    }
    else
    {
        yIndex = minimum_node(tree, z->right);
        yOriginalColor = node_from_pool(tree, yIndex)->color;
        xIndex = node_from_pool(tree, yIndex)->right;
        if (node_from_pool(tree, yIndex)->parent == zIndex)
        {
            node_from_pool(tree, xIndex)->parent = yIndex;
        }
        else
        {
            transplant_node(tree, yIndex, xIndex);
            node_from_pool(tree, yIndex)->right = z->right;
            node_from_pool(tree, node_from_pool(tree, yIndex)->right)->parent = yIndex;
        }
        transplant_node(tree, zIndex, yIndex);
        node_from_pool(tree, yIndex)->left = z->left;
        node_from_pool(tree, node_from_pool(tree, yIndex)->left)->parent = yIndex;
        node_from_pool(tree, yIndex)->color = z->color;
    }
    if (yOriginalColor == BLACK)
        delete_fixup(tree, xIndex);
    free_node(&tree->pool, zIndex);
    return 1;
}

// // Вывод дерева (для отладки)
// void printTreeHelper(rbt_map *tree, int nodeIndex, int indent)
// {
//     if (nodeIndex == tree->nil)
//         return;

//     printTreeHelper(tree, node_from_pool(tree, nodeIndex)->right, indent + 4);

//     for (int i = 0; i < indent; i++)
//         printf(" ");
//     rbt_node *node = node_from_pool(tree, nodeIndex);
//     printf("%d(%c)\n", node->data, (node->color == RED ? 'R' : 'B'));

//     printTreeHelper(tree, node_from_pool(tree, nodeIndex)->left, indent + 4);
// }

// void printTree(rbt_map *tree)
// {
//     printTreeHelper(tree, tree->root, 0);
// }

// Очистка дерева
void destroyTree(rbt_map *tree)
{
    free(tree->pool.nodes);
    free(tree->pool.keys);
    free(tree->pool.data);
    tree->pool.nodes = NULL;
    tree->pool.keys = NULL;
    tree->pool.data = NULL;
    tree->pool.size = 0;
    tree->pool.count = 0;
    tree->root = tree->nil;
    return;
}

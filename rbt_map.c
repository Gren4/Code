#include "rbt_map.h"
#include "shared_ptr.h"
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
    rbt_node_color color;
    char used;
} rbt_node;

typedef struct rbt_node_pool
{
    size_t size;
    size_t count;
    size_t least_unused;
    const type_func *keys_type;
    const type_func *container_type;
    shared_ptr nodes;
    shared_ptr keys;
    shared_ptr container;
} rbt_node_pool;

typedef struct rbt_map
{
    rbt_node_pool pool;
    size_t root;
    size_t nil;
} rbt_map;

static int init_pool(rbt_node_pool *const pool, const size_t size)
{
    pool->nodes = calloc_shared_ptr(size, sizeof(rbt_node));
    if (pool->nodes == NULL)
        return 0;
    pool->keys = calloc_shared_ptr(size, pool->keys_type->t_size);
    if (pool->keys == NULL)
    {
        free_shared_ptr(pool->nodes);
        return 0;
    }
    pool->container = calloc_shared_ptr(size, pool->container_type->t_size);
    if (pool->container == NULL)
    {
        free_shared_ptr(pool->nodes);
        free_shared_ptr(pool->keys);
        return 0;
    }
    pool->size = size;
    pool->count = 1;
    return 1;
}

static int expand_pool(rbt_node_pool *const pool)
{
    if (++pool->count < pool->size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(pool->size << 1);
    shared_ptr new_nodes = realloc_shared_ptr(pool->nodes, mul_of_2_size, sizeof(rbt_node));
    if (new_nodes == NULL)
        return 0;
    shared_ptr new_keys = realloc_shared_ptr(pool->keys, mul_of_2_size, pool->keys_type->t_size);
    if (new_keys == NULL)
    {
        free_shared_ptr(new_nodes);
        return 0;
    }
    shared_ptr new_container = realloc_shared_ptr(pool->container, mul_of_2_size, pool->container_type->t_size);
    if (new_container == NULL)
    {
        free_shared_ptr(new_nodes);
        free_shared_ptr(new_keys);
        return 0;
    }
    rbt_node *nodes_data = data_shared_ptr(new_nodes);
    void *keys_data = data_shared_ptr(new_keys);
    void *container_data = data_shared_ptr(new_container);
    size_t elements = mul_of_2_size - pool->size;
    memset(nodes_data + pool->size, 0, elements * sizeof(rbt_node));
    memset(pool->keys_type->t_at(keys_data, pool->size), 0, elements * pool->keys_type->t_size);
    memset(pool->container_type->t_at(container_data, pool->size), 0, elements * pool->container_type->t_size);
    pool->nodes = new_nodes;
    pool->keys = new_keys;
    pool->container = new_container;
    pool->size = mul_of_2_size;
    return 1;
}

static int shrink_pool(rbt_map *const m)
{
    if (--m->pool.count > m->pool.size >> 3)
        return 1;
    size_t mul_of_2_size = next_power_of_2(m->pool.count);
    if (mul_of_2_size <= RBT_MAP_MIN_SIZE)
        return 1;
    size_t i = 1;
    size_t j = 1;
    rbt_node *nodes_data = data_shared_ptr(m->pool.nodes);
    void *keys_data = data_shared_ptr(m->pool.keys);
    void *container_data = data_shared_ptr(m->pool.container);
    for (; j < m->pool.size; j++)
    {
        if (nodes_data[j].used == 1)
        {
            if (j != i)
            {
                nodes_data[i] = nodes_data[j];
                nodes_data[i].used = 1;
                nodes_data[j].used = 0;
                m->pool.keys_type->t_move(m->pool.keys_type->t_at(keys_data, i), m->pool.keys_type->t_at(keys_data, j));
                m->pool.container_type->t_move(m->pool.container_type->t_at(container_data, i), m->pool.container_type->t_at(container_data, j));
                if (m->root == j)
                    m->root = i;
                if (nodes_data[i].left != 0)
                    nodes_data[nodes_data[i].left].parent = i;
                if (nodes_data[i].right != 0)
                    nodes_data[nodes_data[i].right].parent = i;
                if (nodes_data[i].parent != 0)
                {
                    if (nodes_data[nodes_data[i].parent].left == j)
                        nodes_data[nodes_data[i].parent].left = i;
                    else
                        nodes_data[nodes_data[i].parent].right = i;
                }
            }
            i++;
        }
    }
    shared_ptr new_nodes = realloc_shared_ptr(m->pool.nodes, mul_of_2_size, sizeof(rbt_node));
    if (new_nodes == NULL)
        return 0;
    shared_ptr new_keys = realloc_shared_ptr(m->pool.keys, mul_of_2_size, m->pool.keys_type->t_size);
    if (new_keys == NULL)
    {
        free_shared_ptr(new_nodes);
        return 0;
    }
    shared_ptr new_container = realloc_shared_ptr(m->pool.container, mul_of_2_size, m->pool.container_type->t_size);
    if (new_container == NULL)
    {
        free_shared_ptr(new_nodes);
        free_shared_ptr(new_keys);
        return 0;
    }
    m->pool.nodes = new_nodes;
    m->pool.keys = new_keys;
    m->pool.container = new_container;
    m->pool.size = mul_of_2_size;
    return 1;
}

static rbt_node *node_from_pool(const rbt_map *const m, const size_t index)
{
    return ((rbt_node *)data_shared_ptr(m->pool.nodes)) + index;
}

static size_t get_unused_node(rbt_node_pool *const pool)
{
    ssize_t i = pool->least_unused;
    rbt_node *nodes_data = data_shared_ptr(pool->nodes);
    for (; i < pool->size; i++)
    {
        if (nodes_data[i].used == 0)
            return i;
    }
    return pool->size;
}

static rbt_node * allocate_node(rbt_node_pool *const pool, const size_t index, const void *const key, const void *const val)
{
    if (expand_pool(pool) == 0)
        return NULL;
    rbt_node *nodes_data = data_shared_ptr(pool->nodes);
    void *keys_data = data_shared_ptr(pool->keys);
    void *container_data = data_shared_ptr(pool->container);
    rbt_node *node = nodes_data + index;
    node->left = 0;
    node->right = 0;
    node->parent = 0;
    node->used = 1;
    pool->container_type->t_cpy(pool->container_type->t_at(container_data, index), val);
    pool->keys_type->t_cpy(pool->keys_type->t_at(keys_data, index), key);
    node->color = RED;
    if (pool->least_unused != SIZE_MAX)
        pool->least_unused = index + 1;
    return node;
}

static int free_node(rbt_map *const m, const size_t index, void *const val)
{
    if (index >= m->pool.size)
        return 0;
    rbt_node *nodes_data = data_shared_ptr(m->pool.nodes);
    void *keys_data = data_shared_ptr(m->pool.keys);
    void *container_data = data_shared_ptr(m->pool.container);
    if (val != NULL)
        m->pool.container_type->t_cpy(val, m->pool.container_type->t_at(container_data, index));
    m->pool.keys_type->t_free(m->pool.keys_type->t_at(keys_data, index));
    m->pool.container_type->t_free(m->pool.container_type->t_at(container_data, index));
    nodes_data[index].left = 0;
    nodes_data[index].right = 0;
    nodes_data[index].parent = 0;
    nodes_data[index].used = 0;
    nodes_data[index].color = RED;
    if (index < m->pool.least_unused)
        m->pool.least_unused = index;
    return shrink_pool(m);
}

static void left_rotate(rbt_map *const m, const size_t x_index)
{
    rbt_node *x = node_from_pool(m, x_index);
    size_t y_index = x->right;
    rbt_node *y = node_from_pool(m, y_index);
    x->right = y->left;
    if (y->left != m->nil)
        node_from_pool(m, y->left)->parent = x_index;
    y->parent = x->parent;
    if (x->parent == m->nil)
    {
        m->root = y_index;
    }
    else
    {
        rbt_node *x_parent = node_from_pool(m, x->parent);
        if (x_index == x_parent->left)
            x_parent->left = y_index;
        else
            x_parent->right = y_index;
    }
    y->left = x_index;
    x->parent = y_index;
}

static void right_rotate(rbt_map *const m, const size_t y_index)
{
    rbt_node *y = node_from_pool(m, y_index);
    size_t x_index = y->left;
    rbt_node *x = node_from_pool(m, x_index);
    y->left = x->right;
    if (x->right != m->nil)
        node_from_pool(m, x->right)->parent = y_index;
    x->parent = y->parent;
    if (y->parent == m->nil)
        m->root = x_index;
    else
    {
        rbt_node *y_parent = node_from_pool(m, y->parent);
        if (y_index == y_parent->right)
            y_parent->right = x_index;
        else
            y_parent->left = x_index;
    }
    x->right = y_index;
    y->parent = x_index;
}

static void insert_fixup(rbt_map *const m, size_t z_index)
{
    rbt_node *z = node_from_pool(m, z_index);
    rbt_node *z_parent = node_from_pool(m, z->parent);
    while (z_parent->color == RED)
    {
        rbt_node *z_parent_parent = node_from_pool(m, z_parent->parent);
        if (z->parent == z_parent_parent->left)
        {
            rbt_node *uncle = node_from_pool(m, z_parent_parent->right);
            if (uncle->color == RED)
            {
                z_parent->color = BLACK;
                uncle->color = BLACK;
                z_parent_parent->color = RED;
                z_index = z_parent->parent;
                z = node_from_pool(m, z_index);
                z_parent = node_from_pool(m, z->parent);
            }
            else
            {
                if (z_index == z_parent->right)
                {
                    z_index = z->parent;
                    left_rotate(m, z_index);
                    z = node_from_pool(m, z_index);
                    z_parent = node_from_pool(m, z->parent);
                    z_parent_parent = node_from_pool(m, z_parent->parent);
                }
                z_parent->color = BLACK;
                z_parent_parent->color = RED;
                right_rotate(m, z_parent->parent);
            }
        }
        else
        {
            rbt_node *uncle = node_from_pool(m, z_parent_parent->left);
            if (uncle->color == RED)
            {
                z_parent->color = BLACK;
                uncle->color = BLACK;
                z_parent_parent->color = RED;
                z_index = z_parent->parent;
                z = node_from_pool(m, z_index);
                z_parent = node_from_pool(m, z->parent);
            }
            else
            {
                if (z_index == z_parent->left)
                {
                    z_index = z->parent;
                    right_rotate(m, z_index);
                    z = node_from_pool(m, z_index);
                    z_parent = node_from_pool(m, z->parent);
                    z_parent_parent = node_from_pool(m, z_parent->parent);
                }
                z_parent->color = BLACK;
                z_parent_parent->color = RED;
                left_rotate(m, z_parent->parent);
            }
        }
    }
    node_from_pool(m, m->root)->color = BLACK;
    return;
}

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

static int maximum_node(rbt_map *const m, size_t x_index)
{
    rbt_node *x = node_from_pool(m, x_index);
    while (x->right != m->nil)
    {
        x_index = x->right;
        x = node_from_pool(m, x_index);
    }
    return x_index;
}

static void transplant_node(rbt_map *const m, const size_t u_index, const size_t v_index)
{
    rbt_node *u = node_from_pool(m, u_index);
    if (u->parent == m->nil)
        m->root = v_index;
    else
    {
        rbt_node *u_parent = node_from_pool(m, u->parent);
        if (u_index == u_parent->left)
            u_parent->left = v_index;
        else
            u_parent->right = v_index;
    }
    node_from_pool(m, v_index)->parent = u->parent;
    return;
}

static void delete_fixup(rbt_map *const m, size_t x_index)
{
    rbt_node *x = node_from_pool(m, x_index);
    while (x_index != m->root && x->color == BLACK)
    {
        rbt_node *x_parent = node_from_pool(m, x->parent);
        if (x_index == x_parent->left)
        {
            size_t w_index = x_parent->right;
            rbt_node *w = node_from_pool(m, w_index);
            if (w->color == RED)
            {
                w->color = BLACK;
                x_parent->color = RED;
                left_rotate(m, x->parent);
                w_index = x_parent->right;
                w = node_from_pool(m, w_index);
            }
            rbt_node *w_left = node_from_pool(m, w->left);
            rbt_node *w_right = node_from_pool(m, w->right);
            if (w_left->color == BLACK && w_right->color == BLACK)
            {
                w->color = RED;
                x_index = x->parent;
                x = node_from_pool(m, x_index);
            }
            else
            {
                if (w_right->color == BLACK)
                {
                    w_left->color = BLACK;
                    w->color = RED;
                    right_rotate(m, w_index);
                    w_index = x_parent->right;
                    w = node_from_pool(m, w_index);
                    w_right = node_from_pool(m, w->right);
                }
                w->color = x_parent->color;
                x_parent->color = BLACK;
                w_right->color = BLACK;
                left_rotate(m, x->parent);
                x_index = m->root;
                x = node_from_pool(m, x_index);
            }
        }
        else
        {
            size_t w_index = x_parent->left;
            rbt_node *w = node_from_pool(m, w_index);
            if (w->color == RED)
            {
                w->color = BLACK;
                x_parent->color = RED;
                right_rotate(m, x->parent);
                w_index = x_parent->left;
                w = node_from_pool(m, w_index);
            }
            rbt_node *w_right = node_from_pool(m, w->right);
            rbt_node *w_left = node_from_pool(m, w->left);
            if (w_right->color == BLACK && w_left->color == BLACK)
            {
                w->color = RED;
                x_index = x->parent;
                x = node_from_pool(m, x_index);
            }
            else
            {
                if (w_left->color == BLACK)
                {
                    w_right->color = BLACK;
                    w->color = RED;
                    left_rotate(m, w_index);
                    w_index = x_parent->left;
                    w = node_from_pool(m, w_index);
                    w_left = node_from_pool(m, w->left);
                }
                w->color = x_parent->color;
                x_parent->color = BLACK;
                w_left->color = BLACK;
                right_rotate(m, x->parent);
                x_index = m->root;
                x = node_from_pool(m, x_index);
            }
        }
    }
    x->color = BLACK;
    return;
}

rbt_map *create_rbt_map(const size_t size, const type_func *const keys_type, const type_func *const container_type)
{
    size_t mul_of_2_size = size < RBT_MAP_MIN_SIZE ? RBT_MAP_MIN_SIZE : next_power_of_2(size);
    rbt_map new_m = {
        .root = 0,
        .nil = 0,
        .pool = {
            .least_unused = 1,
            .keys_type = keys_type,
            .container_type = container_type}};
    if (init_pool(&new_m.pool, mul_of_2_size) == 0)
        return NULL;
    rbt_node *nil_node = data_shared_ptr(new_m.pool.nodes);
    nil_node->color = BLACK;
    nil_node->left = 0;
    nil_node->right = 0;
    nil_node->parent = 0;
    nil_node->used = 1;
    return (rbt_map *)memcpy(malloc(sizeof(rbt_map)), &new_m, sizeof(rbt_map));
}

void free_rbt_map(rbt_map *m)
{
    if (m->pool.keys != NULL)
    {
        ssize_t i = 1;
        rbt_node *nodes_data = data_shared_ptr(m->pool.nodes);
        void *keys_data = data_shared_ptr(m->pool.keys);
        for (; i < m->pool.size; i++)
        {
            if (nodes_data[i].used == 1)
                m->pool.keys_type->t_free(m->pool.keys_type->t_at(keys_data, i));
        }
        free_shared_ptr(m->pool.keys);
    }
    if (m->pool.container != NULL)
    {
        ssize_t i = 1;
        rbt_node *nodes_data = data_shared_ptr(m->pool.nodes);
        void *container_data = data_shared_ptr(m->pool.container);
        for (; i < m->pool.size; i++)
        {
            if (nodes_data[i].used == 1)
                m->pool.container_type->t_free(m->pool.container_type->t_at(container_data, i));
        }
        free_shared_ptr(m->pool.container);
    }
    if (m->pool.nodes != NULL)
        free_shared_ptr(m->pool.nodes);
    m->pool.nodes = NULL;
    m->pool.keys = NULL;
    m->pool.container = NULL;
    m->pool.size = 0;
    m->pool.count = 0;
    m->root = m->nil;
    free(m);
    return;
}

int set_rbt_map(rbt_map *const m, const void *const key, const void *const val)
{
    if (key == NULL || val == NULL || m->pool.count == SIZE_MAX)
        return 0;
    size_t z_index = get_unused_node(&m->pool);
    if (z_index == m->pool.size)
        return 0;
    size_t y = m->nil;
    size_t x = m->root;
    void *keys_data = data_shared_ptr(m->pool.keys);
    void *container_data = data_shared_ptr(m->pool.container);
    while (x != m->nil)
    {
        y = x;
        rbt_node *x_node = node_from_pool(m, x);
        int cmp_result = m->pool.keys_type->t_cmp(key, m->pool.keys_type->t_at(keys_data, x));
        if (cmp_result == 0)
        {
            void *container_val = m->pool.container_type->t_at(container_data, x);
            m->pool.container_type->t_free(container_val);
            m->pool.container_type->t_cpy(container_val, val);
            return 1;
        }
        else
        {
            if (cmp_result == -1)
                x = x_node->left;
            else
                x = x_node->right;
        }
    }
    rbt_node *z = allocate_node(&m->pool, z_index, key, val);
    if (z == NULL)
        return 0;
    keys_data = data_shared_ptr(m->pool.keys);
    container_data = data_shared_ptr(m->pool.container);
    z->parent = y;
    if (y == m->nil)
    {
        m->root = z_index;
    }
    else
    {
        rbt_node *y_node = node_from_pool(m, y);
        int cmp_result = m->pool.keys_type->t_cmp(m->pool.keys_type->t_at(keys_data, z_index), m->pool.keys_type->t_at(keys_data, y));
        if (cmp_result == -1)
            y_node->left = z_index;
        else
            y_node->right = z_index;
    }
    z->left = m->nil;
    z->right = m->nil;
    z->color = RED;
    insert_fixup(m, z_index);
    return 1;
}

static size_t get_index_rbt_map(const rbt_map *const m, const void *const key)
{
    size_t current = m->root;
    void *keys_data = data_shared_ptr(m->pool.keys);
    while (current != m->nil)
    {
        rbt_node *node = node_from_pool(m, current);
        int cmp_result = m->pool.keys_type->t_cmp(m->pool.keys_type->t_at(keys_data, current), key);
        if (cmp_result == 0)
            return current;
        else if (cmp_result == 1)
            current = node->left;
        else
            current = node->right;
    }
    return m->nil;
}

int has_key_rbt_map(const rbt_map *const m, const void *const key)
{
    if (key == NULL)
        return 0;
    else
        return get_index_rbt_map(m, key) != m->nil;
}

int get_rbt_map(rbt_map *m, const void *const key, void *const val)
{
    if (key == NULL || val == NULL || m->pool.count <= 1)
        return 0;
    size_t z_index = get_index_rbt_map(m, key);
    if (z_index == m->nil)
        return 0;
    void *container_data = data_shared_ptr(m->pool.container);
    m->pool.container_type->t_cpy(val, m->pool.container_type->t_at(container_data, z_index));
    return 1;
}

int delete_rbt_map(rbt_map *const m, const void *const key, void *const val)
{
    if (key == NULL || m->pool.count <= 1)
        return 0;
    size_t z_index = get_index_rbt_map(m, key);
    if (z_index == m->nil)
        return 0;
    rbt_node *z = node_from_pool(m, z_index);
    size_t y_index = z_index;
    size_t x_index;
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
        rbt_node *y_node = node_from_pool(m, y_index);
        y_original_color = y_node->color;
        x_index = y_node->right;
        if (y_node->parent == z_index)
            node_from_pool(m, x_index)->parent = y_index;
        else
        {
            transplant_node(m, y_index, x_index);
            y_node->right = z->right;
            node_from_pool(m, y_node->right)->parent = y_index;
        }
        transplant_node(m, z_index, y_index);
        y_node->left = z->left;
        node_from_pool(m, y_node->left)->parent = y_index;
        y_node->color = z->color;
    }
    if (y_original_color == BLACK)
        delete_fixup(m, x_index);
    return free_node(m, z_index, val);
}

int get_min_rbt_map(rbt_map *m, const void *const key, void *const val)
{
    if ((key == NULL && val == NULL) || m->root == m->nil)
        return 0;
    size_t index = minimum_node(m, m->root);
    void *keys_data = data_shared_ptr(m->pool.keys);
    void *container_data = data_shared_ptr(m->pool.container);
    if (key != NULL)
        m->pool.keys_type->t_cpy(key, m->pool.keys_type->t_at(keys_data, index));
    if (val != NULL)
        m->pool.container_type->t_cpy(val, m->pool.container_type->t_at(container_data, index));
    return 1;
}

int get_max_rbt_map(rbt_map *m, const void *const key, void *const val)
{
    if ((key == NULL && val == NULL) || m->root == m->nil)
        return 0;
    size_t index = maximum_node(m, m->root);
    void *keys_data = data_shared_ptr(m->pool.keys);
    void *container_data = data_shared_ptr(m->pool.container);
    if (key != NULL)
        m->pool.keys_type->t_cpy(key, m->pool.keys_type->t_at(keys_data, index));
    if (val != NULL)
        m->pool.container_type->t_cpy(val, m->pool.container_type->t_at(container_data, index));
    return 1;
}

#include "rbt_map.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

#define RBT_MAP_MIN_SIZE 32

/* Struct's declarations */
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
    size_t count;
    size_t size;
    size_t least_unused;
    const type_func *keys_type;
    const type_func *container_type;
    rbt_node *nodes;
    void *keys;
    void *container;
} rbt_node_pool;

typedef struct rbt_map_t
{
    rbt_node_pool pool;
    size_t root;
    size_t nil;
} rbt_map_t;

/* Static function's declarations */
static int init_pool(rbt_node_pool *const pool, const size_t size);
static int expand_pool(rbt_node_pool *const pool);
static int shrink_pool(rbt_map_t *const m);
static rbt_node *node_from_pool(const rbt_map_t *const m, const size_t index);
static size_t get_unused_node(rbt_node_pool *const pool);
static rbt_node *allocate_node(rbt_node_pool *const pool, const size_t index, const void *const key, const void *const val);
static int free_node(rbt_map_t *const m, const size_t index, void *const val);
static void left_rotate(rbt_map_t *const m, const size_t x_index);
static void right_rotate(rbt_map_t *const m, const size_t y_index);
static void insert_fixup(rbt_map_t *const m, size_t z_index);
static int minimum_node(rbt_map_t *const m, size_t x_index);
static int maximum_node(rbt_map_t *const m, size_t x_index);
static void transplant_node(rbt_map_t *const m, const size_t u_index, const size_t v_index);
static void delete_fixup(rbt_map_t *const m, size_t x_index);
static size_t get_index_rbt_map(const rbt_map_t *const m, const void *const key);

/* Main functions */
rbt_map create_rbt_map(const size_t size, const type_func *const keys_type, const type_func *const container_type)
{
    rbt_map ptr = malloc_shared_ptr(1, sizeof(rbt_map_t));
    if (ptr == NULL)
        return NULL;
    size_t mul_of_2_size = size < RBT_MAP_MIN_SIZE ? RBT_MAP_MIN_SIZE : next_power_of_2(size);
    rbt_map_t new_m = {
        .root = 0,
        .nil = 0,
        .pool = {
            .least_unused = 1,
            .keys_type = keys_type,
            .container_type = container_type}};
    if (init_pool(&new_m.pool, mul_of_2_size) == 0)
    {
        free_shared_ptr(ptr);
        return NULL;
    }
    rbt_node *nil_node = new_m.pool.nodes;
    nil_node->color = BLACK;
    nil_node->left = 0;
    nil_node->right = 0;
    nil_node->parent = 0;
    nil_node->used = 1;
    memcpy(data_shared_ptr(ptr), &new_m, sizeof(rbt_map_t));
    return ptr;
}

void free_rbt_map(rbt_map const ptr)
{
    if (ptr == NULL)
        return;
    if (count_shared_ptr(ptr) == 1)
    {
        rbt_map_t *m = data_shared_ptr(ptr);
        if (m->pool.keys != NULL)
        {
            size_t i = 1;
            for (; i < m->pool.size; i++)
            {
                if (m->pool.nodes[i].used == 1)
                    m->pool.keys_type->t_free(m->pool.keys_type->t_at(m->pool.keys, i));
            }
            free(m->pool.keys);
        }
        if (m->pool.container != NULL)
        {
            size_t i = 1;
            for (; i < m->pool.size; i++)
            {
                if (m->pool.nodes[i].used == 1)
                    m->pool.container_type->t_free(m->pool.container_type->t_at(m->pool.container, i));
            }
            free(m->pool.container);
        }
        if (m->pool.nodes != NULL)
            free(m->pool.nodes);
        m->pool.nodes = NULL;
        m->pool.keys = NULL;
        m->pool.container = NULL;
        m->pool.size = 0;
        m->pool.count = 0;
        m->root = m->nil;
    }
    free_shared_ptr(ptr);
    return;
}

int set_rbt_map(rbt_map const ptr, const void *const key, const void *const val)
{
    if (ptr == NULL)
        return 0;
    rbt_map_t *m = data_shared_ptr(ptr);
    if (key == NULL || val == NULL || m->pool.count == SIZE_MAX)
        return 0;
    size_t z_index = get_unused_node(&m->pool);
    if (z_index == m->pool.size)
        return 0;
    size_t y = m->nil;
    size_t x = m->root;
    while (x != m->nil)
    {
        y = x;
        rbt_node *x_node = node_from_pool(m, x);
        int cmp_result = m->pool.keys_type->t_cmp(key, m->pool.keys_type->t_at(m->pool.keys, x));
        if (cmp_result == 0)
        {
            void *container_val = m->pool.container_type->t_at(m->pool.container, x);
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
    z->parent = y;
    if (y == m->nil)
    {
        m->root = z_index;
    }
    else
    {
        rbt_node *y_node = node_from_pool(m, y);
        int cmp_result = m->pool.keys_type->t_cmp(m->pool.keys_type->t_at(m->pool.keys, z_index), m->pool.keys_type->t_at(m->pool.keys, y));
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

int has_key_rbt_map(const rbt_map ptr, const void *const key)
{
    if (ptr == NULL)
        return 0;
    rbt_map_t *m = data_shared_ptr(ptr);
    if (key == NULL)
        return 0;
    else
        return get_index_rbt_map(m, key) != m->nil;
}

int get_rbt_map(const rbt_map ptr, const void *const key, void *const val)
{
    if (ptr == NULL)
        return 0;
    rbt_map_t *m = data_shared_ptr(ptr);
    if (key == NULL || val == NULL || m->pool.count <= 1)
        return 0;
    size_t z_index = get_index_rbt_map(m, key);
    if (z_index == m->nil)
        return 0;
    m->pool.container_type->t_cpy(val, m->pool.container_type->t_at(m->pool.container, z_index));
    return 1;
}

int delete_rbt_map(rbt_map const ptr, const void *const key, void *const val)
{
    if (ptr == NULL)
        return 0;
    rbt_map_t *m = data_shared_ptr(ptr);
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

int get_min_rbt_map(const rbt_map ptr, void *const key, void *const val)
{
    if (ptr == NULL)
        return 0;
    rbt_map_t *m = data_shared_ptr(ptr);
    if ((key == NULL && val == NULL) || m->root == m->nil)
        return 0;
    size_t index = minimum_node(m, m->root);
    if (key != NULL)
        m->pool.keys_type->t_cpy(key, m->pool.keys_type->t_at(m->pool.keys, index));
    if (val != NULL)
        m->pool.container_type->t_cpy(val, m->pool.container_type->t_at(m->pool.container, index));
    return 1;
}

int get_max_rbt_map(const rbt_map ptr, void *const key, void *const val)
{
    if (ptr == NULL)
        return 0;
    rbt_map_t *m = data_shared_ptr(ptr);
    if ((key == NULL && val == NULL) || m->root == m->nil)
        return 0;
    size_t index = maximum_node(m, m->root);
    if (key != NULL)
        m->pool.keys_type->t_cpy(key, m->pool.keys_type->t_at(m->pool.keys, index));
    if (val != NULL)
        m->pool.container_type->t_cpy(val, m->pool.container_type->t_at(m->pool.container, index));
    return 1;
}

/* Static functions */
static int init_pool(rbt_node_pool *const pool, const size_t size)
{
    pool->nodes = calloc(size, sizeof(rbt_node));
    if (pool->nodes == NULL)
        return 0;
    pool->keys = calloc(size, pool->keys_type->t_size);
    if (pool->keys == NULL)
    {
        free(pool->nodes);
        return 0;
    }
    pool->container = calloc(size, pool->container_type->t_size);
    if (pool->container == NULL)
    {
        free(pool->nodes);
        free(pool->keys);
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
    rbt_node *new_nodes = realloc(pool->nodes, mul_of_2_size * sizeof(rbt_node));
    if (new_nodes == NULL)
        return 0;
    void *new_keys = realloc(pool->keys, mul_of_2_size * pool->keys_type->t_size);
    if (new_keys == NULL)
    {
        free(new_nodes);
        return 0;
    }
    void *new_container = realloc(pool->container, mul_of_2_size * pool->container_type->t_size);
    if (new_container == NULL)
    {
        free(new_nodes);
        free(new_keys);
        return 0;
    }
    size_t elements = mul_of_2_size - pool->size;
    memset(new_nodes + pool->size, 0, elements * sizeof(rbt_node));
    memset(pool->keys_type->t_at(new_keys, pool->size), 0, elements * pool->keys_type->t_size);
    memset(pool->container_type->t_at(new_container, pool->size), 0, elements * pool->container_type->t_size);
    pool->nodes = new_nodes;
    pool->keys = new_keys;
    pool->container = new_container;
    pool->size = mul_of_2_size;
    return 1;
}

static int shrink_pool(rbt_map_t *const m)
{
    if (--m->pool.count > m->pool.size >> 3)
        return 1;
    size_t mul_of_2_size = next_power_of_2(m->pool.count);
    if (mul_of_2_size <= RBT_MAP_MIN_SIZE)
        return 1;
    size_t i = 1;
    size_t j = 1;
    for (; j < m->pool.size; j++)
    {
        if (m->pool.nodes[j].used == 1)
        {
            if (j != i)
            {
                m->pool.nodes[i] = m->pool.nodes[j];
                m->pool.nodes[i].used = 1;
                m->pool.nodes[j].used = 0;
                m->pool.keys_type->t_move(m->pool.keys_type->t_at(m->pool.keys, i), m->pool.keys_type->t_at(m->pool.keys, j));
                m->pool.container_type->t_move(m->pool.container_type->t_at(m->pool.container, i), m->pool.container_type->t_at(m->pool.container, j));
                if (m->root == j)
                    m->root = i;
                if (m->pool.nodes[i].left != 0)
                    m->pool.nodes[m->pool.nodes[i].left].parent = i;
                if (m->pool.nodes[i].right != 0)
                    m->pool.nodes[m->pool.nodes[i].right].parent = i;
                if (m->pool.nodes[i].parent != 0)
                {
                    if (m->pool.nodes[m->pool.nodes[i].parent].left == j)
                        m->pool.nodes[m->pool.nodes[i].parent].left = i;
                    else
                        m->pool.nodes[m->pool.nodes[i].parent].right = i;
                }
            }
            i++;
        }
    }
    rbt_node *new_nodes = realloc(m->pool.nodes, mul_of_2_size * sizeof(rbt_node));
    if (new_nodes == NULL)
        return 0;
    void *new_keys = realloc(m->pool.keys, mul_of_2_size * m->pool.keys_type->t_size);
    if (new_keys == NULL)
    {
        free(new_nodes);
        return 0;
    }
    void *new_container = realloc(m->pool.container, mul_of_2_size * m->pool.container_type->t_size);
    if (new_container == NULL)
    {
        free(new_nodes);
        free(new_keys);
        return 0;
    }
    m->pool.nodes = new_nodes;
    m->pool.keys = new_keys;
    m->pool.container = new_container;
    m->pool.size = mul_of_2_size;
    return 1;
}

static rbt_node *node_from_pool(const rbt_map_t *const m, const size_t index)
{
    return (m->pool.nodes) + index;
}

static size_t get_unused_node(rbt_node_pool *const pool)
{
    size_t i = pool->least_unused;
    for (; i < pool->size; i++)
    {
        if (pool->nodes[i].used == 0)
            return i;
    }
    return pool->size;
}

static rbt_node *allocate_node(rbt_node_pool *const pool, const size_t index, const void *const key, const void *const val)
{
    if (expand_pool(pool) == 0)
        return NULL;
    rbt_node *node = pool->nodes + index;
    node->left = 0;
    node->right = 0;
    node->parent = 0;
    node->used = 1;
    pool->container_type->t_cpy(pool->container_type->t_at(pool->container, index), val);
    pool->keys_type->t_cpy(pool->keys_type->t_at(pool->keys, index), key);
    node->color = RED;
    if (pool->least_unused != SIZE_MAX)
        pool->least_unused = index + 1;
    return node;
}

static int free_node(rbt_map_t *const m, const size_t index, void *const val)
{
    if (index >= m->pool.size)
        return 0;
    if (val != NULL)
        m->pool.container_type->t_cpy(val, m->pool.container_type->t_at(m->pool.container, index));
    m->pool.keys_type->t_free(m->pool.keys_type->t_at(m->pool.keys, index));
    m->pool.container_type->t_free(m->pool.container_type->t_at(m->pool.container, index));
    m->pool.nodes[index].left = 0;
    m->pool.nodes[index].right = 0;
    m->pool.nodes[index].parent = 0;
    m->pool.nodes[index].used = 0;
    m->pool.nodes[index].color = RED;
    if (index < m->pool.least_unused)
        m->pool.least_unused = index;
    return shrink_pool(m);
}

static void left_rotate(rbt_map_t *const m, const size_t x_index)
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

static void right_rotate(rbt_map_t *const m, const size_t y_index)
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

static void insert_fixup(rbt_map_t *const m, size_t z_index)
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

static int minimum_node(rbt_map_t *const m, size_t x_index)
{
    rbt_node *x = node_from_pool(m, x_index);
    while (x->left != m->nil)
    {
        x_index = x->left;
        x = node_from_pool(m, x_index);
    }
    return x_index;
}

static int maximum_node(rbt_map_t *const m, size_t x_index)
{
    rbt_node *x = node_from_pool(m, x_index);
    while (x->right != m->nil)
    {
        x_index = x->right;
        x = node_from_pool(m, x_index);
    }
    return x_index;
}

static void transplant_node(rbt_map_t *const m, const size_t u_index, const size_t v_index)
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

static void delete_fixup(rbt_map_t *const m, size_t x_index)
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

static size_t get_index_rbt_map(const rbt_map_t *const m, const void *const key)
{
    size_t current = m->root;
    while (current != m->nil)
    {
        rbt_node *node = node_from_pool(m, current);
        int cmp_result = m->pool.keys_type->t_cmp(m->pool.keys_type->t_at(m->pool.keys, current), key);
        if (cmp_result == 0)
            return current;
        else if (cmp_result == 1)
            current = node->left;
        else
            current = node->right;
    }
    return m->nil;
}

/* Type functionality */
void *t_at_rbt_map(const void *const src, const size_t index)
{
    return ((rbt_map *)src) + index;
}

int t_cmp_rbt_map(const void *const src_1, const void *const src_2)
{
    rbt_map_t *a = data_shared_ptr(*(rbt_map *)src_1);
    rbt_map_t *b = data_shared_ptr(*(rbt_map *)src_2);
    return a->pool.size > b->pool.size;
}

void *t_cpy_rbt_map(void *const dest, const void *const src)
{
    if (*(rbt_map *)dest != *(rbt_map *)src)
        *(rbt_map *)dest = copy_shared_ptr(*(rbt_map *)src);
    return dest;
}

void *t_move_rbt_map(void *const dest, const void *const src)
{
    *(rbt_map *)dest = *(rbt_map *)src;
    return dest;
}

void t_free_rbt_map(void *const src)
{
    rbt_map *val = (rbt_map *)src;
    free_rbt_map(*val);
    return;
}

size_t t_hash_rbt_map(const void *const src)
{
    rbt_map_t *m = data_shared_ptr(*(rbt_map *)src);
    size_t h = 0;
    h = hash_size_t(m->pool.count);
    h = hash_size_t(m->pool.size) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t(m->pool.least_unused) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)m->pool.keys_type) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)m->pool.container_type) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)m->pool.nodes) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)m->pool.keys) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)m->pool.container) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t(m->root) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t(m->nil) ^ (h << 6) ^ (h >> 2);
    return h;
}

void t_swap_rbt_map(void *const src_1, void *const src_2)
{
    if (src_1 == src_2)
        return;
    size_t *a = (size_t *)src_1;
    size_t *b = (size_t *)src_2;
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
    return;
}

const type_func orig_f_rbt_map = {
    .t_size = sizeof(rbt_map),
    .t_at = t_at_rbt_map,
    .t_cmp = t_cmp_rbt_map,
    .t_cpy = t_cpy_rbt_map,
    .t_move = t_move_rbt_map,
    .t_free = t_free_rbt_map,
    .t_hash = t_hash_rbt_map,
    .t_swap = t_swap_rbt_map};

const type_func *f_rbt_map = &orig_f_rbt_map;

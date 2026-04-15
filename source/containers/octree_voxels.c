#include "octree_voxels.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static const point_t child_pos_offset[8] =
{{.x = 0, .y = 0, .z = 0},
 {.x = 0, .y = 0, .z = 1},
 {.x = 0, .y = 1, .z = 0},
 {.x = 0, .y = 1, .z = 1},
 {.x = 1, .y = 0, .z = 0},
 {.x = 1, .y = 0, .z = 1},
 {.x = 1, .y = 1, .z = 0},
 {.x = 1, .y = 1, .z = 1}};

static inline size_t pow3(const size_t x)
{
    return x * x * x;
}

octree_t octree(const uint32_t grid_size, const uint32_t min_size)
{
    assert(grid_size % 2 == 0);
    assert(min_size % 2 == 0 || min_size == 1);
    assert(min_size < grid_size);
    size_t data_size = 1 + pow3(grid_size / min_size) * 9 / 8;
    octree_node_t *const root = calloc(data_size, sizeof(octree_node_t));
    assert(root != 0);
    root[0].is_leaf = 1;
    return (octree_t){.root = root, .grid_size = grid_size, .min_size = min_size};
}

static inline point_t point_sum_mul(const point_t p1, const point_t p2, const int32_t value)
{
    return (point_t){.x = p1.x + p2.x * value, .y = p1.y + p2.y * value, .z = p1.z + p2.z * value};
}

static inline size_t get_index(const octree_node_t *const node, const octree_node_t *const root)
{
    return node - root;
}

static inline size_t get_octant(const octree_node_t *const node, const octree_node_t *const root)
{
    return (get_index(node, root) - 1) % 8;
}

static inline octree_node_t* get_child(octree_node_t *const node, octree_node_t *const root, const uint8_t octant)
{
    return root + octant + 1 + 8 * get_index(node, root);
}

static inline octree_node_t* get_parent(octree_node_t *const node, octree_node_t *const root)
{
    return root + (get_index(node, root) - get_octant(node, root)) / 8;
}

static void octree_update(octree_node_t *node, octree_node_t *const root, const uint8_t value)
{
    while (node != root)
    {
        octree_node_t *const parent = get_parent(node, root);
        octree_node_t *const children = get_child(parent, root, 0);
        const uint8_t is_leaf_mask = (children[0].is_leaf << 0) | (children[1].is_leaf << 1) |
                                     (children[2].is_leaf << 2) | (children[3].is_leaf << 3) |
                                     (children[4].is_leaf << 4) | (children[5].is_leaf << 5) |
                                     (children[6].is_leaf << 6) | (children[7].is_leaf << 7);
        const uint8_t is_value_the_same = children[0].value ^ children[1].value ^ children[2].value ^ children[3].value ^
                                          children[4].value ^ children[5].value ^ children[6].value ^ children[7].value;
        if (is_leaf_mask == 0xFF && is_value_the_same == 0)
        {
            node = parent;
            node->value = value;
            node->is_leaf = 1;
        }
        else
        {
            return;
        }
    }
}

void octree_set_voxel(octree_t ot, const point_t p, const uint8_t value)
{
    /* Проверяем, входит ли точка в куб */
    if ((uint32_t)p.x < ot.grid_size &&
        (uint32_t)p.y < ot.grid_size &&
        (uint32_t)p.z < ot.grid_size)
    {
        octree_node_t *node = ot.root;
        point_t cur_pos = {.x = 0, .y = 0, .z = 0};
        int32_t cur_size = ot.grid_size;
        while (1)
        {
             /* Достигли самого нижнего уровня */
            if (cur_size == ot.min_size)
            {
                node->is_leaf = 1;
                node->value = value;
                octree_update(node, ot.root, value);
                return;
            }
            /* На промежуточном уровне, выбираем потомка ноды для дальнейшего анализа */
            else
            {
                if (node->is_leaf == 1)
                {
                    octree_node_t *const children = get_child(node, ot.root, 0);
                    node->children_mask = (children[0].children_mask != 0) << 0 |
                                            (children[1].children_mask != 0) << 1 |
                                            (children[2].children_mask != 0) << 2 |
                                            (children[3].children_mask != 0) << 3 |
                                            (children[4].children_mask != 0) << 4 |
                                            (children[5].children_mask != 0) << 5 |
                                            (children[6].children_mask != 0) << 6 |
                                            (children[7].children_mask != 0) << 7;
                    children[0].value = node->value;
                    children[0].is_leaf = 1;
                    children[1].value = node->value;
                    children[1].is_leaf = 1;
                    children[2].value = node->value;
                    children[2].is_leaf = 1;
                    children[3].value = node->value;
                    children[3].is_leaf = 1;
                    children[4].value = node->value;
                    children[4].is_leaf = 1;
                    children[5].value = node->value;
                    children[5].is_leaf = 1;
                    children[6].value = node->value;
                    children[6].is_leaf = 1;
                    children[7].value = node->value;
                    children[7].is_leaf = 1;
                    node->is_leaf = 0;
                }
                else
                {
                    /* Ничего не делать */
                }
                cur_size >>= 1;
                const uint8_t octant = ((p.x >= (cur_pos.x + cur_size)) ? 4 : 0) |
                                       ((p.y >= (cur_pos.y + cur_size)) ? 2 : 0) |
                                       ((p.z >= (cur_pos.z + cur_size)) ? 1 : 0);
                node->children_mask |= 1 << octant;
                node = get_child(node, ot.root, octant);
                cur_pos = point_sum_mul(cur_pos, child_pos_offset[octant], cur_size);
            }
        }
    }
    else
    {
        /* Если нет, выходим */
        return;
    }
}

void octree_print(octree_t ot)
{
    octree_node_t *node = ot.root;
    point_t cur_pos = {.x = 0, .y = 0, .z = 0};
    int32_t cur_size = ot.grid_size;
    int32_t i = 0;
    while (1)
    {
        if (node->is_leaf)
        {
            printf("xyz: %d, %d, %d | size: %d | value: %d\n", cur_pos.x, cur_pos.y, cur_pos.z, cur_size, node->value);
            i = get_octant(node, ot.root);
            node = get_parent(node, ot.root);
            cur_pos = point_sum_mul(cur_pos, child_pos_offset[i], -cur_size);
            i++;
            cur_size *= 2;
        }
        else
        {
            if (i < 8)
            {
                cur_size /= 2;
                node = get_child(node, ot.root, i);
                cur_pos = point_sum_mul(cur_pos, child_pos_offset[i], cur_size);
                i = 0;
            }
            else
            {
                if (node == ot.root)
                {
                    return;
                }
                else
                {
                    i = get_octant(node, ot.root);
                    node = get_parent(node, ot.root);
                    cur_pos = point_sum_mul(cur_pos, child_pos_offset[i], -cur_size);
                    i++;
                    cur_size *= 2;
                }
            }
        }
    }
}

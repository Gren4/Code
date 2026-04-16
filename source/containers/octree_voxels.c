#include "octree_voxels.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static const point_t child_pos_offset[8] = {
    {.x = 0, .y = 0, .z = 0},
    {.x = 0, .y = 0, .z = 1},
    {.x = 0, .y = 1, .z = 0},
    {.x = 0, .y = 1, .z = 1},
    {.x = 1, .y = 0, .z = 0},
    {.x = 1, .y = 0, .z = 1},
    {.x = 1, .y = 1, .z = 0},
    {.x = 1, .y = 1, .z = 1}};

static const uint8_t octant_bit[8] =
    {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};

octree_t octree(const uint32_t grid_size, const uint32_t min_size)
{
    assert(grid_size % 2 == 0);
    assert(min_size % 2 == 0 || min_size == 1);
    assert(min_size < grid_size);
    octree_node_t *const root = calloc(1, sizeof(octree_node_t));
    assert(root != 0);
    return (octree_t){.root = root,
                      .node_count = 0,
                      .capacity = 1,
                      .grid_size = grid_size,
                      .min_size = min_size};
}

static inline point_t point_sum_mul(const point_t p1, const point_t p2, const int32_t value)
{
    return (point_t){.x = p1.x + p2.x * value, .y = p1.y + p2.y * value, .z = p1.z + p2.z * value};
}

static inline void octree_update(octree_t *const ot, octree_node_t *node, const uint32_t value)
{
    while (node != ot->root)
    {
        octree_node_t *const parent = node - node->parent_offset;
        if (parent->children_mask != 0xFF)
            return;
        int i;
        for (i = 0; i < 8; i++)
        {
            const octree_node_t *const child = parent + parent->child_offset[i];
            if (child->is_leaf != 1 || child->value != value)
                break;
        }
        if (i != 8)
            return;
        node = parent;
        node->value = value;
        node->is_leaf = 1;
        /* TO DO: free children */
    }
}

void octree_set_voxel(octree_t *const ot, const point_t p, const uint32_t value)
{
    /* Проверяем, входит ли точка в куб */
    if ((uint32_t)p.x >= ot->grid_size ||
        (uint32_t)p.y >= ot->grid_size ||
        (uint32_t)p.z >= ot->grid_size)
        return;
    octree_node_t *node = ot->root;
    point_t cur_pos = {.x = 0, .y = 0, .z = 0};
    uint32_t cur_size = ot->grid_size;
    while (1)
    {
        /* Достигли самого нижнего уровня */
        if (cur_size == ot->min_size)
        {
            node->is_leaf = 1;
            node->value = value;
            octree_update(ot, node, value);
            return;
        }
        /* На промежуточном уровне, выбираем потомка ноды для дальнейшего анализа */
        else
        {
            if (node->is_leaf == 1)
            {
                if (node->value == value)
                    return;
                const uint32_t node_index = node - ot->root;
                node->children_mask = 0xFF;
                if (ot->node_count + 8 >= ot->capacity)
                {
                    ot->capacity += 16;
                    ot->root = realloc(ot->root, ot->capacity * sizeof(octree_node_t));
                    assert(ot->root != 0);
                    node = ot->root + node_index;
                }
                int32_t i;
                for (i = 0; i < 8; i++)
                {
                    ot->node_count++;
                    node->child_offset[i] = ot->node_count - node_index;
                    octree_node_t *const child = node + node->child_offset[i];
                    child->parent_offset = node->child_offset[i];
                    child->value = node->value;
                    child->children_mask = 0;
                    child->is_leaf = 1;
                    child->octant = i;
                }
                node->is_leaf = 0;
            }
            cur_size >>= 1;
            const uint8_t octant = ((p.x >= (cur_pos.x + cur_size)) ? 4 : 0) |
                                   ((p.y >= (cur_pos.y + cur_size)) ? 2 : 0) |
                                   ((p.z >= (cur_pos.z + cur_size)) ? 1 : 0);
            if ((node->children_mask & octant_bit[octant]) == 0)
            {
                const uint32_t node_index = node - ot->root;
                ot->node_count++;
                if (ot->node_count == ot->capacity)
                {
                    ot->capacity += 8;
                    ot->root = realloc(ot->root, ot->capacity * sizeof(octree_node_t));
                    assert(ot->root != 0);
                    node = ot->root + node_index;
                }
                node->child_offset[octant] = ot->node_count - node_index;
                node->children_mask |= octant_bit[octant];
                octree_node_t *const child = node + node->child_offset[octant];
                child->parent_offset = node->child_offset[octant];
                child->value = 0;
                child->children_mask = 0;
                child->is_leaf = 0;
                child->octant = octant;
            }
            node += node->child_offset[octant];
            cur_pos = point_sum_mul(cur_pos, child_pos_offset[octant], cur_size);
        }
    }
}

void octree_print(const octree_t *const ot)
{
    printf("Octree:\nGrid size: %d\nMin size: %d\nNode count: %d\nCapacity: %d\n", ot->grid_size, ot->min_size, ot->node_count, ot->capacity);
    const octree_node_t *node = ot->root;
    point_t cur_pos = {.x = 0, .y = 0, .z = 0};
    uint32_t cur_size = ot->grid_size;
    int32_t octant = 0;
    while (1)
    {
        if (node->is_leaf)
        {
            printf("xyz: %d, %d, %d | size: %d | value: %d\n", cur_pos.x, cur_pos.y, cur_pos.z, cur_size, node->value);
            octant = node->octant;
            node -= node->parent_offset;
            cur_pos = point_sum_mul(cur_pos, child_pos_offset[octant], -cur_size);
            octant++;
            cur_size *= 2;
        }
        else
        {
            for (; octant < 8; octant++)
            {
                if ((node->children_mask & octant_bit[octant]) != 0)
                    break;
            }
            if (octant < 8)
            {
                cur_size /= 2;
                node += node->child_offset[octant];
                cur_pos = point_sum_mul(cur_pos, child_pos_offset[octant], cur_size);
                octant = 0;
            }
            else
            {
                if (node == ot->root)
                {
                    return;
                }
                else
                {
                    octant = node->octant;
                    node -= node->parent_offset;
                    cur_pos = point_sum_mul(cur_pos, child_pos_offset[octant], -cur_size);
                    octant++;
                    cur_size *= 2;
                }
            }
        }
    }
}

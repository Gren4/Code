#include "octree_voxels.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const point_t octant_pos_offset[8] = {
    {.x = 0, .y = 0, .z = 0},
    {.x = 0, .y = 0, .z = 1},
    {.x = 0, .y = 1, .z = 0},
    {.x = 0, .y = 1, .z = 1},
    {.x = 1, .y = 0, .z = 0},
    {.x = 1, .y = 0, .z = 1},
    {.x = 1, .y = 1, .z = 0},
    {.x = 1, .y = 1, .z = 1}};

octree_t octree(const uint32_t grid_size, const uint32_t min_size)
{
    assert(grid_size % 2 == 0);
    assert(grid_size <= 512);
    assert(min_size % 2 == 0 || min_size == 1);
    assert(min_size < grid_size);
    octree_node_t *const root = calloc(1, sizeof(octree_node_t));
    assert(root != 0);
    uint32_t *const empty_list = calloc(1, sizeof(uint32_t));
    assert(empty_list != 0);
    return (octree_t){.root = root,
                      .empty_list = empty_list,
                      .node_count = 1,
                      .capacity = 1,
                      .empty_count = 0,
                      .empty_capacity = 1,
                      .grid_size = grid_size,
                      .max_depth = log2(grid_size / min_size)};
}

void octree_free(octree_t *const ot)
{
    assert(ot != 0);
    assert(ot->root != 0);
    assert(ot->empty_list != 0);
    free(ot->root);
    free(ot->empty_list);
    return;
}

static inline point_t point_sum_mul(const point_t p1, const point_t p2, const int32_t value)
{
    return (point_t){.x = p1.x + p2.x * value, .y = p1.y + p2.y * value, .z = p1.z + p2.z * value};
}

static inline color_t get_color(const octree_node_t node)
{
    return (color_t){.r = (node >> 16) & 0xFF,
                     .g = (node >> 8) & 0xFF,
                     .b = node & 0xFF};
}

static inline uint32_t pack_index(const uint32_t index)
{
    return (index - 1) / 8;
}

static inline uint32_t unpack_index(const octree_node_t node)
{
    return (node & MASK_INDEX) * 8 + 1;
}

static inline uint32_t ask_for_index(octree_t *const ot, octree_node_t **const node)
{
    if (ot->empty_count > 0)
    {
        ot->node_count += 8;
        return ot->empty_list[--ot->empty_count];
    }
    else
    {
        const uint32_t index = ot->node_count;
        if (ot->node_count + 8 >= ot->capacity)
        {
            const uint32_t node_index = *node - ot->root;
            ot->capacity += 8;
            ot->root = realloc(ot->root, ot->capacity * sizeof(octree_node_t));
            assert(ot->root != 0);
            memset(ot->root + index, 0, 8 * sizeof(octree_node_t));
            *node = ot->root + node_index;
        }
        ot->node_count += 8;
        return index;
    }
}

static inline void add_to_empty_list(octree_t *const ot, const uint32_t index)
{
    memset(ot->root + index, 0, 8 * sizeof(octree_node_t));
    ot->node_count -= 8;
    if (ot->empty_count + 1 >= ot->empty_capacity)
    {
        ot->empty_capacity++;
        ot->empty_list = realloc(ot->empty_list, ot->empty_capacity * sizeof(uint32_t));
        assert(ot->empty_list != 0);
    }
    ot->empty_list[ot->empty_count++] = index;
    return;
}

void octree_set_voxel(octree_t *const ot, const point_t p, const color_t color)
{
    assert(ot != 0);
    if ((uint32_t)p.x >= ot->grid_size || (uint32_t)p.y >= ot->grid_size || (uint32_t)p.z >= ot->grid_size)
        return;
    octree_node_t *node = ot->root;
    point_t cur_pos = {.x = 0, .y = 0, .z = 0};
    uint32_t cur_size = ot->grid_size;
    uint32_t parent_cache[10] = {0};
    uint8_t cur_depth = 0;
    const uint32_t packed_color = color.r << 16 | color.g << 8 | color.b;
    while (1)
    {
        if (cur_depth == ot->max_depth)
        {
            const uint32_t node_data = MASK_LEAF | packed_color;
            node[0] = node_data;
            while (node != ot->root)
            {
                octree_node_t *const parent = ot->root + parent_cache[--cur_depth];
                const uint32_t index = *parent & MASK_INDEX;
                octree_node_t *const children = ot->root + index;
                uint8_t octant = 0;
                while (children[octant] == node_data && octant < 8)
                {
                    octant++;
                }
                if (octant != 8)
                    return;
                add_to_empty_list(ot, index);
                node = parent;
                node[0] = node_data;
            }
            return;
        }
        else
        {
            if ((node[0] & MASK_TYPE) == MASK_LEAF)
            {
                const uint32_t node_color = node[0] & MASK_COLOR;
                if (node_color == packed_color)
                    return;
                const uint32_t node_data = MASK_LEAF | node_color;
                const uint32_t index = ask_for_index(ot, &node);
                node[0] = MASK_NODE | index;
                octree_node_t *const children = ot->root + index;
                int8_t octant;
                for (octant = 0; octant < 8; octant++)
                {
                    children[octant] = node_data;
                }
            }
            if ((node[0] & MASK_INDEX) == 0)
            {
                const uint32_t index = ask_for_index(ot, &node);
                node[0] = MASK_NODE | index;
            }
            parent_cache[cur_depth++] = node - ot->root;
            cur_size /= 2;
            const point_t center = (point_t){.x = cur_pos.x + cur_size, .y = cur_pos.y + cur_size, .z = cur_pos.z + cur_size};
            const uint8_t octant = ((p.x >= center.x) ? 4 : 0) | ((p.y >= center.y) ? 2 : 0) | ((p.z >= center.z) ? 1 : 0);
            cur_pos = point_sum_mul(cur_pos, octant_pos_offset[octant], cur_size);
            node = ot->root + (node[0] & MASK_INDEX) + octant;
        }
    }
}

static uint32_t *get_parent_indexes(const octree_t *const ot)
{
    uint32_t *const parent_indexes = calloc(ot->capacity, sizeof(uint32_t));
    assert(parent_indexes != 0);
    const octree_node_t *node = ot->root;
    uint32_t parent_cache[10] = {0};
    uint8_t octant_cache[10] = {0};
    uint8_t cur_depth = 0;
    while (1)
    {
        if ((node[0] & MASK_TYPE) == MASK_LEAF)
        {
            node = ot->root + parent_cache[--cur_depth];
            octant_cache[cur_depth]++;
        }
        else
        {
            const uint32_t index = node[0] & MASK_INDEX;
            const octree_node_t *const children = ot->root + index;
            while ((children[octant_cache[cur_depth]] & MASK_TYPE) == MASK_EMPTY && octant_cache[cur_depth] < 8)
            {
                octant_cache[cur_depth]++;
            }
            if (octant_cache[cur_depth] < 8)
            {
                parent_cache[cur_depth] = node - ot->root;
                node = children + octant_cache[cur_depth];
                octant_cache[++cur_depth] = 0;
            }
            else
            {
                int8_t octant;
                for (octant = 0; octant < 8; octant++)
                {
                    parent_indexes[index + octant] = node - ot->root;
                }
                if (node == ot->root)
                {
                    return parent_indexes;
                }
                else
                {
                    node = ot->root + parent_cache[--cur_depth];
                    octant_cache[cur_depth]++;
                }
            }
        }
    }
}

void octree_optimize(octree_t *const ot)
{
    assert(ot != 0);
    octree_node_t *node = ot->root;
    uint32_t parent_cache[10] = {0};
    uint8_t octant_cache[10] = {0};
    uint8_t cur_depth = 0;
    uint32_t ideal_index = 1;
    if ((node[0] & MASK_TYPE) == MASK_EMPTY)
        return;
    const uint32_t *const parent_indexes = get_parent_indexes(ot);
    while (1)
    {
        if ((node[0] & MASK_TYPE) == MASK_LEAF)
        {
            node = ot->root + parent_cache[--cur_depth];
            octant_cache[cur_depth]++;
        }
        else
        {
            const uint32_t index = node[0] & MASK_INDEX;
            octree_node_t *children_1 = ot->root + index;
            if (octant_cache[cur_depth] == 0)
            {
                if (index != ideal_index)
                {
                    octree_node_t *const children_2 = ot->root + ideal_index;
                    int8_t octant;
                    for (octant = 0; octant < 8; octant++)
                    {
                        children_1[octant] ^= children_2[octant];
                        children_2[octant] ^= children_1[octant];
                        children_1[octant] ^= children_2[octant];
                    }
                    node[0] = (node[0] & MASK_TYPE) | ideal_index;
                    if (parent_indexes[ideal_index] != 0)
                    {
                        octree_node_t *const parent_2 = ot->root + parent_indexes[ideal_index];
                        if ((parent_2[0] & MASK_TYPE) == MASK_NODE)
                            parent_2[0] = (parent_2[0] & MASK_TYPE) | index;
                    }
                    children_1 = ot->root + ideal_index;
                }
                ideal_index += 8;
            }
            while ((children_1[octant_cache[cur_depth]] & MASK_TYPE) == MASK_EMPTY && octant_cache[cur_depth] < 8)
            {
                octant_cache[cur_depth]++;
            }
            if (octant_cache[cur_depth] < 8)
            {
                parent_cache[cur_depth] = node - ot->root;
                node = children_1 + octant_cache[cur_depth];
                octant_cache[++cur_depth] = 0;
            }
            else
            {
                if (node == ot->root)
                {
                    free(parent_indexes);
                    ot->root = realloc(ot->root, ot->node_count * sizeof(octree_node_t));
                    ot->capacity = ot->node_count;
                    ot->empty_list = realloc(ot->empty_list, sizeof(uint32_t));
                    ot->empty_count = 0;
                    ot->empty_capacity = 1;
                    return;
                }
                else
                {
                    node = ot->root + parent_cache[--cur_depth];
                    octant_cache[cur_depth]++;
                }
            }
        }
    }
}

void octree_print(const octree_t *const ot)
{
    assert(ot != 0);
    printf("Octree:\nGrid size: %d\nMax depth: %d\nNode count: %d\nCapacity: %d\n", ot->grid_size, ot->max_depth, ot->node_count, ot->capacity);
    const octree_node_t *node = ot->root;
    point_t cur_pos = {.x = 0, .y = 0, .z = 0};
    uint32_t cur_size = ot->grid_size;
    uint32_t parent_cache[10] = {0};
    uint8_t octant_cache[10] = {0};
    uint8_t cur_depth = 0;
    if ((node[0] & MASK_TYPE) == MASK_EMPTY)
        return;
    while (1)
    {
        if ((node[0] & MASK_TYPE) == MASK_LEAF)
        {
            printf("index: %d | xyz: %d, %d, %d | size: %d | value: %d\n", node - ot->root, cur_pos.x, cur_pos.y, cur_pos.z, cur_size, node[0] & MASK_COLOR);
            cur_pos = point_sum_mul(cur_pos, octant_pos_offset[octant_cache[--cur_depth]++], -cur_size);
            cur_size *= 2;
            node = ot->root + parent_cache[cur_depth];
        }
        else
        {
            const octree_node_t *const children = ot->root + (node[0] & MASK_INDEX);
            while ((children[octant_cache[cur_depth]] & MASK_TYPE) == MASK_EMPTY && octant_cache[cur_depth] < 8)
            {
                octant_cache[cur_depth]++;
            }
            if (octant_cache[cur_depth] < 8)
            {
                cur_size /= 2;
                parent_cache[cur_depth] = node - ot->root;
                node = children + octant_cache[cur_depth];
                cur_pos = point_sum_mul(cur_pos, octant_pos_offset[octant_cache[cur_depth++]], cur_size);
                octant_cache[cur_depth] = 0;
            }
            else
            {
                if (node == ot->root)
                {
                    return;
                }
                else
                {
                    cur_pos = point_sum_mul(cur_pos, octant_pos_offset[octant_cache[--cur_depth]++], -cur_size);
                    cur_size *= 2;
                    node = ot->root + parent_cache[cur_depth];
                }
            }
        }
    }
}

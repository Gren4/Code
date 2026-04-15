#include "octree_voxels.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static const uint8_t child_index_table[2][2][2] =
{{{0, 1},
  {2, 3}},
 {{4, 5},
  {6, 7}}};
static const point_t child_pos_offset[8] =
{{.x = 0, .y = 0, .z = 0},
 {.x = 0, .y = 0, .z = 1},
 {.x = 0, .y = 1, .z = 0},
 {.x = 0, .y = 1, .z = 1},
 {.x = 1, .y = 0, .z = 0},
 {.x = 1, .y = 0, .z = 1},
 {.x = 1, .y = 1, .z = 0},
 {.x = 1, .y = 1, .z = 1}};

octree_t octree(const int32_t grid_size)
{
    assert(grid_size % 2 == 0);
    const int32_t max_level = log2(grid_size);
    size_t data_size = 1;
    int32_t i;
    for (i = 0; i < max_level; i++)
    {
        const int32_t cur_grid_size = grid_size >> i;
        data_size += cur_grid_size * cur_grid_size * cur_grid_size;
    }
    octree_node_t *const data = calloc(data_size, sizeof(octree_node_t));
    assert(data != 0);
    return (octree_t){.data = data, .grid_size = grid_size};
}

static inline point_t point_sum(const point_t p1, const point_t p2)
{
    return (point_t){.x = p1.x + p2.x, .y = p1.y + p2.y, .z = p1.z + p2.z};
}

static inline point_t point_mul(const point_t p, const int32_t status)
{
    return (point_t){.x = p.x * status, .y = p.y * status, .z = p.z * status};
}

static inline size_t get_index(const octree_node_t *const node, const octree_node_t *const first_node)
{
    return node - first_node;
}

static inline octree_node_t* get_child(octree_node_t *const node, octree_node_t *const first_node, const uint8_t i)
{
    return first_node + 8 * get_index(node, first_node) + i + 1;
}

static inline octree_node_t* get_parent(octree_node_t *const node, octree_node_t *const first_node)
{
    const size_t index = get_index(node, first_node);
    return first_node + ((index - ((index - 1) % 8)) >> 3);
}

static void octree_update(octree_node_t *node, octree_node_t *const first_node, point_t cur_pos, int32_t cur_offset)
{
    for (; node != first_node;)
    {
        octree_node_t *parent = get_parent(node, first_node);
        uint8_t value_mask =
         (get_child(parent, first_node, 0)->status) |
         (get_child(parent, first_node, 1)->status << 1) |
         (get_child(parent, first_node, 2)->status << 2) |
         (get_child(parent, first_node, 3)->status << 3) |
         (get_child(parent, first_node, 4)->status << 4) |
         (get_child(parent, first_node, 5)->status << 5) |
         (get_child(parent, first_node, 6)->status << 6) |
         (get_child(parent, first_node, 7)->status << 7);
        if (value_mask != 0xFF && parent->status == 1)
        {
            node = parent;
            node->status = 0;
        }
        else if (value_mask == 0xFF && parent->status == 0)
        {
            node = parent;
            node->status = 1;
        }
        else
        {
            return;
        }
    }
}

void octree_set(octree_t *const ot, const point_t p)
{
    /* Проверяем, входит ли точка в куб */
    if ((uint32_t)p.x >= ot->grid_size ||
        (uint32_t)p.y >= ot->grid_size ||
        (uint32_t)p.z >= ot->grid_size)
    {
        /* Если нет, выходим */
        return;
    }
    octree_node_t *const first_node = ot->data;
    octree_node_t *node = first_node;
    point_t cur_pos = {.x = 0, .y = 0, .z = 0};
    int32_t cur_offset = ot->grid_size;
    for (;;)
    {
        /* Уже задали точку до этого, выходим */
        if (node->status == 1)
        {
            return;
        }
         /* Достигли самого нижнего уровня */
        else if (cur_offset == 1)
        {
            node->status = 1;
            octree_update(node, first_node, cur_pos, cur_offset);
            return;
        }
        /* На промежуточном уровне, выбираем потомка ноды для дальнейшего анализа */
        else
        {
            cur_offset >>= 1;
            const uint8_t dx = p.x >= (cur_pos.x + cur_offset);
            const uint8_t dy = p.y >= (cur_pos.y + cur_offset);
            const uint8_t dz = p.z >= (cur_pos.z + cur_offset);
            const int8_t child_index = child_index_table[dx][dy][dz];
            node->children_status |= 1 << child_index;
            node = get_child(node, first_node, child_index);
            cur_pos = point_sum(cur_pos, point_mul(child_pos_offset[child_index], cur_offset));
        }
    }
}

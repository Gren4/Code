#include "octree_voxels.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_GRID_SIZE 1024
#define MAX_DEPTH 10
#define MASK_TYPE 0xC0000000U
#define MASK_EMPTY 0x00000000U
#define MASK_LEAF 0x40000000U
#define MASK_NODE 0x80000000U
#define MASK_INDEX 0x3FFFFFFFU
#define MASK_COLOR 0x00FFFFFFU

#define INVALID_VOXEL VOXEL(POINT(-1, -1, -1), COLOR(0, 0, 0), 0)

static const uint8_t traversal_order[8][8] = {
    // sign = (0,0,0)
    {0, 1, 2, 3, 4, 5, 6, 7},
    // sign = (0,0,1)
    {1, 0, 3, 2, 5, 4, 7, 6},
    // sign = (0,1,0)
    {2, 3, 0, 1, 6, 7, 4, 5},
    // sign = (0,1,1)
    {3, 2, 1, 0, 7, 6, 5, 4},
    // sign = (1,0,0)
    {4, 5, 6, 7, 0, 1, 2, 3},
    // sign = (1,0,1)
    {5, 4, 7, 6, 1, 0, 3, 2},
    // sign = (1,1,0)
    {6, 7, 4, 5, 2, 3, 0, 1},
    // sign = (1,1,1)
    {7, 6, 5, 4, 3, 2, 1, 0}};

octree_t octree(const uint32_t grid_size, const uint32_t min_size)
{
    assert(grid_size % 2 == 0);
    assert(grid_size <= MAX_GRID_SIZE);
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

void octree_clear(octree_t *const ot)
{
    assert(ot != 0);
    assert(ot->root != 0);
    assert(ot->empty_list != 0);
    ot->node_count = 1;
    ot->root = realloc(ot->root, ot->node_count * sizeof(octree_node_t));
    ot->capacity = ot->node_count;
    ot->empty_list = realloc(ot->empty_list, sizeof(uint32_t));
    ot->empty_count = 0;
    ot->empty_capacity = 1;
    return;
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

static inline uint32_t pack_index(const uint32_t index)
{
    return (index - 1) / 8;
}

static inline uint32_t unpack_index(const octree_node_t node)
{
    return (node & MASK_INDEX) * 8 + 1;
}

static inline color_t unpack_color(const octree_node_t node)
{
    return COLOR((node >> 16) & 0xFF, (node >> 8) & 0xFF, node & 0xFF);
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
            ot->capacity += 32;
            ot->root = realloc(ot->root, ot->capacity * sizeof(octree_node_t));
            assert(ot->root != 0);
            memset(ot->root + index, 0, 32 * sizeof(octree_node_t));
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
        ot->empty_capacity *= 2;
        ot->empty_list = realloc(ot->empty_list, ot->empty_capacity * sizeof(uint32_t));
        assert(ot->empty_list != 0);
    }
    ot->empty_list[ot->empty_count++] = index;
    return;
}

static inline voxel_t convert_to_voxel(const point_t position, const octree_node_t node, const uint32_t size)
{
    return VOXEL(position, unpack_color(node), size);
}

voxel_t octree_get_voxel(octree_t *const ot, const point_t p)
{
    assert(ot != 0);
    if ((uint32_t)p.x >= ot->grid_size || (uint32_t)p.y >= ot->grid_size || (uint32_t)p.z >= ot->grid_size)
        return INVALID_VOXEL;
    octree_node_t *node = ot->root;
    point_t cur_pos = {.x = 0, .y = 0, .z = 0};
    uint32_t cur_size = ot->grid_size;
    while (1)
    {
        const uint32_t node_type = node[0] & MASK_TYPE;
        if (node_type == MASK_LEAF)
            return convert_to_voxel(cur_pos, node[0], cur_size);
        else if (node_type == MASK_EMPTY)
            return INVALID_VOXEL;
        cur_size /= 2;
        const point_t center = POINT(cur_pos.x + cur_size,
                                     cur_pos.y + cur_size,
                                     cur_pos.z + cur_size);
        const uint8_t octant = ((p.x >= center.x) ? 4 : 0) | ((p.y >= center.y) ? 2 : 0) | ((p.z >= center.z) ? 1 : 0);
        cur_pos = POINT(cur_pos.x + (octant & 4 ? cur_size : 0),
                        cur_pos.y + (octant & 2 ? cur_size : 0),
                        cur_pos.z + (octant & 1 ? cur_size : 0));
        node = ot->root + unpack_index(node[0]) + octant;
    }
}

void octree_set_voxel(octree_t *const ot, const point_t p, const color_t color)
{
    assert(ot != 0);
    if ((uint32_t)p.x >= ot->grid_size || (uint32_t)p.y >= ot->grid_size || (uint32_t)p.z >= ot->grid_size)
        return;
    octree_node_t *node = ot->root;
    point_t cur_pos = {.x = 0, .y = 0, .z = 0};
    uint32_t cur_size = ot->grid_size;
    uint32_t parent_cache[MAX_DEPTH] = {0};
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
                const uint32_t index = unpack_index(*parent);
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
            const uint32_t node_type = node[0] & MASK_TYPE;
            if (node_type == MASK_LEAF)
            {
                const uint32_t node_color = node[0] & MASK_COLOR;
                if (node_color == packed_color)
                    return;
                const uint32_t node_data = MASK_LEAF | node_color;
                const uint32_t index = ask_for_index(ot, &node);
                node[0] = MASK_NODE | pack_index(index);
                octree_node_t *const children = ot->root + index;
                int8_t octant;
                for (octant = 0; octant < 8; octant++)
                {
                    children[octant] = node_data;
                }
            }
            else if (node_type == MASK_EMPTY)
            {
                const uint32_t index = ask_for_index(ot, &node);
                node[0] = MASK_NODE | pack_index(index);
            }
            parent_cache[cur_depth++] = node - ot->root;
            cur_size /= 2;
            const point_t center = POINT(cur_pos.x + cur_size,
                                         cur_pos.y + cur_size,
                                         cur_pos.z + cur_size);
            ;
            const uint8_t octant = ((p.x >= center.x) ? 4 : 0) | ((p.y >= center.y) ? 2 : 0) | ((p.z >= center.z) ? 1 : 0);
            cur_pos = POINT(cur_pos.x + (octant & 4 ? cur_size : 0),
                            cur_pos.y + (octant & 2 ? cur_size : 0),
                            cur_pos.z + (octant & 1 ? cur_size : 0));
            node = ot->root + unpack_index(node[0]) + octant;
        }
    }
}

void octree_unset_voxel(octree_t *const ot, const point_t p)
{
    assert(ot != 0);
    if ((uint32_t)p.x >= ot->grid_size || (uint32_t)p.y >= ot->grid_size || (uint32_t)p.z >= ot->grid_size)
        return;
    octree_node_t *node = ot->root;
    point_t cur_pos = {.x = 0, .y = 0, .z = 0};
    uint32_t cur_size = ot->grid_size;
    uint32_t parent_cache[MAX_DEPTH] = {0};
    uint8_t cur_depth = 0;
    while (1)
    {
        if (cur_depth == ot->max_depth)
        {
            const uint32_t node_data = MASK_EMPTY;
            node[0] = node_data;
            while (node != ot->root)
            {
                octree_node_t *const parent = ot->root + parent_cache[--cur_depth];
                const uint32_t index = unpack_index(*parent);
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
            const uint32_t node_type = node[0] & MASK_TYPE;
            if (node_type == MASK_EMPTY)
                return;
            else if (node_type == MASK_LEAF)
            {
                const uint32_t node_color = node[0] & MASK_COLOR;
                const uint32_t node_data = MASK_LEAF | node_color;
                const uint32_t index = ask_for_index(ot, &node);
                node[0] = MASK_NODE | pack_index(index);
                octree_node_t *const children = ot->root + index;
                int8_t octant;
                for (octant = 0; octant < 8; octant++)
                {
                    children[octant] = node_data;
                }
            }
            parent_cache[cur_depth++] = node - ot->root;
            cur_size /= 2;
            const point_t center = POINT(cur_pos.x + cur_size,
                                         cur_pos.y + cur_size,
                                         cur_pos.z + cur_size);
            const uint8_t octant = ((p.x >= center.x) ? 4 : 0) | ((p.y >= center.y) ? 2 : 0) | ((p.z >= center.z) ? 1 : 0);
            cur_pos = POINT(cur_pos.x + (octant & 4 ? cur_size : 0),
                            cur_pos.y + (octant & 2 ? cur_size : 0),
                            cur_pos.z + (octant & 1 ? cur_size : 0));
            node = ot->root + unpack_index(node[0]) + octant;
        }
    }
}

static uint32_t *get_parent_indexes(const octree_t *const ot)
{
    uint32_t *const parent_indexes = calloc(ot->capacity, sizeof(uint32_t));
    assert(parent_indexes != 0);
    const octree_node_t *node = ot->root;
    uint32_t parent_cache[MAX_DEPTH] = {0};
    uint8_t octant_cache[MAX_DEPTH] = {0};
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
            const uint32_t index = unpack_index(node[0]);
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
    uint32_t parent_cache[MAX_DEPTH] = {0};
    uint8_t octant_cache[MAX_DEPTH] = {0};
    uint8_t cur_depth = 0;
    uint32_t ideal_index = 1;
    if ((node[0] & MASK_TYPE) == MASK_EMPTY)
        return;
    uint32_t *const parent_indexes = get_parent_indexes(ot);
    while (1)
    {
        const uint32_t node_type = node[0] & MASK_TYPE;
        if (node_type == MASK_LEAF)
        {
            node = ot->root + parent_cache[--cur_depth];
            octant_cache[cur_depth]++;
        }
        else
        {
            const uint32_t index = unpack_index(node[0]);
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
                    node[0] = node_type | pack_index(ideal_index);
                    if (parent_indexes[ideal_index] != 0)
                    {
                        octree_node_t *const parent_2 = ot->root + parent_indexes[ideal_index];
                        const uint32_t parent_2_type = parent_2[0] & MASK_TYPE;
                        if (parent_2_type == MASK_NODE)
                            parent_2[0] = parent_2_type | pack_index(index);
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
    printf("Octree:\nGrid size: %d\nMax depth: %d\nNode count: %d\nCapacity: %d\n",
           ot->grid_size,
           ot->max_depth,
           ot->node_count,
           ot->capacity);
    const octree_node_t *node = ot->root;
    point_t cur_pos = {.x = 0, .y = 0, .z = 0};
    uint32_t cur_size = ot->grid_size;
    uint32_t parent_cache[MAX_DEPTH] = {0};
    uint8_t octant_cache[MAX_DEPTH] = {0};
    uint8_t cur_depth = 0;
    if ((node[0] & MASK_TYPE) == MASK_EMPTY)
        return;
    while (1)
    {
        if ((node[0] & MASK_TYPE) == MASK_LEAF)
        {
            const color_t color = unpack_color(node[0]);
            printf("index: %lld | xyz: %d, %d, %d | rgb: %d, %d, %d | size: %d\n",
                   node - ot->root,
                   cur_pos.x,
                   cur_pos.y,
                   cur_pos.z,
                   color.r,
                   color.g,
                   color.b,
                   cur_size);
            cur_depth--;
            cur_pos = POINT(cur_pos.x - (octant_cache[cur_depth] & 4 ? cur_size : 0),
                            cur_pos.y - (octant_cache[cur_depth] & 2 ? cur_size : 0),
                            cur_pos.z - (octant_cache[cur_depth] & 1 ? cur_size : 0));
            octant_cache[cur_depth]++;
            cur_size *= 2;
            node = ot->root + parent_cache[cur_depth];
        }
        else
        {
            const octree_node_t *const children = ot->root + unpack_index(node[0]);
            while ((children[octant_cache[cur_depth]] & MASK_TYPE) == MASK_EMPTY && octant_cache[cur_depth] < 8)
            {
                octant_cache[cur_depth]++;
            }
            if (octant_cache[cur_depth] < 8)
            {
                cur_size /= 2;
                parent_cache[cur_depth] = node - ot->root;
                node = children + octant_cache[cur_depth];
                cur_pos = POINT(cur_pos.x + (octant_cache[cur_depth] & 4 ? cur_size : 0),
                                cur_pos.y + (octant_cache[cur_depth] & 2 ? cur_size : 0),
                                cur_pos.z + (octant_cache[cur_depth] & 1 ? cur_size : 0));
                cur_depth++;
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
                    cur_depth--;
                    cur_pos = POINT(cur_pos.x - (octant_cache[cur_depth] & 4 ? cur_size : 0),
                                    cur_pos.y - (octant_cache[cur_depth] & 2 ? cur_size : 0),
                                    cur_pos.z - (octant_cache[cur_depth] & 1 ? cur_size : 0));
                    octant_cache[cur_depth]++;
                    cur_size *= 2;
                    node = ot->root + parent_cache[cur_depth];
                }
            }
        }
    }
}

static inline float pow2f(const float x)
{
    return x * x;
}

static inline void normalized_vec3(float *const vec3)
{
    const float length = sqrtf(pow2f(vec3[0]) + pow2f(vec3[1]) + pow2f(vec3[2]));
    assert(length > 1e-5);
    int32_t i;
    for (i = 0; i < 3; i++)
    {
        vec3[i] /= length;
    }
    return;
}

static inline bool ray_intersect_aabb(const point_t position, const uint32_t size, const point_t start, const float *const direction, const float *const inv_direction, float *const t_min, float *const t_max)
{
    float tmin = 0.0;
    float tmax = __FLT_MAX__;
    int32_t i;
    for (i = 0; i < 3; i++)
    {
        if (direction[i] == 0.0f) {
            if (start.raw[i] < position.raw[i] || start.raw[i] > position.raw[i] + size)
                return false;
            continue;
        }
        const float distance = (float)position.raw[i] - (float)start.raw[i];
        float t0 = distance * inv_direction[i];
        float t1 = (distance + (float)size) * inv_direction[i];
        if (inv_direction[i] < 0.0f)
        {
            float tmp = t0;
            t0 = t1;
            t1 = tmp;
        }
        tmin = fmaxf(tmin, t0);
        tmax = fminf(tmax, t1);
        if (tmax < tmin)
            return false;
    }
    *t_min = tmin;
    *t_max = tmax;
    return true;
}

ray_hit_t octree_ray_cast(const octree_t *const ot, const point_t start, const point_t end, float max_dist)
{
    ray_hit_t result = {
        .distance = __FLT_MAX__,
        .voxel = INVALID_VOXEL,
        .hit = false};
    float direction[3] = {
        end.x - start.x,
        end.y - start.y,
        end.z - start.z};
    normalized_vec3(direction);
    const float inv_direction[3] = {
        1.0f / direction[0],
        1.0f / direction[1],
        1.0f / direction[2]};
    float t_min;
    float t_max;
    if (ray_intersect_aabb(POINT(0, 0, 0), ot->grid_size, start, direction, inv_direction, &t_min, &t_max) == false)
        return result;
    if (t_min > max_dist || t_max < 0.0f)
        return result;
    typedef struct stack_item_t
    {
        const octree_node_t *node;
        point_t pos;
        uint32_t size;
        float t_min;
    } stack_item_t;
    stack_item_t stack[MAX_DEPTH * 8 + 4];
    int32_t stack_size = 1;
    stack[0] = (stack_item_t){
        .node = ot->root,
        .pos = POINT(0, 0, 0),
        .size = ot->grid_size,
        .t_min = t_min};
    int8_t sign_x = (direction[0] < 0.0f);
    int8_t sign_y = (direction[1] < 0.0f);
    int8_t sign_z = (direction[2] < 0.0f);
    int8_t sign_index = (sign_x << 2) | (sign_y << 1) | sign_z;
    const uint8_t *order = traversal_order[sign_index];
    while (stack_size > 0)
    {
        stack_item_t item = stack[--stack_size];
        if (item.t_min >= result.distance)
            continue;
        const uint32_t node_type = item.node[0] & MASK_TYPE;
        if (node_type == MASK_LEAF)
        {
            result.distance = item.t_min;
            result.voxel.position = item.pos;
            result.voxel.color = unpack_color(item.node[0]);
            result.voxel.size = item.size;
            result.hit = true;
            continue;
        }
        else if (node_type == MASK_EMPTY)
            continue;
        const octree_node_t *children = ot->root + unpack_index(item.node[0]);
        const uint32_t half = item.size / 2;
        int8_t i;
        for (i = 7; i >= 0; i--)
        {
            const uint8_t octant = order[i];
            const point_t child_pos = POINT(item.pos.x + (octant & 4 ? half : 0),
                                            item.pos.y + (octant & 2 ? half : 0),
                                            item.pos.z + (octant & 1 ? half : 0));
            float t_child_min;
            float t_child_max;
            if (ray_intersect_aabb(child_pos, half, start, direction, inv_direction, &t_child_min, &t_child_max) == true &&
                t_child_min <= max_dist && t_child_min < result.distance)
            {
                stack[stack_size++] = (stack_item_t){
                    .node = children + octant,
                    .pos = child_pos,
                    .size = half,
                    .t_min = t_child_min};
            }
        }
    }
    return result;
}

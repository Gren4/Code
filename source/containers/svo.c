#include "svo.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NODES_START_CAPACITY 9
#define INDEXES_START_CAPACITY 4

#define MAX_GRID_SIZE 1024
#define MAX_DEPTH 10
#define MASK_TYPE 0xC0000000U
#define MASK_EMPTY 0x00000000U
#define MASK_LEAF 0x40000000U
#define MASK_NODE 0x80000000U
#define MASK_CHILDREN 0x3FFFFFFFU
#define MASK_COLOR 0x00FFFFFFU

#define INVALID_VOXEL VOXEL(AABB(POINT(-1, -1, -1), 0), COLOR(0, 0, 0, 0))

static inline svo_nodes_t create_nodes(void)
{
    uint32_t *const iroot = calloc(NODES_START_CAPACITY, sizeof(uint32_t));
    assert(iroot != 0);
    uint32_t *const croot = calloc(NODES_START_CAPACITY, sizeof(uint32_t));
    assert(croot != 0);
    return (svo_nodes_t){.iroot = iroot,
                         .croot = croot,
                         .count = 1,
                         .capacity = NODES_START_CAPACITY};
}

static inline void clear_nodes(svo_nodes_t *const nodes)
{
    nodes->iroot = realloc(nodes->iroot, NODES_START_CAPACITY * sizeof(uint32_t));
    memset(nodes->iroot, 0, NODES_START_CAPACITY * sizeof(uint32_t));
    nodes->croot = realloc(nodes->croot, NODES_START_CAPACITY * sizeof(uint32_t));
    memset(nodes->croot, 0, NODES_START_CAPACITY * sizeof(uint32_t));
    nodes->count = 1;
    nodes->capacity = NODES_START_CAPACITY;
    return;
}

static inline void free_nodes(svo_nodes_t *const nodes)
{
    free(nodes->iroot);
    free(nodes->croot);
    return;
}

static inline void adjust_nodes(svo_nodes_t *const nodes)
{
    nodes->iroot = realloc(nodes->iroot, nodes->count * sizeof(uint32_t));
    nodes->croot = realloc(nodes->croot, nodes->count * sizeof(uint32_t));
    nodes->capacity = nodes->count;
    return;
}

static inline void increase_nodes(svo_nodes_t *const nodes)
{
    if (nodes->count + 8 >= nodes->capacity)
    {
        nodes->capacity += 32;
        nodes->iroot = realloc(nodes->iroot, nodes->capacity * sizeof(uint32_t));
        assert(nodes->iroot != 0);
        memset(nodes->iroot + nodes->count, 0, 32 * sizeof(uint32_t));
        nodes->croot = realloc(nodes->croot, nodes->capacity * sizeof(uint32_t));
        assert(nodes->croot != 0);
        memset(nodes->croot + nodes->count, 0, 32 * sizeof(uint32_t));
    }
    nodes->count += 8;
    return;
}

static inline svo_queue_t create_spare(void)
{
    uint32_t *const spare = calloc(INDEXES_START_CAPACITY, sizeof(uint32_t));
    assert(spare != 0);
    return (svo_queue_t){.queue = spare,
                         .count = 0,
                         .offset = 0,
                         .capacity = INDEXES_START_CAPACITY};
}

static inline void clear_spare(svo_queue_t *const spare)
{
    spare->queue = realloc(spare->queue, INDEXES_START_CAPACITY * sizeof(uint32_t));
    memset(spare->queue, 0, INDEXES_START_CAPACITY * sizeof(uint32_t));
    spare->count = 0;
    spare->offset = 0;
    spare->capacity = INDEXES_START_CAPACITY;
    return;
}

static inline void free_spare(svo_queue_t *const spare)
{
    free(spare->queue);
    return;
}

static inline void add_spare(svo_queue_t *const spare, const uint32_t index)
{
    spare->queue[(spare->offset + spare->count) % spare->capacity] = index;
    spare->count++;
    if (spare->count == spare->capacity)
    {
        spare->queue = realloc(spare->queue, spare->capacity * 2);
        assert(spare->queue != 0);
        memmove(spare->queue + spare->capacity, spare->queue, spare->offset * sizeof(uint32_t));
        spare->capacity *= 2;
    }
    return;
}

static inline uint32_t pop_spare(svo_queue_t *const spare)
{
    const uint32_t result = spare->queue[spare->offset];
    spare->offset = (spare->offset + 1) % spare->capacity;
    if (--spare->count == 0 && spare->capacity >= 8 * INDEXES_START_CAPACITY)
    {
        spare->queue = realloc(spare->queue, INDEXES_START_CAPACITY);
        spare->offset = 0;
    }
    return result;
}

svo_t svo(const uint32_t grid_size, const uint32_t min_size)
{
    assert(grid_size % 2 == 0);
    assert(grid_size <= MAX_GRID_SIZE);
    assert(min_size % 2 == 0 || min_size == 1);
    assert(min_size < grid_size);
    return (svo_t){.nodes = create_nodes(),
                   .spare = create_spare(),
                   .grid_size = grid_size,
                   .max_depth = log2(grid_size / min_size)};
}

void svo_clear(svo_t *const svo)
{
    clear_nodes(&svo->nodes);
    clear_spare(&svo->spare);
    return;
}

void svo_free(svo_t *const svo)
{
    free_nodes(&svo->nodes);
    free_spare(&svo->spare);
    return;
}

void svo_adjust(svo_t *const svo)
{
    adjust_nodes(&svo->nodes);
    clear_spare(&svo->spare);
    return;
}

static inline bool is_in_grid(const svo_t *const svo, const point_t *const point)
{
    return (uint32_t)point->x < svo->grid_size &&
           (uint32_t)point->y < svo->grid_size &&
           (uint32_t)point->z < svo->grid_size;
}

static inline uint32_t pack_children(const uint32_t children)
{
    return (children - 1) / 8;
}

static inline uint32_t unpack_children(const uint32_t node)
{
    return (node & MASK_CHILDREN) * 8 + 1;
}

static inline uint32_t pack_color(const color_t color)
{
    return color.r << 24 | color.g << 16 | color.b << 8 | color.a;
}

static inline color_t unpack_color(const uint32_t node)
{
    return COLOR((node >> 24) & 0xFF, (node >> 16) & 0xFF, (node >> 8) & 0xFF, node & 0xFF);
}

static inline uint32_t get_type(const svo_t *const svo, const uint32_t index)
{
    return svo->nodes.iroot[index] & MASK_TYPE;
}

static inline uint32_t get_children(const svo_t *const svo, const uint32_t index)
{
    return unpack_children(svo->nodes.iroot[index]);
}

static inline color_t get_color(const svo_t *const svo, const uint32_t index)
{
    return unpack_color(svo->nodes.croot[index]);
}

static inline uint32_t get_raw_color(const svo_t *const svo, const uint32_t index)
{
    return svo->nodes.croot[index];
}

static inline void set_empty(const svo_t *const svo, const uint32_t index)
{
    svo->nodes.iroot[index] = MASK_EMPTY;
    svo->nodes.croot[index] = 0x0;
    return;
}

static inline void set_children(const svo_t *const svo, const uint32_t index, const uint32_t children)
{
    svo->nodes.iroot[index] = MASK_NODE | pack_children(children);
    return;
}

static inline void set_color(svo_t *const svo, const uint32_t index, const color_t color)
{
    svo->nodes.iroot[index] = MASK_LEAF;
    svo->nodes.croot[index] = pack_color(color);
    return;
}

static inline void set_raw_color(svo_t *const svo, const uint32_t index, const uint32_t raw_color)
{
    svo->nodes.iroot[index] = MASK_LEAF;
    svo->nodes.croot[index] = raw_color;
    return;
}

static inline void swap_nodes(svo_t *const svo, const uint32_t index_1, const uint32_t index_2)
{
    svo->nodes.iroot[index_1] ^= svo->nodes.iroot[index_2];
    svo->nodes.iroot[index_2] ^= svo->nodes.iroot[index_1];
    svo->nodes.iroot[index_1] ^= svo->nodes.iroot[index_2];
    svo->nodes.croot[index_1] ^= svo->nodes.croot[index_2];
    svo->nodes.croot[index_2] ^= svo->nodes.croot[index_1];
    svo->nodes.croot[index_1] ^= svo->nodes.croot[index_2];
    return;
}

static inline uint32_t ask_for_index(svo_t *const svo)
{
    if (svo->spare.count != 0)
    {
        return pop_spare(&svo->spare);
    }
    else
    {
        const uint32_t index = svo->nodes.count;
        increase_nodes(&svo->nodes);
        return index;
    }
}

static inline void add_to_spare(svo_t *const svo, const uint32_t index)
{
    memset(svo->nodes.iroot + index, 0, 8 * sizeof(uint32_t));
    add_spare(&svo->spare, index);
    return;
}

static inline int8_t find_octant_and_update_aabb(aabb_t *const aabb, const point_t *const point)
{
    aabb->offset /= 2;
    const point_t center = POINT(aabb->point.x + aabb->offset,
                                 aabb->point.y + aabb->offset,
                                 aabb->point.z + aabb->offset);
    int8_t octant = 0;
    if (point->x >= center.x)
    {
        octant |= 4;
        aabb->point.x += aabb->offset;
    }
    if (point->y >= center.y)
    {
        octant |= 2;
        aabb->point.y += aabb->offset;
    }
    if (point->z >= center.z)
    {
        octant |= 1;
        aabb->point.z += aabb->offset;
    }
    return octant;
}

voxel_t svo_get(svo_t *const svo, const point_t point)
{
    if (is_in_grid(svo, &point) == false)
        return INVALID_VOXEL;
    aabb_t aabb = AABB(POINT(0, 0, 0), svo->grid_size);
    uint32_t i = 0;
    while (1)
    {
        const uint32_t node_type = get_type(svo, i);
        if (node_type == MASK_LEAF)
            return VOXEL(aabb, get_color(svo, i));
        else if (node_type == MASK_EMPTY)
            return INVALID_VOXEL;
        const int8_t octant = find_octant_and_update_aabb(&aabb, &point);
        i = get_children(svo, i) + octant;
    }
}

void svo_set(svo_t *const svo, const point_t point, const color_t color)
{
    if (is_in_grid(svo, &point) == false)
        return;
    uint32_t parent_stack[MAX_DEPTH] = {0};
    aabb_t aabb = AABB(POINT(0, 0, 0), svo->grid_size);
    const uint32_t packed_color = color.r << 24 | color.g << 16 | color.b << 8 | color.a;
    uint32_t i = 0;
    uint8_t cur_depth = 0;
    while (1)
    {
        if (cur_depth == svo->max_depth)
        {
            set_raw_color(svo, i, packed_color);
            while (i != 0)
            {
                cur_depth--;
                const uint32_t children = get_children(svo, parent_stack[cur_depth]);
                int8_t octant = 0;
                while (get_type(svo, children + octant) == MASK_LEAF &&
                       get_raw_color(svo, children + octant) == packed_color &&
                       octant < 8)
                {
                    octant++;
                }
                if (octant != 8)
                    return;
                add_to_spare(svo, children);
                i = parent_stack[cur_depth];
                set_raw_color(svo, i, packed_color);
            }
            return;
        }
        else
        {
            const uint32_t node_type = get_type(svo, i);
            if (node_type == MASK_LEAF)
            {
                const uint32_t node_color = get_raw_color(svo, i);
                if (node_color == packed_color)
                    return;
                const uint32_t children = ask_for_index(svo);
                set_children(svo, i, children);
                int8_t octant;
                for (octant = 0; octant < 8; octant++)
                {
                    set_raw_color(svo, children + octant, node_color);
                }
            }
            else if (node_type == MASK_EMPTY)
            {
                const uint32_t children = ask_for_index(svo);
                set_children(svo, i, children);
            }
            parent_stack[cur_depth] = i;
            cur_depth++;
            const int8_t octant = find_octant_and_update_aabb(&aabb, &point);
            i = get_children(svo, i) + octant;
        }
    }
}

void svo_unset(svo_t *const svo, const point_t point)
{
    if (is_in_grid(svo, &point) == false)
        return;
    uint32_t parent_stack[MAX_DEPTH] = {0};
    aabb_t aabb = AABB(POINT(0, 0, 0), svo->grid_size);
    uint32_t i = 0;
    uint8_t cur_depth = 0;
    while (1)
    {
        if (cur_depth == svo->max_depth)
        {
            set_empty(svo, i);
            while (i != 0)
            {
                cur_depth--;
                const uint32_t children = get_children(svo, parent_stack[cur_depth]);
                int8_t octant = 0;
                while (get_type(svo, children + octant) == MASK_EMPTY &&
                       octant < 8)
                {
                    octant++;
                }
                if (octant != 8)
                    return;
                add_to_spare(svo, children);
                i = parent_stack[cur_depth];
                set_empty(svo, i);
            }
            return;
        }
        else
        {
            const uint32_t node_type = get_type(svo, i);
            if (node_type == MASK_EMPTY)
                return;
            else if (node_type == MASK_LEAF)
            {
                const uint32_t node_color = get_raw_color(svo, i);
                const uint32_t children = ask_for_index(svo);
                set_children(svo, i, children);
                int8_t octant;
                for (octant = 0; octant < 8; octant++)
                {
                    set_raw_color(svo, children + octant, node_color);
                }
            }
            parent_stack[cur_depth] = i;
            cur_depth++;
            const int8_t octant = find_octant_and_update_aabb(&aabb, &point);
            i = get_children(svo, i) + octant;
        }
    }
}

static uint32_t *get_parent_indexes(const svo_t *const svo)
{
    uint32_t *const parent_indexes = calloc(svo->nodes.capacity, sizeof(uint32_t));
    assert(parent_indexes != 0);
    uint32_t parent_stack[MAX_DEPTH] = {0};
    uint32_t i = 0;
    int8_t octant_stack[MAX_DEPTH] = {0};
    uint8_t cur_depth = 0;
    while (1)
    {
        const uint32_t children = get_children(svo, i);
        while (get_type(svo, children + octant_stack[cur_depth]) != MASK_NODE &&
               octant_stack[cur_depth] < 8)
        {
            octant_stack[cur_depth]++;
        }
        if (octant_stack[cur_depth] < 8)
        {
            parent_stack[cur_depth] = i;
            i = children + octant_stack[cur_depth];
            cur_depth++;
            octant_stack[cur_depth] = 0;
        }
        else
        {
            int8_t octant;
            for (octant = 0; octant < 8; octant++)
            {
                parent_indexes[children + octant] = i;
            }
            if (i == 0)
            {
                return parent_indexes;
            }
            else
            {
                cur_depth--;
                i = parent_stack[cur_depth];
                octant_stack[cur_depth]++;
            }
        }
    }
}

void svo_optimize(svo_t *const svo)
{
    if (get_type(svo, 0) == MASK_EMPTY)
        return;
    uint32_t *const parent_indexes = get_parent_indexes(svo);
    uint32_t parent_stack[MAX_DEPTH] = {0};
    uint32_t i = 0;
    uint32_t children_ideal = 1;
    int8_t octant_stack[MAX_DEPTH] = {0};
    uint8_t cur_depth = 0;
    while (1)
    {
        const uint32_t node_type = get_type(svo, i);
        if (node_type == MASK_LEAF)
        {
            cur_depth--;
            i = parent_stack[cur_depth];
            octant_stack[cur_depth]++;
        }
        else
        {
            const uint32_t children_1 = get_children(svo, i);
            if (octant_stack[cur_depth] == 0)
            {
                if (children_1 != children_ideal)
                {
                    int8_t octant;
                    for (octant = 0; octant < 8; octant++)
                    {
                        swap_nodes(svo, children_1 + octant, children_ideal + octant);
                        parent_indexes[children_1 + octant] ^= parent_indexes[children_ideal + octant];
                        parent_indexes[children_ideal + octant] ^= parent_indexes[children_1 + octant];
                        parent_indexes[children_1 + octant] ^= parent_indexes[children_ideal + octant];
                    }
                    set_children(svo, i, children_ideal);
                    if (parent_indexes[children_1] != 0)
                    {
                        if (get_type(svo, parent_indexes[children_1]) == MASK_NODE)
                        {
                            set_children(svo, parent_indexes[children_1], children_1);
                        }
                    }
                }
                children_ideal += 8;
            }
            while (get_type(svo, children_1 + octant_stack[cur_depth]) == MASK_EMPTY &&
                   octant_stack[cur_depth] < 8)
            {
                octant_stack[cur_depth]++;
            }
            if (octant_stack[cur_depth] < 8)
            {
                parent_stack[cur_depth] = i;
                i = children_1 + octant_stack[cur_depth];
                cur_depth++;
                octant_stack[cur_depth] = 0;
            }
            else
            {
                if (i == 0)
                {
                    free(parent_indexes);
                    svo_adjust(svo);
                    return;
                }
                else
                {
                    cur_depth--;
                    i = parent_stack[cur_depth];
                    octant_stack[cur_depth]++;
                }
            }
        }
    }
}

static inline void update_aabb_down(aabb_t *const aabb, const int8_t octant)
{
    aabb->offset /= 2;
    aabb->point.x += (octant & 4) ? aabb->offset : 0;
    aabb->point.y += (octant & 2) ? aabb->offset : 0;
    aabb->point.z += (octant & 1) ? aabb->offset : 0;
    return;
}

static inline void update_aabb_up(aabb_t *const aabb, const int8_t octant)
{
    aabb->point.x -= (octant & 4) ? aabb->offset : 0;
    aabb->point.y -= (octant & 2) ? aabb->offset : 0;
    aabb->point.z -= (octant & 1) ? aabb->offset : 0;
    aabb->offset *= 2;
    return;
}

void svo_print(svo_t *const svo)
{
    if (get_type(svo, 0) == MASK_EMPTY)
        return;
    printf("Octree:\nGrid size: %d\nMax depth: %d\nNode count: %d\nCapacity: %d\n",
           svo->grid_size,
           svo->max_depth,
           svo->nodes.count,
           svo->nodes.capacity);
    aabb_t aabb = AABB(POINT(0, 0, 0), svo->grid_size);
    uint32_t parent_stack[MAX_DEPTH] = {0};
    uint32_t i = 0;
    int8_t octant_stack[MAX_DEPTH] = {0};
    uint8_t cur_depth = 0;
    while (1)
    {
        const uint32_t node_type = get_type(svo, i);
        if (node_type == MASK_LEAF)
        {
            const color_t color = get_color(svo, i);
            printf("index: %d | xyz: %d, %d, %d | size: %d | rgba: %d, %d, %d, %d\n",
                   i,
                   aabb.point.x,
                   aabb.point.y,
                   aabb.point.z,
                   aabb.offset,
                   color.r,
                   color.g,
                   color.b,
                   color.a);
            cur_depth--;
            i = parent_stack[cur_depth];
            update_aabb_up(&aabb, octant_stack[cur_depth]);
            octant_stack[cur_depth]++;
        }
        else
        {
            const uint32_t children = get_children(svo, i);
            while (get_type(svo, children + octant_stack[cur_depth]) == MASK_EMPTY &&
                   octant_stack[cur_depth] < 8)
            {
                octant_stack[cur_depth]++;
            }
            if (octant_stack[cur_depth] < 8)
            {
                parent_stack[cur_depth] = i;
                i = children + octant_stack[cur_depth];
                update_aabb_down(&aabb, octant_stack[cur_depth]);
                cur_depth++;
                octant_stack[cur_depth] = 0;
            }
            else
            {
                if (i == 0)
                {
                    return;
                }
                else
                {
                    cur_depth--;
                    i = parent_stack[cur_depth];
                    update_aabb_up(&aabb, octant_stack[cur_depth]);
                    octant_stack[cur_depth]++;
                }
            }
        }
    }
}

// static inline float pow2f(const float x)
// {
//     return x * x;
// }

// static inline void normalized_vec3(float *const vec3)
// {
//     const float length = sqrtf(pow2f(vec3[0]) + pow2f(vec3[1]) + pow2f(vec3[2]));
//     assert(length > 1e-5);
//     int32_t i;
//     for (i = 0; i < 3; i++)
//     {
//         vec3[i] /= length;
//     }
//     return;
// }

// static inline bool ray_intersect_aabb(
//     const float *const position,
//     const float size,
//     const float *const start,
//     const float *const direction,
//     const float *const inv_direction,
//     float *const t_min,
//     float *const t_max)
// {
//     float tmin = 0.0;
//     float tmax = __FLT_MAX__;
//     int32_t i;
//     for (i = 0; i < 3; i++)
//     {
//         if (direction[i] == 0.0f)
//         {
//             if (start[i] < position[i] || start[i] > position[i] + size)
//                 return false;
//             continue;
//         }
//         const float distance = position[i] - start[i];
//         float t0 = distance * inv_direction[i];
//         float t1 = (distance + size) * inv_direction[i];
//         if (inv_direction[i] < 0.0f)
//         {
//             float tmp = t0;
//             t0 = t1;
//             t1 = tmp;
//         }
//         tmin = fmaxf(tmin, t0);
//         tmax = fminf(tmax, t1);
//         if (tmax < tmin)
//             return false;
//     }
//     *t_min = tmin;
//     *t_max = tmax;
//     return true;
// }

// static const uint8_t traversal_order[8][8] = {
//     // sign = (0,0,0)
//     {0, 1, 2, 3, 4, 5, 6, 7},
//     // sign = (0,0,1)
//     {1, 0, 3, 2, 5, 4, 7, 6},
//     // sign = (0,1,0)
//     {2, 3, 0, 1, 6, 7, 4, 5},
//     // sign = (0,1,1)
//     {3, 2, 1, 0, 7, 6, 5, 4},
//     // sign = (1,0,0)
//     {4, 5, 6, 7, 0, 1, 2, 3},
//     // sign = (1,0,1)
//     {5, 4, 7, 6, 1, 0, 3, 2},
//     // sign = (1,1,0)
//     {6, 7, 4, 5, 2, 3, 0, 1},
//     // sign = (1,1,1)
//     {7, 6, 5, 4, 3, 2, 1, 0}};

// ray_hit_t svo_ray_cast(const svo_t *const ot, const point_t start, const point_t end, float max_dist)
// {
//     ray_hit_t result = {
//         .distance = __FLT_MAX__,
//         .voxel = INVALID_VOXEL,
//         .hit = false};
//     float direction[3] = {
//         end.x - start.x,
//         end.y - start.y,
//         end.z - start.z};
//     normalized_vec3(direction);
//     const float inv_direction[3] = {
//         1.0f / direction[0],
//         1.0f / direction[1],
//         1.0f / direction[2]};
//     float t_min;
//     float t_max;
//     if (ray_intersect_aabb((float[3]){0.0f, 0.0f, 0.0f},
//                            ot->grid_size,
//                            (float[3]){start.x, start.y, start.z},
//                            direction,
//                            inv_direction,
//                            &t_min,
//                            &t_max) == false)
//         return result;
//     if (t_min > max_dist || t_max < 0.0f)
//         return result;
//     typedef struct stack_item_t
//     {
//         const svo_node_t *node;
//         point_t pos;
//         uint32_t size;
//         float t_min;
//     } stack_item_t;
//     stack_item_t stack[MAX_DEPTH * 8 + 4];
//     int32_t stack_size = 1;
//     stack[0] = (stack_item_t){
//         .node = ot->root,
//         .pos = POINT(0, 0, 0),
//         .size = ot->grid_size,
//         .t_min = t_min};
//     const int8_t sign_x = (direction[0] < 0.0f);
//     const int8_t sign_y = (direction[1] < 0.0f);
//     const int8_t sign_z = (direction[2] < 0.0f);
//     const int8_t sign_index = sign_x << 2 | sign_y << 1 | sign_z;
//     const uint8_t *order = traversal_order[sign_index];
//     while (stack_size > 0)
//     {
//         stack_size--;
//         stack_item_t item = stack[stack_size];
//         if (item.t_min >= result.distance)
//             continue;
//         const uint32_t node_type = item.node[0] & MASK_TYPE;
//         if (node_type == MASK_LEAF)
//         {
//             result.distance = item.t_min;
//             result.voxel.position = item.pos;
//             result.voxel.color = unpack_color(item.node[0]);
//             result.voxel.size = item.size;
//             result.hit = true;
//             continue;
//         }
//         else if (node_type == MASK_EMPTY)
//             continue;
//         const svo_node_t *children = ot->root + unpack_index(item.node[0]);
//         const uint32_t half = item.size / 2;
//         int8_t i;
//         for (i = 7; i >= 0; i--)
//         {
//             const int8_t octant = order[i];
//             const point_t child_pos = POINT(item.pos.x + (octant & 4 ? half : 0),
//                                             item.pos.y + (octant & 2 ? half : 0),
//                                             item.pos.z + (octant & 1 ? half : 0));
//             float t_child_min;
//             float t_child_max;
//             if (ray_intersect_aabb((float[3]){child_pos.x, child_pos.y, child_pos.z},
//                                    half,
//                                    (float[3]){start.x, start.y, start.z},
//                                    direction,
//                                    inv_direction,
//                                    &t_child_min,
//                                    &t_child_max) == true &&
//                 t_child_min <= max_dist && t_child_min < result.distance)
//             {
//                 stack[stack_size] = (stack_item_t){
//                     .node = children + octant,
//                     .pos = child_pos,
//                     .size = half,
//                     .t_min = t_child_min};
//                 stack_size++;
//             }
//         }
//     }
//     return result;
// }

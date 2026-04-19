#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef union point_t
{
    int32_t raw[3];
    struct
    {
        int32_t x, y, z;
    };
} point_t;

typedef union color_t
{
    uint8_t raw[3];
    struct
    {
        uint8_t r, g, b;
    };
} color_t;

typedef struct voxel_t
{
    point_t position;
    color_t color;
    uint32_t size;
} voxel_t;

typedef struct ray_hit_t
{
    float distance;
    voxel_t voxel;
    bool hit;
} ray_hit_t;

typedef uint32_t octree_node_t;

typedef struct octree_t
{
    octree_node_t *root;
    uint32_t *empty_list;
    uint32_t node_count;
    uint32_t capacity;
    uint32_t empty_count;
    uint32_t empty_capacity;
    const uint32_t grid_size;
    const uint32_t max_depth;
} octree_t;

#define POINT(X, Y, Z) \
    (point_t) { .x = X, .y = Y, .z = Z }
#define COLOR(R, G, B) \
    (color_t) { .r = R, .g = G, .b = B }
#define VOXEL(P, C, S) \
    (voxel_t) { .position = P, .color = C, .size = S }

octree_t octree(const uint32_t grid_size, const uint32_t min_size);
void octree_clear(octree_t *const ot);
void octree_free(octree_t *const ot);
voxel_t octree_get_voxel(octree_t *const ot, const point_t p);
void octree_set_voxel(octree_t *const ot, const point_t p, const color_t color);
void octree_unset_voxel(octree_t *const ot, const point_t p);
void octree_optimize(octree_t *const ot);
void octree_print(const octree_t *const ot);
ray_hit_t octree_ray_cast(const octree_t *const ot, const point_t start, const point_t end, const float max_dist);

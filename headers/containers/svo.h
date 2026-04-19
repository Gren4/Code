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
    uint8_t raw[4];
    struct
    {
        uint8_t r, g, b, a;
    };
} color_t;

typedef struct aabb_t
{
    point_t point;
    uint32_t offset;
} aabb_t;

typedef struct voxel_t
{
    aabb_t aabb;
    color_t color;
} voxel_t;

typedef struct ray_hit_t
{
    float distance;
    voxel_t voxel;
    bool hit;
} ray_hit_t;

typedef struct svo_queue_t
{
    uint32_t *queue;
    uint32_t count;
    uint32_t offset;
    uint32_t capacity;
} svo_queue_t;

typedef struct svo_nodes_t
{
    uint32_t *iroot;
    uint32_t *croot;
    uint32_t count;
    uint32_t capacity;
} svo_nodes_t;

typedef struct svo_t
{
    svo_nodes_t nodes;
    svo_queue_t spare;
    const uint32_t grid_size;
    const uint32_t max_depth;
} svo_t;

#define POINT(X, Y, Z) \
    (point_t) { .x = X, .y = Y, .z = Z }
#define COLOR(R, G, B, A) \
    (color_t) { .r = R, .g = G, .b = B, .a = A }
#define AABB(P, O) \
    (aabb_t) { .point = P, .offset = O }
#define VOXEL(A, C) \
    (voxel_t) { .aabb = A, .color = C }

svo_t svo(const uint32_t grid_size, const uint32_t min_size);
void svo_clear(svo_t *const svo);
void svo_free(svo_t *const svo);
void svo_adjust(svo_t *const svo);
voxel_t svo_get(svo_t *const svo, const point_t point);
void svo_set(svo_t *const svo, const point_t point, const color_t color);
void svo_unset(svo_t *const svo, const point_t point);
void svo_optimize(svo_t *const svo);
void svo_print(svo_t *const svo);
// ray_hit_t svo_ray_cast(const svo_t *const ot, const point_t start, const point_t end, const float max_dist);

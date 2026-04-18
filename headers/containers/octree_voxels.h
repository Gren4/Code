#pragma once
#include <stdint.h>

typedef union point_t
{
    int32_t data[3];
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

#define MASK_EMPTY /**/ 0x00000000U
#define MASK_LEAF /* */ 0x40000000U
#define MASK_NODE /* */ 0x80000000U

#define MASK_TYPE /* */ 0xC0000000U
#define MASK_INDEX /**/ 0x3FFFFFFFU
#define MASK_COLOR /**/ 0x00FFFFFFU

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

octree_t octree(const uint32_t grid_size, const uint32_t min_size);
void octree_free(octree_t *const ot);
void octree_set_voxel(octree_t *const ot, const point_t p, const color_t color);
void octree_optimize(octree_t *const ot);
void octree_print(const octree_t *const ot);

#pragma once
#include <stdint.h>

typedef union point_t
{
    int32_t data[3];
    struct { int32_t x, y, z; };
} point_t;

typedef struct octree_node_t
{
    uint8_t is_leaf : 1;
    union
    {
        uint8_t children_mask;
        uint8_t value;
    };
    uint32_t children_offset : 23;
} octree_node_t;

typedef struct octree_t
{
    octree_node_t *root;
    uint32_t grid_size;
    uint32_t min_size;
} octree_t;

octree_t octree(const uint32_t grid_size, const uint32_t min_size);
void octree_set_voxel(octree_t ot, const point_t p, const uint8_t value);
void octree_print(octree_t ot);

#pragma once
#include <stdint.h>

typedef union point_t
{
    int32_t data[3];
    struct { int32_t x, y, z; };
} point_t;

typedef struct octree_node_t
{
    uint32_t parent_offset;
    union { uint32_t child_offset[8], value; };
    uint8_t children_mask;
    uint8_t is_leaf : 1;
    uint8_t octant : 3;
} octree_node_t;

typedef struct octree_t
{
    octree_node_t *root;
    uint32_t node_count;
    uint32_t capacity;
    const uint32_t grid_size;
    const uint32_t min_size;
} octree_t;

octree_t octree(const uint32_t grid_size, const uint32_t min_size);
void octree_set_voxel(octree_t *const ot, const point_t p, const uint32_t value);
void octree_print(const octree_t *const ot);

#pragma once
#include <stdint.h>

typedef union point_t
{
    int32_t data[3];
    struct { int32_t x, y, z; };
} point_t;

typedef struct octree_node_t
{
    uint8_t children_status;
    uint8_t status;
} octree_node_t;

typedef struct octree_t
{
    octree_node_t *data;
    int32_t grid_size;
} octree_t;

octree_t octree(const int32_t grid_size);
void octree_set(octree_t *const ot, const point_t p);

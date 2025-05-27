#include <stdint.h>
#ifndef CGEMODEL
#define CGEMODEL
typedef struct 
{
    float x;
    float y;
    float z;
    float w;
} cgemodel_vec4;

typedef struct
{
    float u, v;
} cgemodel_uvpoint;

typedef struct
{
    cgemodel_vec4 v1, v2, v3;
    cgemodel_uvpoint uv1, uv2, uv3;
} cgemodel_triangle;

typedef struct
{
    cgemodel_triangle* triangles;
    uint16_t triangleCount;
    float rx, ry, rz;
    cgemodel_vec4 position;
    uint16_t* texture;
} cgemodel_object;
#endif
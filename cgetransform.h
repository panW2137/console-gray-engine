#include "cgemodel.h"
#include "cgebuffer.h"
#include <math.h>

#ifndef CGETRANSFORM
#define CGETRANSFORM

typedef struct
{
    cgemodel_vec4 position;
    float rx;
    float ry;
    float rz;
    float fov;
    int ortographic;
} cgetransform_camera;

cgemodel_vec4 cgetransform_rotate_vec(cgemodel_vec4 v, float rx, float ry, float rz);

cgemodel_vec4 cgetransform_project(cgemodel_vec4 v, float fov, uint16_t width, uint16_t height);

cgemodel_vec4 cgetransform_project_orth(cgemodel_vec4 v, float zoom, uint16_t width, uint16_t height);

cgemodel_triangle cgetransform_transform_triangle(cgemodel_triangle t, cgemodel_vec4 obj_pos, float rx, float ry, float rz, cgetransform_camera cam, cgebuffer_screen* scr);


#endif
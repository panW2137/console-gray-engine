#include "cgetransform.h"

#ifndef PI
#define PI 3.14f
#endif

//applies 3 rotation matricies to a vec4
cgemodel_vec4 cgetransform_rotate_vec(cgemodel_vec4 v, float rx, float ry, float rz) {
    float cosy = cosf(rx), siny = sinf(rx);
    float y1 = v.y * cosy - v.z * siny;
    float z1 = v.y * siny + v.z * cosy;

    v.y = y1;
    v.z = z1;

    float cosx = cosf(ry), sinx = sinf(ry);
    float x2 = v.x * cosx + v.z * sinx;
    float z2 = -v.x * sinx + v.z * cosx;

    v.x = x2;
    v.z = z2;

    float cosz = cosf(rz), sinz = sinf(rz);
    float x3 = v.x * cosz - v.y * sinz;
    float y3 = v.x * sinz + v.y * cosz;

    v.x = x3;
    v.y = y3;

    return v;
}

//applies perspective projection matrix to a vec4
cgemodel_vec4 cgetransform_project(cgemodel_vec4 v, float fov, uint16_t width, uint16_t height) {
    if (v.z == 0.0f) v.z = 0.01f;
    cgemodel_vec4 projected;

    float aspect = (float)width / (float)height;
    float tan_half_fov = tanf(fov / 2.0f); 

    projected.x = (v.x / (aspect * tan_half_fov * v.z)) * (width / 2) + width / 2;
    projected.y = (v.y / (tan_half_fov * v.z)) * (height / 2) + height / 2;

    projected.z = v.z;
    projected.w = 1.0f;
    return projected;
}

//aplpiise ortographic projection matrix to vec4
cgemodel_vec4 cgetransform_project_orth(cgemodel_vec4 v, float zoom, uint16_t width, uint16_t height) {
    cgemodel_vec4 projected;
    float aspect = (float)width / (float)height;

    projected.x = (v.x * zoom) * (width / 2.0f) + width / 2.0f;
    projected.y = (v.y * zoom) * (height / 2.0f) * aspect + height / 2.0f;

    projected.z = v.z;
    projected.w = 1.0f;
    return projected;
}

//applies transormation to an entire triangle
//takes camera angle into consideration
cgemodel_triangle cgetransform_transform_triangle(cgemodel_triangle t, cgemodel_vec4 obj_pos, float rx, float ry, float rz, cgetransform_camera cam, cgebuffer_screen* scr) {
    cgemodel_triangle out;

    cgemodel_vec4 verts[3] = { t.v1, t.v2, t.v3 };
    cgemodel_vec4* out_verts[3] = { &out.v1, &out.v2, &out.v3 };

    for (int i = 0; i < 3; ++i) {
        cgemodel_vec4 v = verts[i];

        v = cgetransform_rotate_vec(v, rx, ry, rz);
        v.x += obj_pos.x;
        v.y += obj_pos.y;
        v.z += obj_pos.z;

        v.x -= cam.position.x;
        v.y -= cam.position.y;
        v.z -= cam.position.z;

        v = cgetransform_rotate_vec(v, cam.rx, cam.ry, cam.rz);

        if(cam.ortographic){
            v = cgetransform_project_orth(v, cam.fov, scr->width, scr->height);
        } else {
            v = cgetransform_project(v, cam.fov, scr->width, scr->height);
        }
        
        *(out_verts[i]) = v;
    }

    return out;
}
#include "cgebuffer.h"
#include "cgemodel.h"
#include "cgetransform.h"
#include "cgedraw.h"

void swap_vec4(cgemodel_vec4* a, cgemodel_vec4* b) {
    cgemodel_vec4 temp = *a;
    *a = *b;
    *b = temp;
}

//draws line
//takes zbuffer into consideration
void cgedraw_draw_line(cgebuffer_screen* scr,int x0, int y0, float z0, int x1, int y1, float z1, uint8_t value) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    int steps = fmaxf(abs(x1 - x0), abs(y1 - y0));
    float dz = steps == 0 ? 0 : (z1 - z0) / steps;
    float z = z0;

    while (1) {
        cgebuffer_put_pixel(scr, x0, y0, z, value);
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }

        z += dz;
    }
}

//draws triangle
//probably takes zbuffer into consideration
//not tested
void cgedraw_draw_triangle(cgebuffer_screen* scr,cgemodel_triangle t, uint8_t brightness) {
    int x1 = (int)(t.v1.x);
    int y1 = (int)(t.v1.y);
    float z1 = t.v1.z;

    int x2 = (int)(t.v2.x);
    int y2 = (int)(t.v2.y);
    float z2 = t.v2.z;

    int x3 = (int)(t.v3.x);
    int y3 = (int)(t.v3.y);
    float z3 = t.v3.z;

    cgedraw_draw_line(scr, x1, y1, z1, x2, y2, z2, brightness);
    cgedraw_draw_line(scr, x2, y2, z2, x3, y3, z3, brightness);
    cgedraw_draw_line(scr, x3, y3, z3, x1, y1, z1, brightness);
}

float interpolate(float y1, float x1, float y2, float x2, float y) {
    if (y2 - y1 == 0.0f) return x1;
    return x1 + (x2 - x1) * ((y - y1) / (y2 - y1));
}

//utility function needed to draw filled triangles
void cgedraw_draw_scanline(cgebuffer_screen* scr,int y, int x_start, float z_start, int x_end, float z_end, uint8_t brightness) {
    if (y < 0 || y >= scr->height) return;

    if (x_start > x_end) {
        int tmp_x = x_start; x_start = x_end; x_end = tmp_x;
        float tmp_z = z_start; z_start = z_end; z_end = tmp_z;
    }

    float dz = (x_end - x_start) == 0 ? 0 : (z_end - z_start) / (x_end - x_start);

    for (int x = x_start; x <= x_end; x++) {
        float z = z_start + (x - x_start) * dz;
        cgebuffer_put_pixel(scr, x, y, z, brightness);
    }
}

//draws filled triangle
//takes zbuffer into consideration
void cgedraw_draw_filled_triangle(cgebuffer_screen* scr,cgemodel_triangle t, uint8_t brightness) {
    cgemodel_vec4 v1 = t.v1, v2 = t.v2, v3 = t.v3;

    if (v1.y > v2.y) swap_vec4(&v1, &v2);
    if (v1.y > v3.y) swap_vec4(&v1, &v3);
    if (v2.y > v3.y) swap_vec4(&v2, &v3);

    int y_start = (int)ceilf(v1.y);
    int y_end   = (int)floorf(v3.y);

    for (int y = y_start; y <= y_end; y++) {
        int is_bottom_half = y > v2.y || v2.y == v1.y;

        float xa = interpolate(v1.y, v1.x, v3.y, v3.x, y);
        float za = interpolate(v1.y, v1.z, v3.y, v3.z, y);

        float xb = is_bottom_half
                 ? interpolate(v2.y, v2.x, v3.y, v3.x, y)
                 : interpolate(v1.y, v1.x, v2.y, v2.x, y);

        float zb = is_bottom_half
                 ? interpolate(v2.y, v2.z, v3.y, v3.z, y)
                 : interpolate(v1.y, v1.z, v2.y, v2.z, y);

        cgedraw_draw_scanline(scr, y, (int)xa, za, (int)xb, zb, brightness);
    }
}

cgemodel_vec4 calculate_normal(cgemodel_triangle t) {
    cgemodel_vec4 u = {
        t.v2.x - t.v1.x,
        t.v2.y - t.v1.y,
        t.v2.z - t.v1.z,
        0
    };
    
    cgemodel_vec4 v = {
        t.v3.x - t.v1.x,
        t.v3.y - t.v1.y,
        t.v3.z - t.v1.z,
        0
    };

    cgemodel_vec4 normal = {
        u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x,
        0
    };

    float length = sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
    if (length > 0) {
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;
    }
    
    return normal;
}

//renders entire object
void cgedraw_render_object(cgebuffer_screen* scr,cgemodel_object obj, cgetransform_camera cam, uint8_t base_brightness, cgemodel_vec4 sun) {
    cgemodel_vec4 light_dir = sun;
    float light_len = sqrtf(light_dir.x * light_dir.x + light_dir.y * light_dir.y + light_dir.z * light_dir.z);
    if (light_len > 0) {
        light_dir.x /= light_len;
        light_dir.y /= light_len;
        light_dir.z /= light_len;
    }

    for (int i = 0; i < obj.triangleCount; ++i) {
        cgemodel_triangle projected = cgetransform_transform_triangle(
            obj.triangles[i],
            obj.position,
            obj.rx, obj.ry, obj.rz,
            cam, scr
        );

        //unprojected triangle needed to calculate light
        cgemodel_triangle transformed_no_camera;
        {
            cgemodel_vec4 verts[3] = { obj.triangles[i].v1, obj.triangles[i].v2, obj.triangles[i].v3 };
            cgemodel_vec4* out_verts[3] = { &transformed_no_camera.v1, &transformed_no_camera.v2, &transformed_no_camera.v3 };

            for (int j = 0; j < 3; ++j) {
                cgemodel_vec4 v = verts[j];
                v = cgetransform_rotate_vec(v, obj.rx, obj.ry, obj.rz);
                v.x += obj.position.x;
                v.y += obj.position.y;
                v.z += obj.position.z;
                *(out_verts[j]) = v;
            }
        }

        cgemodel_vec4 normal = calculate_normal(transformed_no_camera);

        float intensity =
            normal.x * light_dir.x +
            normal.y * light_dir.y +
            normal.z * light_dir.z;

        intensity = fmaxf(intensity, 0.1f);
        intensity = fminf(intensity, 1.0f);

        uint8_t brightness = (uint8_t)(base_brightness * intensity);

        cgedraw_draw_filled_triangle(scr, projected, brightness);
    }
}
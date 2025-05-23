#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>

//todo: fix this goddamn mess

#define WIDTH  230
#define HEIGHT 110

uint8_t buffer[WIDTH * HEIGHT];

uint8_t gray_to_ansi(uint8_t val) {
    return 232 + (val * 23 / 255);
}

typedef struct 
{
    float x;
    float y;
    float z;
    float w;
} vec4;

typedef struct
{
    vec4 position;
    float rx;
    float ry;
    float rz;
} camera;

typedef struct 
{
    vec4 v1, v2, v3;
} triangle;

typedef struct {
    triangle* triangles;
    int triangle_count;

    float rx, ry, rz;
    vec4 position;
} object;

void render_buffer() {
    for (int y = 0; y < HEIGHT - 1; y += 2) {
        for (int x = 0; x < WIDTH; ++x) {
            int top_index = y * WIDTH + x;
            int bottom_index = (y + 1) * WIDTH + x;

            uint8_t top = buffer[top_index];
            uint8_t bottom = buffer[bottom_index];

            uint8_t fg = gray_to_ansi(top);
            uint8_t bg = gray_to_ansi(bottom);

            printf("\033[38;5;%u;48;5;%um▀", fg, bg);
        }
        printf("\033[0m\n");
    }
}

void fill_gradient() {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {

            int diag = x + y;

            int max_diag = WIDTH + HEIGHT - 2;
            uint8_t value = (diag * 255) / max_diag;

            buffer[y * WIDTH + x] = value;
        }
    }
}

int put_pixel(int16_t x, int16_t y, uint8_t value) {
    if (x >= WIDTH || y >= HEIGHT) {
        return 1;
    }
    if (x < 0 || y < 0){
        return 1;
    }
    buffer[y * WIDTH + x] = value;
    return 0;
}

void init_buffer(){
	for(int i=0; i<WIDTH*HEIGHT; i++){
		buffer[i] = 0;
	}
}

void draw_line(int x0, int y0, int x1, int y1, uint8_t value) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy; 

    while (1) {
        put_pixel(x0, y0, value);

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
    }
}

void draw_triangle(triangle t, uint8_t brightness) {
    int x1 = (int)(t.v1.x);
    int y1 = (int)(t.v1.y);
    int x2 = (int)(t.v2.x);
    int y2 = (int)(t.v2.y);
    int x3 = (int)(t.v3.x);
    int y3 = (int)(t.v3.y);

    draw_line(x1, y1, x2, y2, brightness);
    draw_line(x2, y2, x3, y3, brightness);
    draw_line(x3, y3, x1, y1, brightness);
}

void swap_vec4(vec4* a, vec4* b) {
    vec4 temp = *a;
    *a = *b;
    *b = temp;
}

float interpolate(float y1, float x1, float y2, float x2, float y) {
    if (y2 - y1 == 0.0f) return x1;
    return x1 + (x2 - x1) * ((y - y1) / (y2 - y1));
}

void draw_scanline(int y, int x_start, int x_end, uint8_t brightness) {
    if (y < 0 || y >= HEIGHT) return;

    if (x_start > x_end) {
        int tmp = x_start;
        x_start = x_end;
        x_end = tmp;
    }

    for (int x = x_start; x <= x_end; x++) {
        put_pixel(x, y, brightness);
    }
}

void draw_filled_triangle(triangle t, uint8_t brightness) {
    vec4 v1 = t.v1, v2 = t.v2, v3 = t.v3;

    if (v1.y > v2.y) swap_vec4(&v1, &v2);
    if (v1.y > v3.y) swap_vec4(&v1, &v3);
    if (v2.y > v3.y) swap_vec4(&v2, &v3);

    int y_start = (int)ceilf(v1.y);
    int y_end   = (int)floorf(v3.y);

    for (int y = y_start; y <= y_end; y++) {
        int is_bottom_half = y > v2.y || v2.y == v1.y;

        float xa = interpolate(v1.y, v1.x, v3.y, v3.x, y);
        float xb = is_bottom_half
                 ? interpolate(v2.y, v2.x, v3.y, v3.x, y)
                 : interpolate(v1.y, v1.x, v2.y, v2.x, y);

        draw_scanline(y, (int)xa, (int)xb, brightness);
    }
}

vec4 rotate_vec(vec4 v, float rx, float ry, float rz) {
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

vec4 project(vec4 v) {
    if (v.z == 0.0f) v.z = 0.01f;
    vec4 projected;

    float aspect = (float)WIDTH / (float)HEIGHT;

    projected.x = (v.x / v.z) * WIDTH / 2 + WIDTH / 2;
    projected.y = (v.y / v.z) * HEIGHT / 2 * aspect + HEIGHT / 2;

    projected.z = v.z;
    projected.w = 1.0f;
    return projected;
}

triangle transform_triangle(triangle t, camera cam) {
    triangle out;

    vec4 cam_pos = cam.position;

    vec4 verts[3] = { t.v1, t.v2, t.v3 };
    vec4* out_verts[3] = { &out.v1, &out.v2, &out.v3 };

    for (int i = 0; i < 3; ++i) {
        vec4 v = verts[i];

        v.x -= cam_pos.x;
        v.y -= cam_pos.y;
        v.z -= cam_pos.z;

        v = rotate_vec(v, cam.rx, cam.ry, cam.rz);

        // rzutowanie
        v = project(v);

        *(out_verts[i]) = v;
    }

    return out;
}

triangle transform_single_triangle(triangle t, vec4 pos, float rx, float ry, float rz) {
    triangle out;

    vec4 verts[3] = { t.v1, t.v2, t.v3 };
    vec4* out_verts[3] = { &out.v1, &out.v2, &out.v3 };

    for (int i = 0; i < 3; ++i) {
        vec4 v = verts[i];

        v = rotate_vec(v, rx, ry, rz);
        v.x += pos.x;
        v.y += pos.y;
        v.z += pos.z;

        *(out_verts[i]) = v;
    }

    return out;
}

void render_object(object obj, camera cam, uint8_t brightness) {
    for (int i = 0; i < obj.triangle_count; ++i) {
        triangle transformed = transform_single_triangle(obj.triangles[i], obj.position, obj.rx, obj.ry, obj.rz);
        triangle projected = transform_triangle(transformed, cam);
        draw_triangle(projected, brightness);
    }
}

void clear_buffer(){
    for(int i=0; i<WIDTH*HEIGHT; i++){
        buffer[i] = 0;
    }
}

void clear_console() {
    printf("\033[2J\033[H");
}

int main() {

    init_buffer();

    triangle* cube_tris = malloc(sizeof(triangle) * 12);

    triangle cube_data[12] = {
        // front
        { { -5, -5,  5 }, {  5, -5,  5 }, {  5,  5,  5 } },
        { { -5, -5,  5 }, {  5,  5,  5 }, { -5,  5,  5 } },
        // back
        { { -5, -5, -5 }, {  5,  5, -5 }, {  5, -5, -5 } },
        { { -5, -5, -5 }, { -5,  5, -5 }, {  5,  5, -5 } },
        // left
        { { -5, -5, -5 }, { -5, -5,  5 }, { -5,  5,  5 } },
        { { -5, -5, -5 }, { -5,  5,  5 }, { -5,  5, -5 } },
        // right
        { { 5, -5, -5 }, { 5,  5,  5 }, { 5, -5,  5 } },
        { { 5, -5, -5 }, { 5,  5, -5 }, { 5,  5,  5 } },
        // top
        { { -5, 5, -5 }, { -5, 5,  5 }, {  5, 5,  5 } },
        { { -5, 5, -5 }, {  5, 5,  5 }, {  5, 5, -5 } },
        // bottom
        { { -5, -5, -5 }, {  5, -5,  5 }, { -5, -5,  5 } },
        { { -5, -5, -5 }, {  5, -5, -5 }, {  5, -5,  5 } }
    };

    for (int i = 0; i < 12; i++) {
        cube_tris[i] = cube_data[i];
    }

    object cube_obj = {
        .triangles = cube_tris,
        .triangle_count = 12,
        .rx = 0.5f,
        .ry = 0.7f,
        .rz = 0.0f,
        .position = {0.0f, 0.0f, 0.0f, 1.0f}
    };

    camera cam = {
        .position = {0, 0, -30},
        .rx = 0.0f,
        .ry = 0.0f,
        .rz = 0.0f
    };

    float cuberx = 0.0f;
    float cubery = 0.0f;
    float cuberz = 0.0f;

    while(1){
    clear_console();
    clear_buffer();

    cube_obj.rx = cuberx;
    cube_obj.ry = cubery;
    cube_obj.rz = cuberz;
    cuberx += 0.1f;
    cubery += 0.2f;
    cuberz += 0.1f;
    render_object(cube_obj, cam, 255);
    render_buffer();

    usleep(100000);
}

    free(cube_tris);
    return 0;
}

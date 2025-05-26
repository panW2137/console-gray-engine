#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>

#define WIDTH  230
#define HEIGHT 110

#define PI 3.14f

uint8_t buffer[WIDTH * HEIGHT];
float zbuffer[WIDTH * HEIGHT];
char strBuffer[(WIDTH * HEIGHT * 17) + (HEIGHT * 5) + 1];

//todo: base color should be held by triangles individually
//change in the far future

//todo: modify struct object so it can be chosen
//how to render it (filled triangles or wireframes)

//todo: modify projection matricies to account for FOV

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

vec4 global_sun = {2, 1, -1, 0};

//utility function
//takes unsigned 8-bit intiger and normalizes it
//to the range of 16 numbers
uint8_t gray_to_ansi(uint8_t val) {
    return 232 + (val * 23 / 255);
}

//function that renders buffer on the screen
void render_buffer() {
    int i = 0;
    for (int y = 0; y < HEIGHT - 1; y += 2) {
        for (int x = 0; x < WIDTH; ++x) {
            int top_index = y * WIDTH + x;
            int bottom_index = (y + 1) * WIDTH + x;

            uint8_t top = buffer[top_index];
            uint8_t bottom = buffer[bottom_index];

            uint8_t fg = gray_to_ansi(top);
            uint8_t bg = gray_to_ansi(bottom);

            i += snprintf(&strBuffer[i], sizeof(strBuffer) - i,
                          "\033[38;5;%u;48;5;%um▀", fg, bg);
        }

        i += snprintf(&strBuffer[i], sizeof(strBuffer) - i, "\033[0m\n");
    }

    strBuffer[i] = '\0';
    printf("%s", strBuffer);
}


//puts pixel on the screen
//takes zbuffer into consideration
int put_pixel_depth(int16_t x, int16_t y, float depth, uint8_t value) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return 1;

    int index = y * WIDTH + x;
    if (depth < zbuffer[index]) {
        zbuffer[index] = depth;
        buffer[index] = value;
    }

    return 0;
}

//both functions reset their respective buffers
//in future they may also perform other actions
void init_buffer(){
	for(int i=0; i<WIDTH*HEIGHT; i++){
		buffer[i] = 0;
	}
}
void init_z_buffer(){
    for(int i = 0; i < WIDTH * HEIGHT; i++){
        zbuffer[i] = INFINITY;
    }
}

//draws line
//takes zbuffer into consideration
void draw_line_z(int x0, int y0, float z0, int x1, int y1, float z1, uint8_t value) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    int steps = fmaxf(abs(x1 - x0), abs(y1 - y0));
    float dz = steps == 0 ? 0 : (z1 - z0) / steps;
    float z = z0;

    while (1) {
        put_pixel_depth(x0, y0, z, value);
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
void draw_triangle_z(triangle t, uint8_t brightness) {
    int x1 = (int)(t.v1.x);
    int y1 = (int)(t.v1.y);
    float z1 = t.v1.z;

    int x2 = (int)(t.v2.x);
    int y2 = (int)(t.v2.y);
    float z2 = t.v2.z;

    int x3 = (int)(t.v3.x);
    int y3 = (int)(t.v3.y);
    float z3 = t.v3.z;

    draw_line_z(x1, y1, z1, x2, y2, z2, brightness);
    draw_line_z(x2, y2, z2, x3, y3, z3, brightness);
    draw_line_z(x3, y3, z3, x1, y1, z1, brightness);
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

//utility function needed to draw filled triangles
void draw_scanline_z(int y, int x_start, float z_start, int x_end, float z_end, uint8_t brightness) {
    if (y < 0 || y >= HEIGHT) return;

    if (x_start > x_end) {
        int tmp_x = x_start; x_start = x_end; x_end = tmp_x;
        float tmp_z = z_start; z_start = z_end; z_end = tmp_z;
    }

    float dz = (x_end - x_start) == 0 ? 0 : (z_end - z_start) / (x_end - x_start);

    for (int x = x_start; x <= x_end; x++) {
        float z = z_start + (x - x_start) * dz;
        put_pixel_depth(x, y, z, brightness);
    }
}

//draws filled triangle
//takes zbuffer into consideration
void draw_filled_triangle_z(triangle t, uint8_t brightness) {
    vec4 v1 = t.v1, v2 = t.v2, v3 = t.v3;

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

        draw_scanline_z(y, (int)xa, za, (int)xb, zb, brightness);
    }
}

//applies 3 rotation matricies to a vec4
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

//applies perspective projection matrix to a vec4
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

//aplpiise ortographic projection matrix to vec4
vec4 project_orth(vec4 v) {
    vec4 projected;

    float aspect = (float)WIDTH / (float)HEIGHT;

    projected.x = v.x * (WIDTH / 2) + WIDTH / 2;
    projected.y = v.y * (HEIGHT / 2) * aspect + HEIGHT / 2;

    projected.z = v.z;
    projected.w = 1.0f;

    return projected;
}

//applies transormation to an entire triangle
//takes camera angle into consideration
triangle transform_triangle(triangle t, vec4 obj_pos, float rx, float ry, float rz, camera cam) {
    triangle out;

    vec4 verts[3] = { t.v1, t.v2, t.v3 };
    vec4* out_verts[3] = { &out.v1, &out.v2, &out.v3 };

    for (int i = 0; i < 3; ++i) {
        vec4 v = verts[i];

        v = rotate_vec(v, rx, ry, rz);
        v.x += obj_pos.x;
        v.y += obj_pos.y;
        v.z += obj_pos.z;

        v.x -= cam.position.x;
        v.y -= cam.position.y;
        v.z -= cam.position.z;

        v = rotate_vec(v, cam.rx, cam.ry, cam.rz);

        v = project(v);

        *(out_verts[i]) = v;
    }

    return out;
}

//calculates normal of a triangle
//important for lightning
vec4 calculate_normal(triangle t) {
    vec4 u = {
        t.v2.x - t.v1.x,
        t.v2.y - t.v1.y,
        t.v2.z - t.v1.z,
        0
    };
    
    vec4 v = {
        t.v3.x - t.v1.x,
        t.v3.y - t.v1.y,
        t.v3.z - t.v1.z,
        0
    };

    vec4 normal = {
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
void render_object(object obj, camera cam, uint8_t base_brightness) {
    vec4 light_dir = global_sun;
    float light_len = sqrtf(light_dir.x * light_dir.x + light_dir.y * light_dir.y + light_dir.z * light_dir.z);
    if (light_len > 0) {
        light_dir.x /= light_len;
        light_dir.y /= light_len;
        light_dir.z /= light_len;
    }

    for (int i = 0; i < obj.triangle_count; ++i) {
        triangle projected = transform_triangle(
            obj.triangles[i],
            obj.position,
            obj.rx, obj.ry, obj.rz,
            cam
        );

        //unprojected triangle needed to calculate light
        triangle transformed_no_camera;
        {
            vec4 verts[3] = { obj.triangles[i].v1, obj.triangles[i].v2, obj.triangles[i].v3 };
            vec4* out_verts[3] = { &transformed_no_camera.v1, &transformed_no_camera.v2, &transformed_no_camera.v3 };

            for (int j = 0; j < 3; ++j) {
                vec4 v = verts[j];
                v = rotate_vec(v, obj.rx, obj.ry, obj.rz);
                v.x += obj.position.x;
                v.y += obj.position.y;
                v.z += obj.position.z;
                *(out_verts[j]) = v;
            }
        }

        vec4 normal = calculate_normal(transformed_no_camera);

        float intensity =
            normal.x * light_dir.x +
            normal.y * light_dir.y +
            normal.z * light_dir.z;

        intensity = fmaxf(intensity, 0.1f);
        intensity = fminf(intensity, 1.0f);

        uint8_t brightness = (uint8_t)(base_brightness * intensity);

        draw_filled_triangle_z(projected, brightness);
    }
}

//clears buffer
void clear_buffer(){
    for(int i=0; i<WIDTH*HEIGHT; i++){
        buffer[i] = 0;
    }
}

//clears consoke
void clear_console() {
    printf("\033[2J\033[H");
}

//utility function needed to draw donut
vec4 compute_torus_vertex(float R, float r, float theta, float phi) {
    vec4 v;
    v.x = (R + r * cosf(phi)) * cosf(theta);
    v.y = (R + r * cosf(phi)) * sinf(theta);
    v.z = r * sinf(phi);
    v.w = 1.0f;
    return v;
}

//creates donut
object create_donut(float major_radius, float minor_radius, int steps_theta, int steps_phi) {
    int num_tris = 2 * steps_theta * steps_phi;
    triangle* tris = malloc(num_tris * sizeof(triangle));
    
    int idx = 0;
    for (int i = 0; i < steps_theta; i++) {
        float theta1 = i * 2 * PI / steps_theta;
        float theta2 = ((i + 1) % steps_theta) * 2 * PI / steps_theta;
        
        for (int j = 0; j < steps_phi; j++) {
            float phi1 = j * 2 * PI / steps_phi;
            float phi2 = ((j + 1) % steps_phi) * 2 * PI / steps_phi;
            
            vec4 v1 = compute_torus_vertex(major_radius, minor_radius, theta1, phi1);
            vec4 v2 = compute_torus_vertex(major_radius, minor_radius, theta2, phi1);
            vec4 v3 = compute_torus_vertex(major_radius, minor_radius, theta2, phi2);
            vec4 v4 = compute_torus_vertex(major_radius, minor_radius, theta1, phi2);
            
            // Pierwszy trójkąt
            tris[idx++] = (triangle){v1, v2, v3};
            // Drugi trójkąt
            tris[idx++] = (triangle){v1, v3, v4};
        }
    }
    
    return (object){
        .triangles = tris,
        .triangle_count = num_tris,
        .rx = 0, .ry = 0, .rz = 0,
        .position = {0,0,0,1}
    };
}


int main() {

    init_buffer();
    init_z_buffer();

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
        .rx = 0.0f,
        .ry = 0.0f,
        .rz = 0.0f,
        .position = {30, 0.0f, 15, 1.0f}
    };


    float cuberx = 0.0f;
    float cubery = 0.0f;
    float cuberz = 0.0f;


    object donut = create_donut(10.0f, 3.0f, 12, 8);
    camera cam = {
        .position = {0, 0, -30},
        .rx = 0, .ry = 0, .rz = 0
    };

    float cubedir = 1;

    while(1){
        clear_buffer();
        init_z_buffer();

        donut.rx = cuberx;
        donut.ry = cubery;
        donut.rz = cuberz;
        cube_obj.rx = cuberx*2;
        cube_obj.ry = cubery/2;
        cube_obj.rz = cuberz/2;
        cuberx += 0.1f/10;
        cubery += 0.1f/10;
        cuberz += 0.1f/10;

        if(cube_obj.position.x >= 30){
            cubedir = -1.0f/10;
        }
        if(cube_obj.position.x <= -30){
            cubedir = 1.0f/10;
        }
        cube_obj.position.x += cubedir;

        render_object(donut, cam, 200);
        render_object(cube_obj, cam, 200);

        clear_console();
        render_buffer();

        usleep(10000);
    }

    free(cube_tris);
    return 0;
}

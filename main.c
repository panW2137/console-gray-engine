#include "cgebuffer.h"
#include "cgemodel.h"
#include "cgetransform.h"
#include "cgedraw.h"
#include <stdio.h>
#include <unistd.h>

#define PI 3.14f

cgemodel_vec4 sun = {-1, 0, -1, 0};

cgetransform_camera cam1 = {
        .position = {0, 0, -35},
        .rx = 0, .ry = 0, .rz = 0,
        .fov = PI/3,
        .ortographic = 0
};
cgetransform_camera cam2 = {
        .position = {-45, -35, -30},
        .rx = PI/4, .ry = -PI/4, .rz = -PI/5,
        .fov = PI/  100,
        .ortographic = 1
};

//utility function needed to draw donut
cgemodel_vec4 compute_torus_vertex(float R, float r, float theta, float phi) {
    cgemodel_vec4 v;
    v.x = (R + r * cosf(phi)) * cosf(theta);
    v.y = (R + r * cosf(phi)) * sinf(theta);
    v.z = r * sinf(phi);
    v.w = 1.0f;
    return v;
}

//creates donut
cgemodel_object create_donut(float major_radius, float minor_radius, int steps_theta, int steps_phi) {
    int num_tris = 2 * steps_theta * steps_phi;
    cgemodel_triangle* tris = malloc(num_tris * sizeof(cgemodel_triangle));
    
    int idx = 0;
    for (int i = 0; i < steps_theta; i++) {
        float theta1 = i * 2 * PI / steps_theta;
        float theta2 = ((i + 1) % steps_theta) * 2 * PI / steps_theta;
        
        for (int j = 0; j < steps_phi; j++) {
            float phi1 = j * 2 * PI / steps_phi;
            float phi2 = ((j + 1) % steps_phi) * 2 * PI / steps_phi;
            
            cgemodel_vec4 v1 = compute_torus_vertex(major_radius, minor_radius, theta1, phi1);
            cgemodel_vec4 v2 = compute_torus_vertex(major_radius, minor_radius, theta2, phi1);
            cgemodel_vec4 v3 = compute_torus_vertex(major_radius, minor_radius, theta2, phi2);
            cgemodel_vec4 v4 = compute_torus_vertex(major_radius, minor_radius, theta1, phi2);
            
            // Pierwszy trójkąt
            tris[idx++] = (cgemodel_triangle){v1, v2, v3};
            // Drugi trójkąt
            tris[idx++] = (cgemodel_triangle){v1, v3, v4};
        }
    }
    
    return (cgemodel_object){
        .triangles = tris,
        .triangleCount = num_tris,
        .rx = 0, .ry = 0, .rz = 0,
        .position = {0,0,0,1}
    };
}

void clear_console() {
    printf("\033[2J\033[H");
}

int main(){
    cgemodel_triangle* cube_tris = malloc(sizeof(cgemodel_triangle) * 12);

    cgemodel_triangle cube_data[12] = {
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

    cgemodel_object cube_obj = {
        .triangles = cube_tris,
        .triangleCount = 12,
        .rx = 0.0f,
        .ry = 0.0f,
        .rz = 0.0f,
        .position = {30, 0.0f, 0, 1.0f}
    };

    cgemodel_object donut = create_donut(10, 3, 12, 8);
    cgebuffer_screen screen = cgebuffer_create_screen(230, 110);

    float rx, ry, rz = 0;

    float orbit_radius = 25.0f;
    float orbit_speed = 0.01f;
    float angle = 0.0f;

    cgetransform_camera workingCam = cam1;
    int count = 0;

    while(1){
        count++;
        if(count == 1000){
            workingCam = cam1;
        }
        if(count == 2000){
            workingCam = cam2;
            count = 0;
        }
        clear_console();
        rx += 0.01; ry += 0.01; rz += 0.01;
        donut.rx = rx;
        donut.ry = ry;
        donut.rz = rz;
        cube_obj.rx = -rx;
        cube_obj.rz = rz*2;

        angle += orbit_speed;
        cube_obj.position.x = orbit_radius * cosf(angle);
        cube_obj.position.z = orbit_radius * sinf(angle);

        cgebuffer_clear_buffer(&screen);
        cgebuffer_clear_zbuffer(&screen);

        cgedraw_render_object(&screen, donut, workingCam, 200, sun);
        cgedraw_render_object(&screen, cube_obj, workingCam, 200, sun);
        cgebuffer_render_buffer(&screen);
        usleep(1000000/100);
    }

    free(donut.triangles);
    free(screen.buffer);
    free(screen.zBuffer);

    return 0;
}
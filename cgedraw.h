#include "cgebuffer.h"
#include "cgemodel.h"
#include "cgetransform.h"
#include <stdlib.h>

#ifndef CGEDRAW
#define CGEDRAW

void cgedraw_draw_line(cgebuffer_screen* scr,int x0, int y0, float z0, int x1, int y1, float z1, uint8_t value);

void cgedraw_draw_triangle(cgebuffer_screen* scr,cgemodel_triangle t, uint8_t brightness);

void cgedraw_draw_scanline(cgebuffer_screen* scr,int y, int x_start, float z_start, int x_end, float z_end, uint8_t brightness);

void cgedraw_draw_filled_triangle(cgebuffer_screen* scr,cgemodel_triangle t, uint8_t brightness);

void cgedraw_render_object(cgebuffer_screen* scr,cgemodel_object obj, cgetransform_camera cam, uint8_t base_brightness, cgemodel_vec4 sun);

#endif
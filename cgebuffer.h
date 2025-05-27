#include <stdint.h>
#include <malloc.h> // Consider using <stdlib.h> for malloc/free for better portability
#include <float.h> // Add this for FLT_MAX

#ifndef CGEBUFFER
#define CGEBUFFER

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t* buffer; // Changed from uint16_t* to uint8_t*
    float* zBuffer;
} cgebuffer_screen;

void cgebuffer_render_buffer(cgebuffer_screen* screen);

cgebuffer_screen cgebuffer_create_screen(uint16_t width, uint16_t height);

int cgebuffer_put_pixel(cgebuffer_screen* screen,int16_t x, int16_t y, float depth, uint8_t value);

void cgebuffer_clear_buffer(cgebuffer_screen* scr); // Already defined, but good to ensure
void cgebuffer_clear_zbuffer(cgebuffer_screen* scr); // Already defined, but good to ensure

#endif
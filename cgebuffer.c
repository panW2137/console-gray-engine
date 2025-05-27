#include "cgebuffer.h"
#include <stdio.h> // For snprintf and printf

//utility function
//takes unsigned 8-bit intiger and normalizes it
//to the range of 16 numbers
uint8_t gray_to_ansi(uint8_t val) {
    return 232 + (val * 23 / 255);
}

//function that renders buffer on the screen
void cgebuffer_render_buffer(cgebuffer_screen* screen) {
    //create strBuffer
    char strBuffer[(screen->width * screen->height * 17) + (screen->height * 5) + 1];

    int i = 0;
    for (int y = 0; y < screen->height - 1; y += 2) {
        for (int x = 0; x < screen->width; ++x) {
            int top_index = y * screen->width + x;
            int bottom_index = (y + 1) * screen->width + x;

            uint8_t top = screen->buffer[top_index];
            uint8_t bottom = screen->buffer[bottom_index];

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

//creates new screen
cgebuffer_screen cgebuffer_create_screen(uint16_t width, uint16_t height){
    cgebuffer_screen new = {
        .width = width,
        .height = height,
        .buffer = malloc(width*height*sizeof(uint8_t)), // Changed from uint16_t to uint8_t
        .zBuffer = malloc(width*height*sizeof(float))
    };

    // Initialize zBuffer to a large value
    for(int i=0; i<width*height; i++){
        new.zBuffer[i] = FLT_MAX; // Initialize with max float value
    }
    
    return new;
}

int cgebuffer_put_pixel(cgebuffer_screen* screen,int16_t x, int16_t y, float depth, uint8_t value) {
    if (x < 0 || x >= screen->width || y < 0 || y >= screen->height) return 1;

    int index = y * screen->width + x;
    if (depth < screen->zBuffer[index]) { // This logic is correct for closer objects
        screen->zBuffer[index] = depth;
        screen->buffer[index] = value;
    }

    return 0;
}

void cgebuffer_clear_buffer(cgebuffer_screen* scr){
    for(int i=0; i<scr->width*scr->height; i++){
        scr->buffer[i] = 0;
    }
}
void cgebuffer_clear_zbuffer(cgebuffer_screen* scr){
    for(int i=0; i<scr->width*scr->height; i++){
        scr->zBuffer[i] = FLT_MAX; // Changed to FLT_MAX
    }
}
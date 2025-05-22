#include <stdio.h>
#include <stdint.h>
#include <malloc.h>

#define HALFBLOCK 223

#define WIDTH  110
#define HEIGHT 60

uint8_t buffer[WIDTH * HEIGHT];

uint8_t gray_to_ansi(uint8_t val) {
    return 232 + (val * 23 / 255);
}

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

int put_pixel(uint16_t x, uint16_t y, uint8_t value) {
    if (x >= WIDTH || y >= HEIGHT) {
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

int main() {
	init_buffer();
    fill_gradient();
	draw_line(WIDTH - 1, 0, 0, HEIGHT - 1, 255);
    render_buffer();
    return 0;
}

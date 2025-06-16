# Console Gray Engine (CGE)

**Console Gray Engine (CGE)** is a lightweight C-based framework for rendering 3D geometry directly in a console window.
Currently, the project supports **Linux terminals** only, but Windows support can likely be added with minimal effort.

---

## ✨ Features

* Normal-based lighting
* Perspective and orthographic projections
* Multiple camera support
* Object rotation

---

## 📁 Project Structure

The project currently has no central library file. Include the following headers directly in your project:

* `cgebuffer.h`
* `cgemodel.h`
* `cgetransform.h`
* `cgedraw.h`

---

## 🛠 Installation

Simply copy the files into your project folder and include them in your source code.

---

## 📦 File Descriptions

### `cgebuffer.h` — Buffer Management

Contains logic for creating, modifying, and rendering a screen buffer.

```c
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t* buffer;
    float* zBuffer;
} cgebuffer_screen;
```

This structure holds screen dimensions and pixel/z-buffer data.

```c
cgebuffer_screen cgebuffer_create_screen(uint16_t width, uint16_t height);
```

Initializes a new screen buffer.

```c
void cgebuffer_render_buffer(cgebuffer_screen* screen);
```

Renders the current buffer contents to the console.

```c
int cgebuffer_put_pixel(cgebuffer_screen* screen, int16_t x, int16_t y, float depth, uint8_t value);
```

Places a pixel at the specified position and depth.

```c
void cgebuffer_clear_buffer(cgebuffer_screen* scr);
void cgebuffer_clear_zbuffer(cgebuffer_screen* scr);
```

Clears the pixel and depth buffers, respectively.

---

### `cgemodel.h` — 3D Geometry Definitions

Defines structures for 3D objects and components.

```c
typedef struct {
    float x, y, z, w;
} cgemodel_vec4;
```

Represents a 4D vector used in geometry calculations.

```c
typedef struct {
    float u, v;
} cgemodel_uvpoint;
```

UV mapping point (currently unused).

```c
typedef struct {
    cgemodel_vec4 v1, v2, v3;
    cgemodel_uvpoint uv1, uv2, uv3;
} cgemodel_triangle;
```

Triangle composed of 3D vertices and UV points.

```c
typedef struct {
    cgemodel_triangle* triangles;
    uint16_t triangleCount;
    float rx, ry, rz;
    cgemodel_vec4 position;
    uint16_t* texture;
} cgemodel_object;
```

3D object with rotation, position, and geometry data. Texture support is currently unimplemented.

---

### `cgetransform.h` — Transformations and Camera

Implements essential 3D math operations and camera structures.

```c
typedef struct {
    cgemodel_vec4 position;
    float rx, ry, rz;
    float fov;
    int ortographic;
} cgetransform_camera;
```

Defines a camera with position, rotation, and projection settings.

```c
cgemodel_vec4 cgetransform_rotate_vec(cgemodel_vec4 v, float rx, float ry, float rz);
```

Rotates a vector around the origin.

```c
cgemodel_triangle cgetransform_transform_triangle(cgemodel_triangle t, cgemodel_vec4 obj_pos, float rx, float ry, float rz, cgetransform_camera cam, cgebuffer_screen* scr);
```

Transforms an entire triangle using object and camera data.

```c
cgemodel_vec4 cgetransform_project(cgemodel_vec4 v, float fov, uint16_t width, uint16_t height);
cgemodel_vec4 cgetransform_project_orth(cgemodel_vec4 v, float zoom, uint16_t width, uint16_t height);
```

Applies either perspective or orthographic projection to a vector.

---

### `cgedraw.h` — Drawing API

Handles geometry drawing into the screen buffer (not rendered directly to screen).

```c
void cgedraw_draw_line(cgebuffer_screen* scr, int x0, int y0, float z0, int x1, int y1, float z1, uint8_t value);
```

Draws a straight line between two 3D points. Z-buffering is implemented but not fully tested.

```c
void cgedraw_draw_triangle(cgebuffer_screen* scr, cgemodel_triangle t, uint8_t brightness);
```

Draws a wireframe triangle using lines.

```c
void cgedraw_draw_filled_triangle(cgebuffer_screen* scr, cgemodel_triangle t, uint8_t brightness);
```

Draws a filled triangle.

```c
void cgedraw_render_object(cgebuffer_screen* scr, cgemodel_object obj, cgetransform_camera cam, uint8_t base_brightness, cgemodel_vec4 sun);
```

Renders an entire 3D object with lighting based on a sun vector and triangle normals.

---

### 🧪 Example Usage

A basic usage example is provided in the file:

> **`main.c`** – contains sample code demonstrating how to use the engine components.

This example serves as a reference for how to set up a screen buffer, define geometry, transform it, and render it to the console.

you can compile and run it using the following command:
```
gcc main.c cgebuffer.c cgetransform.c cgedraw.c -o main.out -lm && ./main.out
```





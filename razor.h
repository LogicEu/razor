
/*  Copyright (c) 2023 Eugenio Arteaga A.

Permission is hereby granted, free of charge, to any 
person obtaining a copy of this software and associated 
documentation files (the "Software"), to deal in the 
Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to 
permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice 
shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#ifndef RAZOR_RASTERIZER_H
#define RAZOR_RASTERIZER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************
****** razor ******
*******************
@Eugenio Arteaga A.
******************/

#include <spxe.h>       /* Px       */
#include <fract.h>      /* vec3     */
#include <imgtool.h>    /* bmp_t    */

#define Z_NEAR 0.1
#define Z_FAR 1000.0

#define swap(A, B, Type) do { Type tmp = (A); (A) = (B); (B) = tmp; } while (0)
#define ulerp(A, B, t) (unsigned char)(int)_lerpf((float)(A), (float)(B), (t))
#define pxlerp(A, B, t) (Px){ulerp(A.r, B.r, t), ulerp(A.g, B.g, t), ulerp(A.b, B.b, t), ulerp(A.a, B.a, t)}

typedef struct ivec2 {
    int x, y;
} ivec2;

typedef struct RZframebuffer {
    bmp_t bitmap;
    float* zbuffer;
} RZframebuffer;

typedef struct RZvertex {
    vec3 pos;
    vec2 uv;
    vec3 normal;
} RZvertex;

typedef struct RZtriangle {
    RZvertex vertices[3];
} RZtriangle;

typedef struct RZfont {
    unsigned char* pixmap;
    ivec2 size;
    ivec2 bearing;
    unsigned int advance;
} RZfont;

void rzRasterize(RZframebuffer* framebuffer, const bmp_t* bmp, RZtriangle tri);

RZframebuffer rzFramebufferCreate(const bmp_t bmp);
void rzFramebufferFree(RZframebuffer* framebuffer);
void rzFramebufferClear(RZframebuffer* framebuffer);
void rzFramebufferClearColor(const Px color);

RZfont* rzFontLoad(const char* path, const unsigned int size);
void rzFontDrawChar(bmp_t* bmp, const RZfont font, const Px color, const ivec2 pos);
void rzFontDrawText(bmp_t* bmp, const RZfont* font, const char* text, const Px color, ivec2 pos);
void rzFontFree(RZfont* font);

#ifdef __cplusplus
}
#endif
#endif

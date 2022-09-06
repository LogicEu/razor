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

#include <utopia.h>
#include <fract.h>
#include <imgtool.h>
#include <spxe.h>

#define Z_NEAR 0.1
#define Z_FAR 1000.0

#define swap(A, B, Type) do { Type tmp = (A); (A) = (B); (B) = tmp; } while (0)
#define ulerp(A, B, t) (unsigned char)(int)_lerpf((float)(A), (float)(B), (t))
#define pxlerp(A, B, t) (Px){ulerp(A.r, B.r, t), ulerp(A.g, B.g, t), ulerp(A.b, B.b, t), ulerp(A.a, B.a, t)}

typedef struct RZfont {
    unsigned char* pixmap;
    ivec2 size;
    ivec2 bearing;
    unsigned int advance;
} RZfont;

typedef struct RZframebuffer {
    bmp_t bitmap;
    float* zbuffer;
} RZframebuffer;

typedef struct RZvertex {
    vec3 pos;
    vec2 uvs;
} RZvertex;

typedef struct RZtriangle {
    RZvertex vertices[3];
} RZtriangle;

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

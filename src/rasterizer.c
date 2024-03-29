#include <razor.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define swap(A, B, Type) do { Type tmp = (A); (A) = (B); (B) = tmp; } while (0)

static inline Px pxget(const bmp_t* bmp, unsigned int x, unsigned int y)
{
    return *((Px*)bmp->pixels + (y % bmp->height) * bmp->width + (x % bmp->width));
}

static inline Px rzTexmap(const bmp_t* bmp, const vec2 uv)
{
    return *(Px*)px_at(bmp, (int)(uv.x * (float)(bmp->width - 1)) % bmp->width, (int)((1.0 - uv.y) * (float)(bmp->height - 1)) % bmp->height);
}

static inline RZvertex rzVertexLerp(const RZvertex A, const RZvertex B, const int y)
{
    const float t = ilerpf((int)A.pos.y, (int)B.pos.y, y);
    return (RZvertex){vec3_lerp(A.pos, B.pos, t), vec2_lerp(A.uv, B.uv, t), vec3_lerp(A.normal, B.normal, t)};
}

static inline void rzVertexSort(RZvertex* restrict a, RZvertex* restrict b)
{
    if (a->pos.y < b->pos.y || (a->pos.y == b->pos.y && a->pos.x < b->pos.x)) {
        swap(*a, *b, RZvertex);
    }
}

static inline Px rzTexlerp(const bmp_t* bmp, const vec2 uv)
{
    Px xRange[2];
    vec2 p = {uv.x * (float)(bmp->width - 1), (1.0F - uv.y) * (float)(bmp->height - 1)};
    const ivec2 coord = {(int)p.x, (int)p.y};
    
    p = (vec2){p.x - (float)coord.x, p.y - (float)coord.y};
    for (int i = 0; i < 2; ++i) {
        Px x0 = pxget(bmp, coord.x + 0, coord.y + i);
        Px x1 = pxget(bmp, coord.x + 1, coord.y + i);
        xRange[i] = pxlerp(x0, x1, p.x);
    }

    return pxlerp(xRange[0], xRange[1], p.y);
}

void rzRasterScanline(RZframebuffer* framebuffer, const bmp_t* bmp, const int y, RZvertex p0, RZvertex p1)
{
    static const vec3 up = {0.0F, 1.0F, 0.0F};

    if (p0.pos.x > p1.pos.x) {
        swap(p0, p1, RZvertex);
    }

    const int startx = (int)p0.pos.x >= 0 ? (int)p0.pos.x : 0;
    const int endx = (int)p1.pos.x < (int)framebuffer->bitmap.width ? 
                    (int)p1.pos.x : (int)framebuffer->bitmap.width;
    
    for (int x = startx; x < endx; ++x) {
        float t = ilerpf((int)p0.pos.x, (int)p1.pos.x, x);
        float z = 1.0 / lerpf(p0.pos.z, p1.pos.z, t);
        if (z > Z_NEAR && z < Z_FAR && z < framebuffer->zbuffer[y * framebuffer->bitmap.width + x]) {
            framebuffer->zbuffer[y * framebuffer->bitmap.width + x] = z;
            vec2 uv = vec2_mult(vec2_lerp(p0.uv, p1.uv, t), z);
            vec3 n = vec3_normal(vec3_mult(vec3_lerp(p0.normal, p1.normal, t), z));
            Px col = rzTexlerp(bmp, uv);
            const float f = clampf((1.0F + _vec3_dot(n, up)) * 0.5F, 0.5F, 1.0F);
            col = (Px){(int)(f * (float)col.r), (int)(f * (float)col.g), (int)(f * (float)col.b), (int)(f * (float)col.a)};
            memcpy(framebuffer->bitmap.pixels + ((y * framebuffer->bitmap.width) + x) * sizeof(Px), &col, sizeof(Px));
        }
    }
}

void rzRasterWire(RZframebuffer* framebuffer, const bmp_t* bmp, const int y, RZvertex p0, RZvertex p1)
{
    static const vec3 up = {0.0F, 1.0F, 0.0F};

    if (p0.pos.x > p1.pos.x) {
        swap(p0, p1, RZvertex);
    }

    const int startx = (int)p0.pos.x >= 0 ? (int)p0.pos.x : 0;
    const int endx = (int)p1.pos.x < (int)framebuffer->bitmap.width ? 
                    (int)p1.pos.x : (int)framebuffer->bitmap.width;
    const int diff = endx - startx;
    
    for (int x = startx; x <= endx; x += diff + !diff) {
        float t = ilerpf((int)p0.pos.x, (int)p1.pos.x, x);
        float z = 1.0 / lerpf(p0.pos.z, p1.pos.z, t);
        vec2 uv = vec2_mult(vec2_lerp(p0.uv, p1.uv, t), z);
        vec3 n = vec3_normal(vec3_mult(vec3_lerp(p0.normal, p1.normal, t), z));
        Px col = rzTexlerp(bmp, uv);
        const float f = clampf((1.0F + _vec3_dot(n, up)) * 0.5F, 0.5F, 1.0F);
        col = (Px){(int)(f * (float)col.r), (int)(f * (float)col.g), (int)(f * (float)col.b), (int)(f * (float)col.a)};
        memcpy(framebuffer->bitmap.pixels + ((y * framebuffer->bitmap.width) + x) * sizeof(Px), &col, sizeof(Px));
    }
}

static void (*rzRasterFunc)(RZframebuffer*, const bmp_t*, const int, RZvertex, RZvertex) = &rzRasterScanline;

void rzRasterModeSwitch(void)
{
    if (rzRasterFunc == &rzRasterScanline) {
        rzRasterFunc = &rzRasterWire;
    } else {
        rzRasterFunc = &rzRasterScanline;
    }
}

void rzRasterize(RZframebuffer* framebuffer, const bmp_t* bmp, RZtriangle tri)
{
    rzVertexSort(tri.vertices + 1, tri.vertices + 0);
    rzVertexSort(tri.vertices + 2, tri.vertices + 0);
    rzVertexSort(tri.vertices + 2, tri.vertices + 1);

    if (tri.vertices[0].pos.y >= tri.vertices[2].pos.y) {
        return;
    }

    const int starty = (int)tri.vertices[0].pos.y >= 0 ? (int)tri.vertices[0].pos.y : 0;
    const int endy = (int)tri.vertices[2].pos.y < (int)framebuffer->bitmap.height ? (int)tri.vertices[2].pos.y : (int)framebuffer->bitmap.height;
    const int midy = (int)tri.vertices[1].pos.y;

    for (int y = starty, n = 0; y < endy; ++y) {
        n += !n && (y == midy || y < (int)tri.vertices[n].pos.y || y > (int)tri.vertices[n + 1].pos.y);

        RZvertex vLong = rzVertexLerp(tri.vertices[0], tri.vertices[2], y);
        RZvertex vShort = rzVertexLerp(tri.vertices[n], tri.vertices[n + 1], y);

        rzRasterFunc(framebuffer, bmp, y, vLong, vShort);
    }
}

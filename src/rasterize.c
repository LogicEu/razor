#include <razor.h>
#include <stdlib.h>
#include <string.h>

#define ULRP(a, b, t) (unsigned char)(int)_lerpf((float)(a), (float)(b), (t))
#define SWAP(a, b, type) do { type tmp = (a); (a) = (b); (b) = tmp; } while (0)

static struct Rasterizer {
    int width;
    int height;
    float* zbuffer;
} rasterizer = {200, 150, NULL};

static inline Px pxlerp(const Px a, const Px b, const float t)
{
    return (Px){ULRP(a.r, b.r, t), ULRP(a.g, b.g, t), ULRP(a.b, b.b, t), ULRP(a.a, b.a, t)};
}

static inline Px pxget(const bmp_t* bmp, unsigned int x, unsigned int y)
{
    if (x >= bmp->width) x = bmp->width - 1;
    if (y >= bmp->height) y = bmp->height - 1;
    return *((Px*)bmp->pixels + ((y * bmp->width) + x));
}

static inline void vsort(Vertex* a, Vertex* b)
{
    if (a->pos.y < b->pos.y || (a->pos.y == b->pos.y && a->pos.x < b->pos.x)) {
        SWAP(*a, *b, Vertex);
    }
}

static Px texmap(const bmp_t* bmp, const vec2 uv)
{
    Px xRange[2];
    vec2 p = {uv.x * (float)(bmp->width - 1), (1.0 - uv.y) * (float)(bmp->height - 1)};
    const ivec2 coord = {(int)p.x, (int)p.y};
    
    p = (vec2){p.x - (float)coord.x, p.y - (float)coord.y};
    for (int i = 0; i < 2; ++i) {
        Px x0 = pxget(bmp, coord.x + 0, coord.y + i);
        Px x1 = pxget(bmp, coord.x + 1, coord.y + i);
        xRange[i] = pxlerp(x0, x1, p.x);
    }

    return pxlerp(xRange[0], xRange[1], p.y);
}

static void scanline(Px* dest, const bmp_t* src, const int y, ivec2 xRange, vec2 zRange, vec2* uvRange)
{
    if (xRange.x > xRange.y) {
        SWAP(xRange.x, xRange.y, int);
        SWAP(zRange.x, zRange.y, float);
        SWAP(uvRange[0], uvRange[1], vec2);
    }

    const int endx = xRange.y < rasterizer.width ? xRange.y : rasterizer.width;
    const int startx = xRange.x >= 0 ? xRange.x : 0;
    
    for (int x = startx; x < endx; ++x) {
        float t = clampf(ilerpf(xRange.x, xRange.y, x), 0.0, 1.0);
        float z = 1.0 / lerpf(zRange.x, zRange.y, t);
        if (z > Z_NEAR && z < Z_FAR && z < rasterizer.zbuffer[y * rasterizer.width + x]) {
            rasterizer.zbuffer[y * rasterizer.width + x] = z;
            vec2 uv = vec2_mult(vec2_lerp(uvRange[0], uvRange[1], t), z);
            Px col = texmap(src, uv);
            memcpy(dest + ((y * rasterizer.width) + x), &col, sizeof(Px));
        }
    }
}

void rasterize(Px* dest, const bmp_t* src, Vertex* p)
{
    vsort(p + 1, p + 0);
    vsort(p + 2, p + 0);
    vsort(p + 2, p + 1);

    const float x0 = p[0].pos.x, y0 = p[0].pos.y;
    const float x2 = p[2].pos.x, y2 = p[2].pos.y;

    if (y0 == y2) {
        return;
    }

    const int starty = (int)y0 >= 0 ? (int)y0 : 0;
    const int endy = (int)y2 < rasterizer.height ? (int)y2 : rasterizer.height;
    const int midy = (int)p[1].pos.y;

    int n = 0;
    for (int y = starty; y < endy; ++y) {
        
        n += (y == midy);
        
        const float tLong = clampf(ilerpf(y0, y2, y), 0.0, 1.0);
        const float tShort = clampf(ilerpf(p[n].pos.y, p[n + 1].pos.y, y), 0.0, 1.0);

        ivec2 xRange = {
            (int)lerpf(x0, x2, tLong),
            (int)lerpf(p[n].pos.x, p[n + 1].pos.x, tShort)
        };

        vec2 zRange = {
            lerpf(p[0].pos.z, p[2].pos.z, tLong),
            lerpf(p[n].pos.z, p[n + 1].pos.z, tShort)
        };

        vec2 uvRange[] = {{
            lerpf(p[0].uvs.x, p[2].uvs.x, tLong),
            lerpf(p[0].uvs.y, p[2].uvs.y, tLong),
        }, {
            lerpf(p[n].uvs.x, p[n + 1].uvs.x, tShort),
            lerpf(p[n].uvs.y, p[n + 1].uvs.y, tShort),
        }};
        
        scanline(dest, src, y, xRange, zRange, uvRange);
    }
}

void rasterinit(const int width, const int height)
{
    rasterizer.width = width;
    rasterizer.height = height;
    rasterizer.zbuffer = malloc(width * height * sizeof(float));
}

void rasterclear(void)
{
    memset(rasterizer.zbuffer, 100, rasterizer.width * rasterizer.height * sizeof(float));
}

void rasterdeinit(void)
{
    free(rasterizer.zbuffer);
}

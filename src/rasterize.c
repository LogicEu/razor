#include <razor.h>

#define SWAP(a, b, type) do { type tmp = (a); (a) = (b); (b) = tmp; } while (0)

static inline void vsort(Vertex* a, Vertex* b)
{
    if (a->pos.y < b->pos.y || (a->pos.y == b->pos.y && a->pos.x < b->pos.x)) {
        SWAP(*a, *b, Vertex);
    }
}

static inline unsigned char ucharlerp(unsigned char a, unsigned char b, float t)
{
    return (unsigned char)(int)lerpf((float)a, (float)b, t);
}

static inline Px pxlerp(Px a, Px b, float t)
{
    return (Px){
        ucharlerp(a.r, b.r, t),
        ucharlerp(a.g, b.g, t),
        ucharlerp(a.b, b.b, t),
        ucharlerp(a.a, b.a, t),
    };
}

static inline Px pxget(const unsigned char* px)
{
    return (Px){px[0], px[1], px[2], px[3]};
}

static inline Px bmp_lerp(const bmp_t* bmp, const vec2 uv)
{
    vec2 xy = {uv.x * (float)(bmp->width - 1), (1.0 - uv.y) * (float)(bmp->height - 1)};
    ivec2 ixy = {(int)xy.x, (int)xy.y};
    vec2 dif = {xy.x - (float)ixy.x, xy.y - (float)ixy.y};

    Px pxfrom1 = pxget(px_at(bmp, ixy.x, ixy.y));
    Px pxto1 = pxget(px_at(bmp, ixy.x + 1, ixy.y));
    Px lerp1 = pxlerp(pxfrom1, pxto1, dif.x);

    Px pxfrom2 = pxget(px_at(bmp, ixy.x, ixy.y + 1));
    Px pxto2 = pxget(px_at(bmp, ixy.x + 1, ixy.y + 1));
    Px lerp2 = pxlerp(pxfrom2, pxto2, dif.x);

    return pxlerp(lerp1, lerp2, dif.y);
}

static void scanline(Px* dest, const bmp_t* src, const int y, ivec2 xRange, vec2* uvRange)
{
    if (xRange.x > xRange.y) {
        SWAP(xRange.x, xRange.y, int);
        SWAP(uvRange[0], uvRange[1], vec2);
    }

    const int endx = xRange.y;
    for (int x = xRange.x; x < endx; ++x) {
        float t = ilerpf(xRange.x, xRange.y, x);
        vec2 uv = vec2_lerp(uvRange[0], uvRange[1], t);
        Px col = bmp_lerp(src, uv);
        plot(dest, col, x, y);
    }
}

void rasterize(Px* dest, const bmp_t* src, Vertex* p)
{
    vsort(p + 1, p + 0);
    vsort(p + 2, p + 0);
    vsort(p + 2, p + 1);

    const float x0 = p[0].pos.x, y0 = p[0].pos.y;
    const float x1 = p[1].pos.x, y1 = p[1].pos.y;
    const float x2 = p[2].pos.x, y2 = p[2].pos.y;

    if (y0 == y2) {
        return;
    }

    int n = 0;
    const int endy = (int)y2, midy = (int)y1;
    for (int y = y0; y < endy; ++y) {
        
        n += (y == midy);
        
        const float tLong = ilerpf(y0, y2, y);
        const float tShort = ilerpf(p[n].pos.y, p[n + 1].pos.y, y);

        ivec2 xRange = {
            (int)lerpf(x0, x2, tLong),
            (int)lerpf(p[n].pos.x, p[n + 1].pos.x, tShort)
        };

        vec2 uvRange[] = {{
            lerpf(p[0].uvs.x, p[2].uvs.x, tLong),
            lerpf(p[0].uvs.y, p[2].uvs.y, tLong),
        }, {
            lerpf(p[n].uvs.x, p[n + 1].uvs.x, tShort),
            lerpf(p[n].uvs.y, p[n + 1].uvs.y, tShort),
        }};
        
        scanline(dest, src, y, xRange, uvRange);
    }
}

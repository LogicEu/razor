#include <razor.h>
#include <stdlib.h>
#include <string.h>

static Px clearColor = {0, 0, 0, 255};

RZframebuffer rzFramebufferCreate(const bmp_t bmp)
{
    RZframebuffer framebuffer;
    const size_t size = sizeof(float) * bmp.width * bmp.height;
    framebuffer.bitmap = bmp;
    framebuffer.zbuffer = malloc(size);
    memset(framebuffer.zbuffer, 100, size);
    return framebuffer;
}

void rzFramebufferClear(RZframebuffer* framebuffer)
{
    const int endx = framebuffer->bitmap.width, endy = framebuffer->bitmap.height;
    Px* pixbuf = (Px*)framebuffer->bitmap.pixels;
    const Px color = clearColor;
    
    for (int x = 0; x < endx; ++x) {
        pixbuf[x] = color;
    }
    
    for (int y = 1; y < endy; ++y) {
        memcpy(pixbuf + y * endx, pixbuf, endx * sizeof(Px));
    }

    memset(framebuffer->zbuffer, 100, sizeof(float) * endx * endy);
}

void rzFramebufferClearColor(const Px color)
{
    clearColor = color;
}

void rzFramebufferFree(RZframebuffer* framebuffer)
{
    bmp_free(&framebuffer->bitmap);
    free(framebuffer->zbuffer);
}

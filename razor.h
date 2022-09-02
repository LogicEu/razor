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

typedef struct Vertex {
    vec2 pos;
    vec2 uvs;
} Vertex;

void plot(Px* dest, const Px src, const int x, const int y);
void rasterize(Px* dest, const bmp_t* src, Vertex* p);

#ifdef __cplusplus
}
#endif
#endif

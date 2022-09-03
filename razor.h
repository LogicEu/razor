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

typedef struct Vertex {
    vec3 pos;
    vec2 uvs;
} Vertex;

void rasterize(Px* dest, const bmp_t* src, Vertex* p);
void rasterinit(const int width, const int height);
void rasterdeinit(void);
void rasterclear(void);

#ifdef __cplusplus
}
#endif
#endif

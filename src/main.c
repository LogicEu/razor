#define SPXE_APPLICATION
#include <razor.h>
#include <stdlib.h>
#include <stdio.h>

#define SENSIBILITY (-0.001)
#define HALF_PI (M_PI * 0.5)
#define FOV_DEGREES (45.0)
#define FOV (FOV_DEGREES / (180.0 / M_PI))

typedef struct Tri {
    Vertex vertices[3];
} Tri;

static inline vec4 vec4_fill(const vec3 v, const float f)
{
    return (vec4){v.x, v.y, v.z, f};
}

static Tri project(const Vertex* vertices, const mat4* mvp)
{
    Tri t;
    for (int i = 0; i < 3; ++i) {
        vec4 p = vec4_mult_mat4(vec4_fill(vertices[i].pos, 1.0), *mvp);
        p = (vec4){p.x / p.w, p.y / p.w, p.z, p.w};
        t.vertices[i].pos.x = (p.x * (float)spxe.scrres.width) / (2.0f * p.w) + spxe.scrres.width * 0.5;
        t.vertices[i].pos.y = (p.y * (float)spxe.scrres.height) / (2.0f * p.w) + spxe.scrres.height * 0.5;
        t.vertices[i].pos.z = 1.0 / p.w;
        t.vertices[i].uvs.x = vertices[i].uvs.x * t.vertices[i].pos.z;
        t.vertices[i].uvs.y = vertices[i].uvs.y * t.vertices[i].pos.z;
    }
    return t;
}

int main(const int argc, char** argv)
{
    int width = 200, height = 150, mousex, mousey;
    for (int i = 1; i < argc - 1; ++i) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'w') {
                width = atoi(argv[++i]);
            }
            else if (argv[i][1] == 'h') {
                height = atoi(argv[++i]);
            }
        }
    }

    //bmp_t bmp = bmp_load("image.png");
    bmp_t bmp = bmp_new(2, 2, 4);
    bmp.pixels[0] = 255, bmp.pixels[1] = 0, bmp.pixels[2] = 0, bmp.pixels[3] = 255;
    bmp.pixels[4] = 0, bmp.pixels[5] = 255, bmp.pixels[6] = 0, bmp.pixels[7] = 255;
    bmp.pixels[8] = 0, bmp.pixels[9] = 0, bmp.pixels[10] = 255, bmp.pixels[11] = 255;
    bmp.pixels[12] = 255, bmp.pixels[13] = 255, bmp.pixels[14] = 255,
    bmp.pixels[15] = 255;

    Px* pixbuf = spxeStart("razor", 800, 600, width, height);
    if (!pixbuf) {
        printf("razor could not initiate spxe pixel engine.\n");
        return EXIT_FAILURE;
    }

    rasterinit(width, height);

    const float halfWidth = (float)width * 0.5;
    const float halfHeight = (float)height * 0.5;
    const float aspect = (float)width / (float)height;
    const mat4 proj = mat4_perspective(FOV, aspect, Z_NEAR, Z_FAR);
    
    vec3 dir, right, up, pos = {40.0, 40.0, 20.0};
    mat4 view, mvp;
    vec2 mouse;

    Vertex tri[] = {
        {{10.0, 10.0, 0.0}, {0.0, 0.0}},
        {{10.0, 100.0, 0.0}, {0.0, 1.0}},
        {{100.0, 100.0, 0.0}, {1.0, 1.0}},
        {{10.0, 10.0, 0.0}, {0.0, 0.0}},
        {{100.0, 100.0, 0.0}, {1.0, 1.0}},
        {{100.0, 10.0, 0.0}, {1.0, 0.0}}
    };

    float T = spxeTime();
    while (spxeRun(pixbuf)) {
        float t = spxeTime();
        float dT = T - t;
        T = t;

        const float f = dT * 50.0;

        if (spxeKeyPressed(ESCAPE)) {
            break;
        }
        if (spxeKeyDown(P)) {
            printf("Pos: %f, %f, %f\nDir: %f, %f, %f\n", pos.x, pos.y, pos.z, dir.x, dir.y, dir.z);
        }

        if (spxeKeyDown(W)) {
            pos = vec3_add(pos, vec3_mult(dir, f));
        }
        if (spxeKeyDown(S)) {
            pos = vec3_sub(pos, vec3_mult(dir, f));
        }
        if (spxeKeyDown(D)) {
            pos = vec3_add(pos, vec3_mult(right, f));
        }
        if (spxeKeyDown(A)) {
            pos = vec3_sub(pos, vec3_mult(right, f));
        }
        if (spxeKeyDown(Z)) {
            pos = vec3_add(pos, vec3_mult(up, f));
        }
        if (spxeKeyDown(X)) {
            pos = vec3_sub(pos, vec3_mult(up, f));
        }

        spxeMousePos(&mousex, &mousey);
        mouse = (vec2){((float)mousex - halfWidth) * SENSIBILITY, ((float)mousey - halfHeight) * SENSIBILITY};

        dir = (vec3){-cosf(mouse.y) * sinf(mouse.x), sinf(mouse.y), cosf(mouse.y) * cosf(mouse.x)};
        right = (vec3){-sinf(mouse.x - HALF_PI), 0.0, cosf(mouse.x - HALF_PI)};
        up = vec3_cross(dir, right);
        
        view = mat4_look_at(pos, vec3_add(pos, dir), up);
        mvp = mat4_mult(proj, view);

        memset(pixbuf, 155, width * height * sizeof(Px));
        rasterclear();

        for (int i = 0; i < 2; ++i) {
            Tri t = project(tri + i * 3, &mvp);
            rasterize(pixbuf, &bmp, t.vertices);
        }
    }

    bmp_free(&bmp);
    rasterdeinit();
    return spxeEnd(pixbuf);
}

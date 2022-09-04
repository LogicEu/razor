#define SPXE_APPLICATION
#include <razor.h>
#include <stdlib.h>
#include <stdio.h>
#include <mass.h>

#define SENSIBILITY (-0.01)
#define HALF_PI (M_PI * 0.5)
#define FOV_DEGREES (25.0)
#define FOV (FOV_DEGREES / (180.0 / M_PI))

typedef struct Tri {
    Vertex vertices[3];
} Tri;

static inline vec4 vec4_fill(const vec3 v, const float f)
{
    return (vec4){v.x, v.y, v.z, f};
}

static Tri project(const mat4* mvp, const vec3* pos, const vec2* uv)
{
    Tri t;
    for (int i = 0; i < 3; ++i) {
        vec4 p = vec4_mult_mat4(vec4_fill(pos[i], 1.0), *mvp);
        p = (vec4){p.x / p.w, p.y / p.w, p.z, p.w};
        t.vertices[i].pos.x = 2.5 * (p.x * (float)spxe.scrres.width) / p.w + spxe.scrres.width * 0.5;
        t.vertices[i].pos.y = 2.5 * (p.y * (float)spxe.scrres.height) / p.w + spxe.scrres.height * 0.5;
        t.vertices[i].pos.z = 1.0 / p.z;
        t.vertices[i].uvs.x = uv[i].x * t.vertices[i].pos.z;
        t.vertices[i].uvs.y = uv[i].y * t.vertices[i].pos.z;
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

    const char* fileimg = "assets/image.png";
    bmp_t bmp = bmp_load(fileimg);
    if (!bmp.pixels) {
        printf("razor could not open image file '%s'.\n", fileimg);
        return EXIT_SUCCESS;
    }

    const char* fileobj = "assets/voxel.obj";
    Mesh3D mesh = mesh3D_load(fileobj);
    if (!mesh.vertices.size) {
        printf("razor could not open 3D model from file '%s'.\n", fileobj);
        bmp_free(&bmp);
        return EXIT_FAILURE;
    }

    Px* pixbuf = spxeStart("razor", 800, 600, width, height);
    if (!pixbuf) {
        printf("razor could not initiate spxe pixel engine.\n");
        bmp_free(&bmp);
        mesh3D_free(&mesh);
        return EXIT_FAILURE;
    }
    
    spxeMouseVisible(0);
    rasterinit(width, height);

    float angle = 0.0;
    const float halfWidth = (float)width * 0.5;
    const float halfHeight = (float)height * 0.5;
    const float aspect = (float)width / (float)height;
    const mat4 proj = mat4_perspective(FOV, aspect, Z_NEAR, Z_FAR);
    
    vec3 dir, right, up, pos = {0.0, 0.0, 4.0};
    mat4 view, mvp, scale;
    vec2 mouse;

    const size_t tricount =  mesh.vertices.size / 3;
    const vec3* vertices = mesh.vertices.data;
    const vec2* uvs = mesh.uvs.data;

    float T = spxeTime();
    while (spxeRun(pixbuf)) {
        float t = spxeTime();
        float dT = T - t;
        T = t;

        const float f = dT * 10.0;
        if (spxeKeyDown(M)) {
            angle += dT;
        }

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

        dir = vec3_normal((vec3){-cosf(mouse.y) * sinf(mouse.x), sinf(mouse.y), cosf(mouse.y) * cosf(mouse.x)});
        right = vec3_normal((vec3){-sinf(mouse.x - HALF_PI), 0.0, cosf(mouse.x - HALF_PI)});
        up = vec3_cross(dir, right);
        
        scale = mat4_model(vec3_uni(0.0), vec3_uni(1.0), (vec3){0.0, 1.0, 0.0}, angle);
        view = mat4_look_at(pos, vec3_add(pos, dir), up);
        mvp = mat4_mult(proj, mat4_mult(view, scale));

        memset(pixbuf, 155, width * height * sizeof(Px));
        rasterclear();

        for (size_t i = 0; i < tricount; ++i) {
            Tri t = project(&mvp, vertices + i * 3, uvs + i * 3);
            rasterize(pixbuf, &bmp, t.vertices);
        }
    }
    rasterdeinit();

    mesh3D_free(&mesh);
    bmp_free(&bmp);
    return spxeEnd(pixbuf);
}

#define SPXE_APPLICATION
#include <razor.h>
#include <utopia/hash.h>
#include <stdlib.h>
#include <stdio.h>

#define TIMEOUT_TIME (100)
#define SENSIBILITY (0.01F)
#define HALF_PI (M_PI * 0.5F)
#define FOV_DEGREES (4.0F)
#define FOV (FOV_DEGREES / (180.0F / M_PI))
#define FLARGE 1000000.0F
#define HCOLOR_COUNT 8

static bmp_t bmp_height_colors(void)
{
    uint8_t colors[HCOLOR_COUNT][4] = {
        {0, 0, 255, 255},       // Blue
        {100, 100, 255, 255},   // Light blue
        {0, 255, 0, 255},       // Light green
        {0, 155, 0, 255},       // Green
        {205, 105, 25, 255},    // Light brown
        {150, 75, 0, 255},      // Brown
        {75, 75, 105, 255},     // Grey
        {235, 235, 255, 255}    // White && Light blue
    };

    bmp_t bmp = bmp_new(HCOLOR_COUNT, 1, sizeof(Px));
    for (unsigned int i = 0; i < HCOLOR_COUNT; ++i) {
        memcpy(px_at(&bmp, i, 0), colors[i], sizeof(Px));
    }
    
    return bmp;
}

static size_t hash_nohash(const void* key)
{
    return *(size_t*)key;
}

static void mesh3D_normalize_smooth(Mesh3D* mesh)
{
    const size_t count = mesh->vertices.size;
    const vec3* vertices = mesh->vertices.data;
    vec3* normals = mesh->normals.data;

    struct hash found = hash_reserve(sizeof(size_t), count);
    hash_overload(&found, &hash_nohash);

    for (size_t i = 0; i < count; ++i) {
        if (hash_search(&found, &i)) {
            continue;
        }

        const size_t found_count = found.size;
        hash_push(&found, &i);
        vec3 normal = normals[i];

        for (size_t j = 0; j < count; ++j) {
            if (i != j && !memcmp(vertices + i, vertices + j, sizeof(vec3))) {
                hash_push(&found, &j);
                normal = vec3_add(normal, normals[j]);
            }
        }

        const size_t* indices = found.data;
        normal = vec3_normal(normal);
        for (size_t j = found_count; j < found.size; ++j) {
            normals[indices[j]] = normal;
        }
    }

    hash_free(&found);
}

static Mesh3D mesh3D_perlin(const size_t size)
{
    size_t i = 0;
    Mesh3D mesh = mesh3D_shape_plane(size, size);
    vec3* vertices = mesh.vertices.data;
    const size_t count = mesh.vertices.size;
    float maxy = -FLARGE, miny = FLARGE;
    for (size_t i = 0; i < count; ++i) {
        float h = perlin2d(vertices[i].x, vertices[i].z, 0.2f, 2, 0) * 16.0f;
        vertices[i].y = h;
        if (h > maxy)
            maxy = h;
        if (h < miny) 
            miny = h;
    }

    for (i = 0; i < count; ++i) {
        vec2 uv = {
            ilerpf(miny, maxy, vertices[i].y), 
            0.0f
        };
        vector_push(&mesh.uvs, &uv);
    }

    const float offset = (float)size * 0.5f;
    mesh3D_move(&mesh, vec3_new(-offset, -maxy, -offset));
    mesh3D_normalize_faces(&mesh);
    mesh3D_normalize_smooth(&mesh);
    return mesh;
}

static RZmodel rzModelPerlin(bmp_t* texture, const size_t size)
{
    RZmodel model;
    model.mesh = mesh3D_perlin(size);
    model.texture = texture;
    return model;
}

int main(const int argc, char** argv)
{
    const char* imgpath = "assets/textures/crate.png";
    const char* fontpath = "assets/fonts/Pixeled.ttf";
    const char* modelpath = NULL;
    
    char uibuf[0xff] = "Razor:";
    int mousex, mousey, width = 400, height = 300, uitimer = TIMEOUT_TIME;  

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'w') {
                width = atoi(argv[++i]);
            } else if (argv[i][1] == 'h') {
                height = atoi(argv[++i]);
            }
        } 
        else if (!modelpath) {
            modelpath = argv[i];
        }
        else imgpath = argv[i];
    }

    bmp_t bmp;
    RZmodel model;
    RZfont* font = rzFontLoad(fontpath, 10);
    
    if (!font) {
        fprintf(stderr, "razor could not open font file '%s'.\n", fontpath);
        return EXIT_FAILURE;
    }

    if (modelpath) {
        bmp = bmp_load(imgpath);
        if (!bmp.pixels) {
            fprintf(stderr, "razor could not open image file '%s'.\n", imgpath);
            rzFontFree(font);
            return EXIT_FAILURE;
        }

        model = rzModelLoad(modelpath, &bmp);
        if (!model.mesh.vertices.size) {
            fprintf(stderr, "razor could not open 3D model file '%s'.\n", modelpath);
            rzFontFree(font);
            bmp_free(&bmp);
            return EXIT_FAILURE;
        }
    } else {
        bmp = bmp_height_colors();
        model = rzModelPerlin(&bmp, 40);
    }

    Px* pixbuf = spxeStart("razor", 800, 600, width, height);
    if (!pixbuf) {
        fprintf(stderr, "razor could not initiate spxe pixel engine.\n");
        bmp_free(&bmp);
        rzModelFree(&model);
        rzFontFree(font);
        return EXIT_FAILURE;
    }

    spxeMouseVisible(0);

    RZframebuffer framebuffer = rzFramebufferCreate((bmp_t){width, height, sizeof(Px), (unsigned char*)pixbuf});
    rzFramebufferClearColor((Px){125, 125, 255, 255});

    float angle = 0.0;
    const float halfWidth = (float)width * 0.5F;
    const float halfHeight = (float)height * 0.5F;
    const float aspect = (float)width / (float)height;
    const mat4 proj = mat4_perspective(FOV, aspect, Z_NEAR, Z_FAR);

    vec3 dir, right, up, pos = {-3.0F, 1.0F, 8.0F};
    mat4 view, mvp, scale;
    vec2 mouse;

    const Px red = {255, 0, 0, 255};
    const ivec2 texPos = {10.0F, 10.0F};
    const vec3 vZero = {0.0F, 0.0F, 0.0F};
    const vec3 vOne = {1.0F, 1.0F, 1.0F};
    const vec3 vRot = {0.0F, 1.0F, 0.0F};

    float T = spxeTime();
    while (spxeRun(pixbuf)) {
        float t = spxeTime();
        float dT = t - T;
        T = t;

        const float f = dT * 10.0;
        angle += dT;

        if (spxeKeyPressed(ESCAPE)) {
            break;
        }

        if (spxeKeyDown(W)) {
            pos = vec3_sub(pos, vec3_mult(dir, f));
        }
        if (spxeKeyDown(S)) {
            pos = vec3_add(pos, vec3_mult(dir, f));
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
        if (spxeKeyPressed(SPACE)) {
            rzRasterModeSwitch();
        }

        if (!uitimer--) {
            sprintf(uibuf, "Razor: %f", dT);
            uitimer = TIMEOUT_TIME;
        }

        spxeMousePos(&mousex, &mousey);
        mouse = (vec2){((float)mousex - halfWidth) * SENSIBILITY, ((float)(height - mousey) - halfHeight) * SENSIBILITY};

        dir = vec3_normal((vec3){cosf(mouse.y) * sinf(mouse.x), sinf(mouse.y), cosf(mouse.y) * cosf(mouse.x)});
        right = vec3_normal((vec3){sinf(mouse.x - HALF_PI), 0.0, cosf(mouse.x - HALF_PI)});
        up = vec3_cross(right, dir);
        
        scale = mat4_model(vZero, vOne, vRot, angle);
        view = mat4_look_at(pos, vec3_add(pos, dir), up);
        mvp = mat4_mult(proj, mat4_mult(view, scale));

        rzFramebufferClear(&framebuffer);
        rzModelDraw(&framebuffer, &model, &mvp);
        rzFontDrawText(&framebuffer.bitmap, font, uibuf, red, texPos);
    }

    free(framebuffer.zbuffer);
    rzModelFree(&model);
    bmp_free(&bmp);
    rzFontFree(font);
    return spxeEnd(pixbuf);
}

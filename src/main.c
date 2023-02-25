#define SPXE_APPLICATION
#include <razor.h>
#include <stdlib.h>
#include <stdio.h>
#include <mass.h>

#define PERSPECTIVE_SCALE (2.5)
#define SENSIBILITY (0.01)
#define HALF_PI (M_PI * 0.5)
#define FOV_DEGREES (15.0)
#define FOV (FOV_DEGREES / (180.0 / M_PI))

static RZtriangle rzTriangleProject(const mat4* mvp, const vec3* pos, const vec2* uv, const vec3* normals)
{
    RZtriangle t;
    for (int i = 0; i < 3; ++i) {
        vec4 p = vec4_mult_mat4((vec4){pos[i].x, pos[i].y, pos[i].z, 1.0}, *mvp);
        vec4 n = vec4_mult_mat4((vec4){normals[i].x, normals[i].y, normals[i].z, 0.0}, *mvp);
        t.vertices[i].pos.x = PERSPECTIVE_SCALE * ((p.x / p.w) * (float)spxe.scrres.width) / p.w + spxe.scrres.width * 0.5;
        t.vertices[i].pos.y = PERSPECTIVE_SCALE * ((p.y / p.w) * (float)spxe.scrres.height) / p.w + spxe.scrres.height * 0.5;
        t.vertices[i].pos.z = 1.0 / p.z;
        t.vertices[i].uv.x = uv[i].x * t.vertices[i].pos.z;
        t.vertices[i].uv.y = uv[i].y * t.vertices[i].pos.z;
        t.vertices[i].normal = (vec3){n.x, n.y, n.z};
    }
    return t;
}

static float rzTriangleArea(const RZtriangle t)
{
    return 0.5 *
       ((t.vertices[0].pos.x * t.vertices[1].pos.y - t.vertices[1].pos.x * t.vertices[0].pos.y) +
        (t.vertices[1].pos.x * t.vertices[2].pos.y - t.vertices[2].pos.x * t.vertices[1].pos.y) +
        (t.vertices[2].pos.x * t.vertices[0].pos.y - t.vertices[0].pos.x * t.vertices[2].pos.y));
}

static vec3 rzTriangleNormal(const RZtriangle t)
{
    vec3 e1 = _vec3_sub(t.vertices[1].pos, t.vertices[0].pos);
    vec3 e2 = _vec3_sub(t.vertices[2].pos, t.vertices[0].pos);
    return vec3_normal(_vec3_cross(e1, e2));
}

static void rzMeshNormalAverage(const struct vector* positions, struct vector* normals)
{
    struct vector eq = vector_create(sizeof(size_t));
    vec3* pos = positions->data, *n = normals->data;
    for (size_t i = 0; i < positions->size; ++i) {
        vec3 sum = {0.0, 0.0, 0.0};
        for (size_t j = 0; j < positions->size; ++j) {
            if (!memcmp(pos + i, pos + j, sizeof(vec3))) {
                sum = vec3_add(sum, n[j]);
                vector_push(&eq, &j);
            }
        }

        sum = vec3_normal(sum);
        const size_t* indices = eq.data;
        for (size_t j = 0; j < eq.size; ++j) {
            n[indices[j]] = sum;
        }
        
        vector_clear(&eq);
    }
    vector_free(&eq);
}

static void rzMeshNormalize(Mesh3D* mesh)
{
    RZtriangle t;
    const vec3 vZero = {0.0, 0.0, 0.0};

    const vec3* vertices = mesh->vertices.data;
    const vec2* uvs = mesh->uvs.data;
    const size_t size = mesh->vertices.size;

    vector_free(&mesh->normals);
    mesh->normals = vector_reserve(sizeof(vec3), size);
    
    for (size_t i = 0; i < size; i += 3) {
        for (int j = 0; j < 3; ++j) {
            t.vertices[j] = (RZvertex){vertices[i + j], uvs[i + j], vZero};
        }
        
        const vec3 normal = rzTriangleNormal(t);
        for (int j = 0; j < 3; ++j) {
            vector_push(&mesh->normals, &normal);
        }
    }
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

    const char* fileimg = "assets/textures/image.png";
    const char* filefont = "assets/fonts/Pixeled.ttf";
    const char* fileobj = "assets/models/voxel.obj";

    bmp_t bmp = bmp_load(fileimg);
    if (!bmp.pixels) {
        printf("razor could not open image file '%s'.\n", fileimg);
        return EXIT_FAILURE;
    }

    RZfont* font = rzFontLoad(filefont, 10);
    if (!font) {
        printf("razor could not open font file '%s'.\n", filefont);
        return EXIT_FAILURE;
    }

    Mesh3D mesh = mesh3D_load(fileobj);
    if (!mesh.vertices.size) {
        printf("razor could not open 3D model from file '%s'.\n", fileobj);
        return EXIT_FAILURE;
    }

    if (!mesh.uvs.size) {
        mesh.uvs = vector_reserve(sizeof(vec2), mesh.vertices.size);
        memset(mesh.uvs.data, 0, mesh.uvs.size * mesh.uvs.bytes);
    }

    rzMeshNormalize(&mesh);
    rzMeshNormalAverage(&mesh.vertices, &mesh.normals);

    const size_t vertcount =  mesh.vertices.size;;
    const vec3* vertices = mesh.vertices.data;
    const vec2* uvs = mesh.uvs.data;
    const vec3* normals = mesh.normals.data;

    Px* pixbuf = spxeStart("razor", 800, 600, width, height);
    if (!pixbuf) {
        printf("razor could not initiate spxe pixel engine.\n");
        return EXIT_FAILURE;
    }
    
    spxeMouseVisible(0);

    RZframebuffer framebuffer = rzFramebufferCreate((bmp_t){width, height, sizeof(Px), (unsigned char*)pixbuf});
    rzFramebufferClearColor((Px){125, 125, 255, 255});

    float angle = 0.0;
    const float halfWidth = (float)width * 0.5;
    const float halfHeight = (float)height * 0.5;
    const float aspect = (float)width / (float)height;
    const mat4 proj = mat4_perspective(FOV, aspect, Z_NEAR, Z_FAR);
    
    vec3 dir, right, up, pos = {-3.0, 1.0, 8.0};
    mat4 view, mvp, scale;
    vec2 mouse;

    const Px red = {255, 0, 0, 255};
    const ivec2 texPos = {10.0, 10.0};
    const vec3 vZero = {0.0, 0.0, 0.0};
    const vec3 vOne = {1.0, 1.0, 1.0};
    const vec3 vRot = {0.0, 1.0, 0.0};

    float T = spxeTime();
    while (spxeRun(pixbuf)) {
        float t = spxeTime();
        float dT = t - T;
        T = t;
    
        //printf("T: %f\n", dT);

        const float f = dT * 10.0;
        angle += dT;

        if (spxeKeyPressed(ESCAPE)) {
            break;
        }
        if (spxeKeyDown(P)) {
            printf("Pos: %f, %f, %f\nDir: %f, %f, %f\n", pos.x, pos.y, pos.z, dir.x, dir.y, dir.z);
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

        spxeMousePos(&mousex, &mousey);
        mouse = (vec2){((float)mousex - halfWidth) * SENSIBILITY, ((float)(height - mousey) - halfHeight) * SENSIBILITY};

        dir = vec3_normal((vec3){cosf(mouse.y) * sinf(mouse.x), sinf(mouse.y), cosf(mouse.y) * cosf(mouse.x)});
        right = vec3_normal((vec3){sinf(mouse.x - HALF_PI), 0.0, cosf(mouse.x - HALF_PI)});
        up = vec3_cross(right, dir);
        
        scale = mat4_model(vZero, vOne, vRot, angle);
        view = mat4_look_at(pos, vec3_add(pos, dir), up);
        mvp = mat4_mult(proj, mat4_mult(view, scale));

        rzFramebufferClear(&framebuffer);
        for (size_t i = 0; i < vertcount; i += 3) {
            RZtriangle t = rzTriangleProject(&mvp, vertices + i, uvs + i, normals + i);
            if (rzTriangleArea(t) < 0.0) {
                rzRasterize(&framebuffer, &bmp, t);
            }
        }
        rzFontDrawText(&framebuffer.bitmap, font, "Razor", red, texPos);
    }

    free(framebuffer.zbuffer);
    mesh3D_free(&mesh);
    bmp_free(&bmp);
    rzFontFree(font);
    spxeEnd(pixbuf);

    return EXIT_SUCCESS;
}

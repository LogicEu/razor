#include <razor.h>
#include <string.h>

RZmodel rzModelLoad(const char* modelpath, bmp_t* texture)
{
    RZmodel model;
    model.texture = NULL;
    model.mesh = mesh3D_load(modelpath);
    if (!model.mesh.vertices.size) {
        return model;
    }

    if (!model.mesh.uvs.size) {
        model.mesh.uvs = vector_reserve(sizeof(vec2), model.mesh.vertices.size);
        memset(model.mesh.uvs.data, 0, model.mesh.uvs.size * sizeof(vec2));
    }

    rzMeshNormalize(&model.mesh);
    rzMeshNormalAverage(&model.mesh.vertices, &model.mesh.normals);

    model.texture = texture;
    return model;
}

void rzModelFree(RZmodel* model)
{
    mesh3D_free(&model->mesh);
    model->texture = NULL;
}

void rzModelDraw(RZframebuffer* framebuffer, const RZmodel* model, const mat4* mvp)
{
    size_t i;
    const vec3* vertices = model->mesh.vertices.data;
    const vec3* normals = model->mesh.normals.data;
    const vec2* uvs = model->mesh.uvs.data;
    const size_t vertcount = model->mesh.vertices.size;
    for (i = 0; i < vertcount; i += 3) {
        RZtriangle t = rzTriangleProject(mvp, vertices + i, uvs + i, normals + i);
        if (rzTriangleArea(t) < 0.0) {
            rzRasterize(framebuffer, model->texture, t);
        }
    }
}

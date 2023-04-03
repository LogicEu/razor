#include <razor.h>
#include <string.h>

void rzMeshNormalAverage(const struct vector* positions, struct vector* normals)
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

void rzMeshNormalize(Mesh3D* mesh)
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

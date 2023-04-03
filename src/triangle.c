#include <razor.h>

#define PERSPECTIVE_SCALE 2.5

RZtriangle rzTriangleProject(const mat4* mvp, const vec3* pos, const vec2* uv, const vec3* normals)
{
    RZtriangle t;
    int scrwidth, scrheight, i;
    spxeScreenSize(&scrwidth, &scrheight);
    for (i = 0; i < 3; ++i) {
        vec4 p = vec4_mult_mat4((vec4){pos[i].x, pos[i].y, pos[i].z, 1.0f}, *mvp);
        vec4 n = vec4_mult_mat4((vec4){normals[i].x, normals[i].y, normals[i].z, 0.0}, *mvp);
        t.vertices[i].pos.x = PERSPECTIVE_SCALE * ((p.x / p.w) * (float)scrwidth) / p.w + scrwidth * 0.5f;
        t.vertices[i].pos.y = PERSPECTIVE_SCALE * ((p.y / p.w) * (float)scrheight) / p.w + scrheight * 0.5f;
        t.vertices[i].pos.z = 1.0f / p.z;
        t.vertices[i].uv.x = uv[i].x * t.vertices[i].pos.z;
        t.vertices[i].uv.y = uv[i].y * t.vertices[i].pos.z;
        t.vertices[i].normal = (vec3){n.x, n.y, n.z};
    }
    return t;
}

float rzTriangleArea(const RZtriangle t)
{
    return 0.5 *
       ((t.vertices[0].pos.x * t.vertices[1].pos.y - t.vertices[1].pos.x * t.vertices[0].pos.y) +
        (t.vertices[1].pos.x * t.vertices[2].pos.y - t.vertices[2].pos.x * t.vertices[1].pos.y) +
        (t.vertices[2].pos.x * t.vertices[0].pos.y - t.vertices[0].pos.x * t.vertices[2].pos.y));
}

vec3 rzTriangleNormal(const RZtriangle t)
{
    vec3 e1 = _vec3_sub(t.vertices[1].pos, t.vertices[0].pos);
    vec3 e2 = _vec3_sub(t.vertices[2].pos, t.vertices[0].pos);
    return vec3_normal(_vec3_cross(e1, e2));
}

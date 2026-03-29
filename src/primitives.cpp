#include <raylib.h>
#include <eecs.h>
#include <cstring>

#include "dcengine/primitives.h"
#include "dcengine/textures.h"
#include "dcengine/math.h"

static void fix_cube_coords(Mesh& mesh, const vec3f& tcMult)
{
  if (mesh.texcoords != nullptr)
  {
    float texcoords[] = {
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f
    };
    for (int i = 0; i < 8 * 2 * 1; ++i)
        texcoords[i] *= i % 2 ? tcMult.y : tcMult.x;
    for (int i = 8 * 2 * 1; i < 8 * 2 * 2; ++i)
        texcoords[i] *= i % 2 ? tcMult.x : tcMult.z;
    for (int i = 8 * 2 * 2; i < 8 * 2 * 3; ++i)
        texcoords[i] *= i % 2 ? tcMult.z : tcMult.y;
    memcpy(mesh.texcoords, texcoords, 24*2*sizeof(float));
    UpdateMeshBuffer(mesh, SHADER_LOC_VERTEX_TEXCOORD01, mesh.texcoords, sizeof(float) * 2 * mesh.vertexCount, 0);
  }
}

void register_primitives(eecs::Registry& reg)
{
    // TODO: move somewhere
    static Image defEmissiveImage = GenImageColor(1, 1, BLACK);
    static Texture2D defEmissiveTex = LoadTextureFromImage(defEmissiveImage);
    eecs::reg_enter(reg, [&](eecs::EntityId eid, const vec3f& primitiveBlock_size)
    {
        Mesh mesh = GenMeshCube(primitiveBlock_size.x, primitiveBlock_size.y, primitiveBlock_size.z);
        const vec3f tcMult = eecs::get_comp_or(reg, eid, COMPID(vec3f, primitiveBlock_tcMult), primitiveBlock_size);
        fix_cube_coords(mesh, tcMult);
        eecs::EntityWrap ent = eecs::wrap_entity(reg, eid);
        Model model = LoadModelFromMesh(mesh);
        SetMaterialTexture(&model.materials[0], MATERIAL_MAP_EMISSION, defEmissiveTex);
        ent.set(COMPID(Model, model), model);
    }, COMPID(const vec3f, primitiveBlock_size));

    eecs::reg_enter(reg, [&](eecs::EntityId eid, float primitiveRamp_base, float primitiveRamp_top, float primitiveRamp_width, float primitiveRamp_length)
    {
        Mesh mesh = { 0 };
        mesh.triangleCount = 12;
        mesh.vertexCount = mesh.triangleCount * 3;

        mesh.vertices = new float[mesh.vertexCount * 3];
        mesh.normals = new float[mesh.vertexCount * 3];
        mesh.texcoords = new float[mesh.vertexCount * 2];

        //
        //    +-----+
        //   /|    /|
        //  +-----+ |
        //  | +-- |-+
        //  |/    |/
        //  +-----+
        //

        const float range = primitiveRamp_top - primitiveRamp_base;
        const float avgHt = (primitiveRamp_top + primitiveRamp_base) * 0.5f;
        const float ramp = range / primitiveRamp_length;
        const float bottom = -avgHt;
        const float base = bottom + primitiveRamp_base;
        const float top = bottom + primitiveRamp_top;
        vec3f inclination = {0.f, sqrtf(1.f - sqr(ramp)), ramp};
        float vertices[] = {
            -primitiveRamp_width * 0.5f, bottom, primitiveRamp_length * 0.5f,
            +primitiveRamp_width * 0.5f, bottom, primitiveRamp_length * 0.5f,
            +primitiveRamp_width * 0.5f, top, primitiveRamp_length * 0.5f,
            -primitiveRamp_width * 0.5f, top, primitiveRamp_length * 0.5f,

            -primitiveRamp_width * 0.5f, bottom, -primitiveRamp_length * 0.5f,
            -primitiveRamp_width * 0.5f, base, -primitiveRamp_length * 0.5f,
            +primitiveRamp_width * 0.5f, base, -primitiveRamp_length * 0.5f,
            +primitiveRamp_width * 0.5f, bottom, -primitiveRamp_length * 0.5f,

            -primitiveRamp_width * 0.5f, base, -primitiveRamp_length * 0.5f,
            -primitiveRamp_width * 0.5f, top, primitiveRamp_length * 0.5f,
            +primitiveRamp_width * 0.5f, top, primitiveRamp_length * 0.5f,
            +primitiveRamp_width * 0.5f, base, -primitiveRamp_length * 0.5f,

            -primitiveRamp_width * 0.5f, bottom, -primitiveRamp_length * 0.5f,
            +primitiveRamp_width * 0.5f, bottom, -primitiveRamp_length * 0.5f,
            +primitiveRamp_width * 0.5f, bottom, +primitiveRamp_length * 0.5f,
            -primitiveRamp_width * 0.5f, bottom, +primitiveRamp_length * 0.5f,

            +primitiveRamp_width * 0.5f, bottom, -primitiveRamp_length * 0.5f,
            +primitiveRamp_width * 0.5f, base, -primitiveRamp_length * 0.5f,
            +primitiveRamp_width * 0.5f, top, +primitiveRamp_length * 0.5f,
            +primitiveRamp_width * 0.5f, bottom, +primitiveRamp_length * 0.5f,

            -primitiveRamp_width * 0.5f, bottom, -primitiveRamp_length * 0.5f,
            -primitiveRamp_width * 0.5f, bottom, +primitiveRamp_length * 0.5f,
            -primitiveRamp_width * 0.5f, top, +primitiveRamp_length * 0.5f,
            -primitiveRamp_width * 0.5f, base, -primitiveRamp_length * 0.5f
        };
        float texcoords[] = {
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,

            1.0f, 1.0f,
            1.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 1.0f,

            0.0f, 1 - primitiveRamp_base,
            0.0f, 1 - primitiveRamp_top,
            1.0f, 1 - primitiveRamp_top,
            1.0f, 1 - primitiveRamp_base,

            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
            0.0f, 0.0f,

            0.0f, 1 - primitiveRamp_base,
            0.0f, 1 - primitiveRamp_top,
            1.0f, 1 - primitiveRamp_top,
            1.0f, 1 - primitiveRamp_base,

            0.0f, 1 - primitiveRamp_base,
            1.0f, 1 - primitiveRamp_base,
            1.0f, 1 - primitiveRamp_top,
            0.0f, 1 - primitiveRamp_top
        };
        float normals[] = {
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,

            0.0f, 0.0f,-1.0f,
            0.0f, 0.0f,-1.0f,
            0.0f, 0.0f,-1.0f,
            0.0f, 0.0f,-1.0f,

            inclination.x, inclination.y, inclination.z,
            inclination.x, inclination.y, inclination.z,
            inclination.x, inclination.y, inclination.z,
            inclination.x, inclination.y, inclination.z,

            0.0f,-1.0f, 0.0f,
            0.0f,-1.0f, 0.0f,
            0.0f,-1.0f, 0.0f,
            0.0f,-1.0f, 0.0f,

            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,

            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f
        };

        memcpy(mesh.vertices, vertices, 24*3*sizeof(float));
        memcpy(mesh.texcoords, texcoords, 24*2*sizeof(float));
        memcpy(mesh.normals, normals, 24*3*sizeof(float));

        mesh.indices = (unsigned short *)RL_MALLOC(36*sizeof(unsigned short));

        int k = 0;

        // Indices can be initialized right now
        for (int i = 0; i < 36; i += 6)
        {
            mesh.indices[i] = 4*k;
            mesh.indices[i + 1] = 4*k + 1;
            mesh.indices[i + 2] = 4*k + 2;
            mesh.indices[i + 3] = 4*k;
            mesh.indices[i + 4] = 4*k + 2;
            mesh.indices[i + 5] = 4*k + 3;

            k++;
        }

        UploadMesh(&mesh, false);

        Model model = LoadModelFromMesh(mesh);
        eecs::EntityWrap ent = eecs::wrap_entity(reg, eid);
        ent.set(COMPID(Model, model), model);
    }, COMPID(const float, primitiveRamp_base), COMPID(const float, primitiveRamp_top), COMPID(const float, primitiveRamp_width), COMPID(const float, primitiveRamp_length));

    eecs::reg_enter(reg, [&](eecs::EntityId eid, const std::string& model_filename)
    {
        Model model = LoadModel(model_filename.c_str());
        eecs::EntityWrap ent = eecs::wrap_entity(reg, eid);
        SetMaterialTexture(&model.materials[0], MATERIAL_MAP_EMISSION, defEmissiveTex);
        ent.set(COMPID(Model, model), model);
    }, COMPID(const std::string, model_filename));

    eecs::reg_enter(reg, [&](eecs::EntityId eid, Model& model, const Texture2D& texture_diff)
    {
        SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE, texture_diff);
        SetMaterialTexture(&model.materials[0], MATERIAL_MAP_ALBEDO, texture_diff);
    }, COMPID(Model, model), COMPID(const Texture2D, texture_diff));
    eecs::reg_enter(reg, [&](eecs::EntityId eid, Model& model, const Texture2D& texture_emissive)
    {
        SetMaterialTexture(&model.materials[0], MATERIAL_MAP_EMISSION, texture_emissive);
    }, COMPID(Model, model), COMPID(const Texture2D, texture_emissive));

    eecs::reg_enter(reg, [&](eecs::EntityId eid, float billboard_size)
    {
        Mesh mesh = { 0 };
        mesh.triangleCount = 2;
        mesh.vertexCount = 4;

        mesh.vertices = new float[mesh.vertexCount * 3];
        mesh.normals = new float[mesh.vertexCount * 3];
        mesh.texcoords = new float[mesh.vertexCount * 2];

        // 0   1
        // +--+
        // | /|
        // |/ |
        // +--+
        // 2   3
        float vertices[] = {
            -billboard_size * 0.5f, +billboard_size * 0.5f, 0.f,
            +billboard_size * 0.5f, +billboard_size * 0.5f, 0.f,
            -billboard_size * 0.5f, -billboard_size * 0.5f, 0.f,
            +billboard_size * 0.5f, -billboard_size * 0.5f, 0.f,
        };
        float texcoords[] = {
            1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
        };
        float normals[] = {
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
        };
        unsigned short indices[] = {
            0, 1, 2,
            1, 3, 2
        };

        memcpy(mesh.vertices, vertices, mesh.vertexCount*3*sizeof(float));
        memcpy(mesh.texcoords, texcoords, mesh.vertexCount*2*sizeof(float));
        memcpy(mesh.normals, normals, mesh.vertexCount*3*sizeof(float));

        mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));
        memcpy(mesh.indices, indices, mesh.triangleCount*3*sizeof(unsigned short));

        UploadMesh(&mesh, false);
        Model model = LoadModelFromMesh(mesh);
        eecs::set_component(reg, eid, COMPID(Model, model), model);
    }, COMPID(const float, billboard_size));
}


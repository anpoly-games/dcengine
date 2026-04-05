#include <raylib.h>
#include <eecs.h>
#include <algorithm>
#include <cmath>
#include <random>
#include <cstring>

#include "dcengine/renderer.h"
#include "raymath.h"
#include "dcengine/level.h"
#include "dcengine/game.h"
#include "dcengine/tags.h"

#define RLIGHTS_IMPLEMENTATION
#include "dcengine/rlights.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

static Shader lightingShader;
static Light lights[MAX_LIGHTS];
static Shader billboardShader;
static int deathProgressLoc = -1;
static float cachedDeathProgress = -1.f;

void register_renderer(eecs::Registry& reg)
{
    lightingShader = LoadShader(TextFormat("res/shaders/glsl%i/lighting.vs", GLSL_VERSION),
                               TextFormat("res/shaders/glsl%i/lighting.fs", GLSL_VERSION));
    lightingShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(lightingShader, "viewPos");
    lightingShader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(lightingShader, "emissiveMap");
    deathProgressLoc = GetShaderLocation(lightingShader, "deathProgress");

    billboardShader = LoadShader(NULL, TextFormat("res/shaders/glsl%i/billboardShader.fs", GLSL_VERSION));

    int ambientLoc = GetShaderLocation(lightingShader, "ambient");
    float ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    SetShaderValue(lightingShader, ambientLoc, ambient, SHADER_UNIFORM_VEC4);

    memset(lights, 0, sizeof(Light) * MAX_LIGHTS);
    // Move to render actually!
    eecs::reg_system(reg, [&](eecs::EntityId eid, const Camera& camera)
    {
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(lightingShader, lightingShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

        int prevLights = lightsCount;
        lightsCount = 0;
        int nl = 0;
        eecs::query_entities(reg, [&](eecs::EntityId, float light_strength, const vec3f& light_direction)
        {
            if (nl >= MAX_LIGHTS)
                return;
            lights[nl] = CreateLight(LIGHT_DIRECTIONAL, toRLVec3(light_direction), Vector3Zero(), Vector4{1.f, 1.f, 1.f, light_strength}, lightingShader);
            nl++;
        }, COMPID(const float, light_strength), COMPID(const vec3f, light_direction));
        eecs::query_entities(reg, [&](eecs::EntityId, float light_strength, const vec3f& position)
        {
            if (nl >= MAX_LIGHTS)
                return;
            lights[nl] = CreateLight(LIGHT_POINT, toRLVec3(position), Vector3Zero(), Vector4{1.f, 1.f, 1.f, light_strength}, lightingShader);
            nl++;
        }, COMPID(const float, light_strength), COMPID(const vec3f, position));

        for (int i = nl; i < prevLights; ++i)
            InitDisabledLight(i, lightingShader);

        float ambientOverride = eecs::get_comp_or(reg, eid, COMPID(float, cam_ambientOverride), 0.f);
        int ambientLoc = GetShaderLocation(lightingShader, "ambient");
        float ambient[4] = { ambientOverride, ambientOverride, ambientOverride, 1.0f };
        SetShaderValue(lightingShader, ambientLoc, ambient, SHADER_UNIFORM_VEC4);

        const int fogParamsLoc = GetShaderLocation(lightingShader, "fogParams");
        vec4f fogParams = vec4f(0, 0, 0, 0);
        eecs::query_entities(reg, [&](eecs::EntityId, const vec4f& fog_params)
        {
            fogParams = fog_params;
        }, COMPID(const vec4f, fog_params));
        SetShaderValue(lightingShader, fogParamsLoc, &fogParams, SHADER_UNIFORM_VEC4);

        eecs::query_entities(reg, [&](eecs::EntityId eid, float& rotation, const vec3f& position, Tag billboard)
        {
            const vec3f camDir = tov3(camera.target) - tov3(camera.position);
            const float angle = 90.f - atan2f(camDir.z, camDir.x) * RAD2DEG;
            rotation = angle;
        }, COMPID(float, rotation), COMPID(const vec3f, position), COMPID(Tag, billboard));

        eecs::query_entities(reg, [&](eecs::EntityId eid, Model& model, const vec3f& position)
        {
            model.materials[0].shader = lightingShader;

            float deathProgress = 0.f;
            const float deathTimer = eecs::get_comp_or(reg, eid, COMPID(float, death_timer), -1.f);
            if (deathTimer >= 0.f)
            {
                const float deathDuration = std::max(eecs::get_comp_or(reg, eid, COMPID(float, death_duration), 1.f), 0.0001f);
                deathProgress = std::clamp(deathTimer / deathDuration, 0.f, 1.f);
            }

            if (deathProgressLoc >= 0 && std::fabs(deathProgress - cachedDeathProgress) > 0.0001f)
            {
                SetShaderValue(lightingShader, deathProgressLoc, &deathProgress, SHADER_UNIFORM_FLOAT);
                cachedDeathProgress = deathProgress;
            }

            vec3f moffs = eecs::get_comp_or(reg, eid, COMPID(vec3f, modelOffs), vec3f(0,0,0));
            DrawModelEx(model, toRLVec3(position + moffs), Vector3{0.f, 1.f, 0.f}, eecs::get_comp_or(reg, eid, COMPID(float, rotation), 0.f), Vector3{1.f, 1.f, 1.f}, WHITE);
        }, COMPID(Model, model), COMPID(const vec3f, position));
    }, COMPID(const Camera, camera));
}

void pre_draw_call(eecs::Registry& reg)
{
}


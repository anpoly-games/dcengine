#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <eecs.h>
#include <edat.h>
#include <parsers.h>
#include <float.h>

#include "dcengine/math.h"
#include "dcengine/cam.h"
#include "dcengine/tags.h"
#include "dcengine/renderer.h"
#include "dcengine/prefabs.h"

void register_cam(eecs::Registry& reg)
{
    eecs::reg_system(reg, [&](eecs::EntityId eid, vec3f& position, vec3f& direction, const Camera&)
    {
        eecs::query_entities(reg, [&](eecs::EntityId plEid, const vec3f& ppos, const vec3f& pdir, Tag player)
        {
            position = move_to_vec<vec3f>(position, ppos + vec3f{0.f, 0.4f, 0.f}, GetFrameTime(), eecs::get_comp_or(reg, eid, COMPID(float, cam_moveSpeed), FLT_MAX));
            const float maxTime = 0.2f;
            float timeSinceHit = eecs::get_comp_or(reg, plEid, COMPID(float, timeSinceHit), maxTime);
            float vAngle = timeSinceHit < maxTime ? sinf(timeSinceHit / maxTime * PI) * 0.2f : 0.f;
            float curAngle = atan2f(direction.x, direction.z);
            const float targAngle = atan2f(pdir.x, pdir.z);
            if (fabsf(targAngle - curAngle) > PI)
                curAngle += sign(targAngle - curAngle) * PI * 2.f;
            curAngle = move_to(curAngle, targAngle, GetFrameTime(), eecs::get_comp_or(reg, eid, COMPID(float, cam_rotSpeed), FLT_MAX));
            direction = vec3f{sinf(curAngle) * cosf(vAngle), sinf(vAngle), cosf(curAngle) * cosf(vAngle)};
        }, COMPID(const vec3f, position), COMPID(const vec3f, direction), COMPID(const Tag, player));
    }, COMPID(vec3f, position), COMPID(vec3f, direction), COMPID(const Camera, camera));


    eecs::reg_system(reg, [&](eecs::EntityId eid, Camera& camera, const vec3f& position, const vec3f& direction, float cam_backOffset)
    {
        camera.position = toRLVec3(position - direction * cam_backOffset);
        camera.target = toRLVec3(position + direction);
        camera.up = Vector3{0.f, 1.f, 0.f};
    }, COMPID(Camera, camera), COMPID(const vec3f, position), COMPID(const vec3f, direction), COMPID(const float, cam_backOffset));
    eecs::reg_system(reg, [&](eecs::EntityId eid, Camera& camera, float cam_wideAngle, float cam_normalAngle)
    {
        if (IsKeyPressed(KEY_C))
        {
            if (camera.fovy == cam_normalAngle)
                camera.fovy = cam_wideAngle;
            else
                camera.fovy = cam_normalAngle;
        }
    }, COMPID(Camera, camera), COMPID(const float, cam_wideAngle), COMPID(const float, cam_normalAngle));

    eecs::reg_system(reg, [&](eecs::EntityId eid, float& cam_ambientOverride, float cam_defLightStr)
    {
        if (IsKeyPressed(KEY_L))
        {
            if (eecs::has_comp(reg, eid, COMPID(float, light_strength)))
            {
                eecs::del_component(reg, eid, COMPID(float, light_strength));
                cam_ambientOverride = 1.f;
            }
            else
            {
                eecs::set_component(reg, eid, COMPID(float, light_strength), cam_defLightStr);
                cam_ambientOverride = 0.f;
            }
        }
    }, COMPID(float, cam_ambientOverride), COMPID(const float, cam_defLightStr));

    eecs::reg_enter(reg, [&](eecs::EntityId eid, int cam_resWidth, int cam_resHeight)
    {
        RenderTexture2D renderTex = LoadRenderTexture(cam_resWidth, cam_resHeight);
        eecs::set_component(reg, eid, COMPID(RenderTexture2D, renderTex), renderTex);
    }, COMPID(const int, cam_resWidth), COMPID(const int, cam_resHeight));

    load_prefabs_from_file(reg, "res/prefabs/cam.edat");
}

void begin_cam(eecs::Registry& reg)
{
    eecs::query_entities(reg, [&](eecs::EntityId, RenderTexture2D& renderTex)
    {
        BeginTextureMode(renderTex);
    }, COMPID(RenderTexture2D, renderTex));
    eecs::query_entities(reg, [&](eecs::EntityId eid, const Camera& camera)
    {
        if (!eecs::has_comp(reg, eid, COMPID(RenderTexture2D, renderTex)))
            BeginDrawing();
        Color clearColor = BLACK;
        eecs::query_entities(reg, [&](eecs::EntityId, const vec4f& clear_color)
        {
            clearColor = ColorFromNormalized(toRLVec4(clear_color));
        }, COMPID(const vec4f, clear_color));
        ClearBackground(clearColor);
        BeginMode3D(camera);
    }, COMPID(const Camera, camera));
}

void end_cam(eecs::Registry& reg)
{
    eecs::query_entities(reg, [&](eecs::EntityId eid, const Camera& camera)
    {
        EndMode3D();
    }, COMPID(const Camera, camera));
    eecs::query_entities(reg, [&](eecs::EntityId, RenderTexture2D& renderTex)
    {
        EndTextureMode();
    }, COMPID(RenderTexture2D, renderTex));
}

void render_cam(eecs::Registry& reg)
{
    eecs::query_entities(reg, [&](eecs::EntityId, RenderTexture2D& renderTex, float cam_resMult)
    {
        eecs::query_entities(reg, [&](eecs::EntityId, float window_scaleFactor)
        {
            ClearBackground(BLACK);
            Camera2D screenCam = {0};
            screenCam.zoom = 1.f;
            BeginMode2D(screenCam);
                Rectangle sourceRect = torect(0, 0, renderTex.texture.width, -renderTex.texture.height);
                Rectangle destRect = torect(0, 0, renderTex.texture.width * cam_resMult * window_scaleFactor, renderTex.texture.height * cam_resMult * window_scaleFactor);
                DrawTexturePro(renderTex.texture, sourceRect, destRect, {0.f, 0.f}, 0.f, WHITE);
            EndMode2D();
        }, COMPID(const float, window_scaleFactor));
    }, COMPID(RenderTexture2D, renderTex), COMPID(const float, cam_resMult));
}

void begin_postcam(eecs::Registry& reg)
{
    eecs::query_entities(reg, [&](eecs::EntityId eid, const Camera& camera)
    {
        if (eecs::has_comp(reg, eid, COMPID(RenderTexture2D, renderTex)))
            BeginDrawing();
    }, COMPID(const Camera, camera));
}

void end_postcam(eecs::Registry& reg)
{
    eecs::query_entities(reg, [&](eecs::EntityId eid, const Camera& camera)
    {
        EndDrawing();
    }, COMPID(const Camera, camera));
}

vec2i get_cam_wh(eecs::Registry& reg)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    eecs::query_entities(reg, [&](eecs::EntityId, float window_scaleFactor)
    {
        eecs::query_entities(reg, [&](eecs::EntityId, int cam_resWidth, int cam_resHeight, float cam_resMult)
        {
            width = cam_resWidth * cam_resMult * window_scaleFactor;
            height = cam_resHeight * cam_resMult * window_scaleFactor;
        }, COMPID(const int, cam_resWidth), COMPID(const int, cam_resHeight), COMPID(const float, cam_resMult));
    }, COMPID(const float, window_scaleFactor));
    return {width, height};
}

Vector3 GetWorldToScreen3dEx(eecs::Registry& reg, Vector3 position, Camera camera)
{
    const vec2i cam_wh = get_cam_wh(reg);
    // Calculate projection matrix (from perspective instead of frustum
    int width = cam_wh.x;
    int height = cam_wh.y;

    Matrix matProj = MatrixIdentity();

    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Calculate projection matrix from perspective
        matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)width/(double)height), rlGetCullDistanceNear(), rlGetCullDistanceFar());
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        double aspect = (double)width/(double)height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = MatrixOrtho(-right, right, -top, top, rlGetCullDistanceNear(), rlGetCullDistanceFar());
    }

    // Calculate view matrix from camera look at (and transpose it)
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

    // TODO: Why not use Vector3Transform(Vector3 v, Matrix mat)?

    // Convert world position vector to quaternion
    Quaternion worldPos = { position.x, position.y, position.z, 1.0f };

    // Transform world position to view
    worldPos = QuaternionTransform(worldPos, matView);

    // Transform result to projection (clip space position)
    worldPos = QuaternionTransform(worldPos, matProj);

    // Calculate normalized device coordinates (inverted y)
    Vector3 ndcPos = { worldPos.x/worldPos.w, -worldPos.y/worldPos.w, worldPos.z/worldPos.w };

    // Calculate 2d screen position vector
    Vector2 screenPosition = { (ndcPos.x + 1.0f)/2.0f*(float)width, (ndcPos.y + 1.0f)/2.0f*(float)height };

    return Vector3{screenPosition.x, screenPosition.y, worldPos.w};
}


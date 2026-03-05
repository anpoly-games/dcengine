#include <eecs.h>
#include <raylib.h>

#include "dcengine/math.h"
#include "dcengine/cam.h"

eecs::EntityId create_cam(eecs::Registry& reg)
{
    eecs::EntityWrap cam = eecs::create_wrap_from_prefab(reg, eecs::find_entity(reg, "EditorCam"));
    float normFov = eecs::get_comp_or(reg, cam.eid, COMPID(float, cam_normalAngle), 75.f);
    Camera camera = { { 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, normFov, 0 };
    return cam
        .set(COMPID(Camera, camera), camera)
        .set(COMPID(vec3f, position), {0.f, 0.4f, 0.f})
        .set(COMPID(vec3f, direction), {0.f, 0.f, 1.f}).eid;
}


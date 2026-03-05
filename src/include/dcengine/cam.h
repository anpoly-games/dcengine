#pragma once

void register_cam(eecs::Registry& reg);
eecs::EntityId create_cam(eecs::Registry& reg);

void begin_cam(eecs::Registry& reg);
void end_cam(eecs::Registry& reg);
void render_cam(eecs::Registry& reg);
void begin_postcam(eecs::Registry& reg);
void end_postcam(eecs::Registry& reg);

vec2i get_cam_wh(eecs::Registry& reg);

Vector3 GetWorldToScreen3dEx(eecs::Registry& reg, Vector3 position, Camera camera);


#pragma once

#include <string>
#include <set>
#include <vector>
#include <unordered_map>

#include "math.h"

void register_level(eecs::Registry& reg);

void load_level(eecs::Registry& reg, const char* filename);
eecs::Registry* change_level(eecs::Registry& reg, std::unordered_map<std::string, eecs::Registry*>& registries);
void preload_levels(std::unordered_map<std::string, eecs::Registry*>& registries, float width, float height, float scaleFactor);

// Only one step at a time, not a visibility check!
bool check_collision_dir(eecs::Registry& reg, const vec3f& pos, const vec2i& dir);

bool ray_hit(eecs::Registry& reg, const vec3f& sourcePos, const vec3f& targetPos, eecs::EntityId target);
bool check_occupancy(eecs::Registry& reg, const vec3f& pos);
bool check_floor(eecs::Registry& reg, const vec3f& pos);


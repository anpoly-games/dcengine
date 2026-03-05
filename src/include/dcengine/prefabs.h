#pragma once

std::vector<eecs::EntityId> load_entities_from_file(eecs::Registry& reg, const std::string_view& filename);
std::vector<eecs::EntityId> load_prefabs_from_file(eecs::Registry& reg, const std::string_view& filename);


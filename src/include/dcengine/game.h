#pragma once

void register_systems(eecs::Registry& reg);
eecs::EntityId init_new_world(eecs::Registry& reg);
void restart_world(eecs::Registry& reg);
eecs::Registry& get_registry(const std::string& name);
void save_registries(eecs::Registry& reg);
void load_registries(eecs::Registry& reg);

void pre_draw_call(eecs::Registry& reg);

enum GameState
{
    E_MAIN_MENU = 0,
    E_GAME,
    E_WIN,
    E_LOSE
};

void set_game_state(GameState gs);
bool has_saved();

//inline float get_game_dt(flecs::world& ecs) { return ecs.has<GamePaused>() ? 0.f : ecs.delta_time(); }

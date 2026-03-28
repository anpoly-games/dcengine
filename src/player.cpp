#include <raylib.h>
#include <eecs.h>

#include "dcengine/player.h"
#include "dcengine/tags.h"
#include "dcengine/ui.h"
#include "dcengine/level.h"
#include "dcengine/game.h"
#include "dcengine/audio.h"

void register_player(eecs::Registry& reg)
{
    static Sound stepSnd = LoadSound("res/audio/sfx/step_single_01.ogg");
    eecs::reg_system(reg, [&](eecs::EntityId eid, vec3f& position, vec3f& direction, Tag player)
    {
        if (is_ui_blocks_input(reg))
            return;
        const float turnRight = (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_KP_7) ? -1.f : 0.f) +
                                (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_KP_9) ? 1.f  : 0.f);

        const vec3f rdir = vec3f{-direction.z, direction.y, direction.x};
        if (turnRight != 0.f)
            direction = rdir * turnRight;
        float forward = (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_KP_8) ? 1.f : 0.f) +
                        (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_KP_5) || IsKeyPressed(KEY_KP_2) ? -1.f  : 0.f);
        float right = (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_KP_4) ? -1.f : 0.f) +
                      (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_KP_6) ? 1.f  : 0.f);
        if (eecs::has_comp(reg, eid, COMPID(Tag, allowVerticalMove)))
        {
            float up = (IsKeyPressed(KEY_R) ? 1.f : 0.f) + (IsKeyPressed(KEY_F) ? -1.f : 0.f);
            position.y += up;
        }
        if (forward != 0 && right != 0)
            right = 0.f;
        if (!eecs::has_comp(reg, eid, COMPID(Tag, ignoresCollision)))
        {
            vec3f mdir = direction * forward + right * rdir;
            vec2i mdir2d = vec2i(mdir.x, mdir.z);
            if (mdir2d.mag2() != 0)
            {
                if (check_collision_dir(reg, position, mdir2d) || !check_floor(reg, position + mdir) || check_occupancy(reg, position + mdir))
                    return;
                // align back to grid
                PlaySound(stepSnd);
                vec2i gridPos = pos_to_grid(position);
                position = vec3f(gridPos.x, floorf(position.y + 0.5f), gridPos.y);
            }
        }
        position += direction * forward;
        position += right * rdir;
        // NOTE: possible to have two turns if two buttons pressed at the same time
        if (forward != 0)
            eecs::emit_event(reg, FNV1(player_moved), eecs::invalid_eid, eid);
        if (right != 0)
            eecs::emit_event(reg, FNV1(player_moved), eecs::invalid_eid, eid);
    }, COMPID(vec3f, position), COMPID(vec3f, direction), COMPID(const Tag, player));
}


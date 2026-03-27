#include <raylib.h>
#include <eecs.h>
#include <random>
#include <cstring>
#include <unordered_map>

#include "dcengine/ui.h"

#include "dcengine/cam.h"
#include "dcengine/game.h"
#include "dcengine/level.h"
#include "dcengine/tags.h"

static std::unordered_map<std::string, eecs::Registry*> registries;

eecs::Registry* load_saved_registries(eecs::Registry& reg);

int main(int argc, char** argv)
{
    int width = 1280;
    int height = 720;
    const char* levelToLoad = nullptr;//"level1";
    bool debugMode = false;
    for ( int i=1; i < argc; i++ )
    {
        if (strncmp(argv[i], "-l", 2)==0)
        {
            levelToLoad = argv[++i];
            printf("Loading level %s\n", levelToLoad);
        }

        if (strncmp(argv[i], "-d", 2)==0)
        {
          debugMode = true;
        }
    }

    ChangeDirectory(TextFormat("%s/../Resources/", GetApplicationDirectory()));

    InitWindow(width, height, "The Hidden Facility");
    const int scrWidth = GetMonitorWidth(0);
    const int scrHeight = GetMonitorHeight(0);
    CloseWindow();

    int scaleFactor = std::max(1, std::min((scrWidth - 50) / width, (scrHeight - 50) / height));

    InitWindow(width * scaleFactor, height * scaleFactor, "The Hidden Facility");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    SetExitKey(KEY_NULL);
    InitAudioDevice();

    eecs::Registry* reg = nullptr;

    if (debugMode)
    {
      eecs::EntityId eid = eecs::create_entity(*reg, "DebugMarker"); // existence of this entity means we are in debug mode
    }

    if (levelToLoad)
    {
        preload_levels(registries, width * scaleFactor, height * scaleFactor, scaleFactor); // maybe only in the game?
        auto itf = registries.find(levelToLoad);
        if (itf != registries.end())
            reg = itf->second;
        else
        {
            reg = new eecs::Registry{};
            create_ui_helper(*reg, width * scaleFactor, height * scaleFactor, scaleFactor);
            load_level(*reg, levelToLoad);
            registries[levelToLoad] = reg;
        }
        set_game_state(E_GAME);
    }
    else
    {
        reg = new eecs::Registry{};
        register_systems(*reg);
        init_new_world(*reg);
        preload_levels(registries, width * scaleFactor, height * scaleFactor, scaleFactor); // maybe only in the game?
        create_ui_helper(*reg, width * scaleFactor, height * scaleFactor, scaleFactor);
        registries["default"] = reg;
    }

    while (!WindowShouldClose())
    {
        pre_draw_call(*reg);
            begin_cam(*reg);
                eecs::step(*reg);
            end_cam(*reg);
            begin_postcam(*reg);
                render_cam(*reg);
                draw_ui(*reg, width * scaleFactor, height * scaleFactor, scaleFactor);
            end_postcam(*reg);
        reg = change_level(*reg, registries); // look for a change level request and execute it.
        reg = load_saved_registries(*reg);
        bool shouldQuit = false;
        eecs::query_entities(*reg, [&](eecs::EntityId, Tag quitGame)
        {
            shouldQuit = true;
        }, COMPID(Tag, quitGame));
        if (shouldQuit)
        {
            CloseWindow();
            break;
        }
        assert(reg);
    }
    CloseAudioDevice();

    for ( auto r : registries )
    {
      delete r.second;
    }

    return 0;
}

eecs::Registry& get_registry(const std::string& name)
{
    auto itf = registries.find(name);
    assert(itf != registries.end());
    if (itf == registries.end())
    {
        static eecs::Registry temp;
        return temp;
    }
    return *itf->second;
}

static std::unordered_map<std::string, eecs::Registry*> savedRegistries;
static std::string savedLevel;

void save_registries(eecs::Registry& reg)
{
    std::string currentName;
    for (auto &[name, r] : registries)
        if (r == &reg)
            currentName = name;
    savedLevel = currentName;
    eecs::query_entities(reg, [&](eecs::EntityId, float window_width, float window_height, float window_scaleFactor)
    {
        preload_levels(savedRegistries, window_width, window_height, window_scaleFactor);
        for (auto &[name, r] : registries)
        {
            auto itf = savedRegistries.find(name);
            if (itf == savedRegistries.end())
                continue;
            eecs::Registry& dest = *itf->second;
            dest.entityToName = r->entityToName;
            dest.entityNames = r->entityNames;
            dest.freeEidsList = r->freeEidsList;
            dest.lastValidEid = r->lastValidEid;
            for (auto &[hash, holder] : r->holders)
            {
                auto iitf = dest.holders.find(hash);
                if (iitf == dest.holders.end())
                    dest.holders.emplace(hash, eecs::SparseSetHolder(holder.typeHash, holder.name.c_str(), holder.set->createClone()));
                else
                {
                    delete iitf->second.set;
                    iitf->second.set =  holder.set->createClone();
                }
            }
        }
    }, COMPID(float, window_width), COMPID(float, window_height), COMPID(float, window_scaleFactor));
}

eecs::Registry* load_saved_registries(eecs::Registry& reg)
{
    if (eecs::find_entity(reg, "trigger_load") == eecs::invalid_eid)
        return &reg;

    for (auto &[name, r] : registries)
        delete r;
    registries.clear();
    registries = savedRegistries;
    save_registries(*registries[savedLevel]);
    return registries[savedLevel];
}

void load_registries(eecs::Registry& reg)
{
    eecs::create_entity_wrap(reg, "trigger_load")
        .set(COMPID(Tag, load_registries), Tag{});
}

bool has_saved()
{
    return savedRegistries.size();
}


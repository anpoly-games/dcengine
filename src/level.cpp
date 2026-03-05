#include <raylib.h>
#include <raymath.h>
#include <eecs.h>
#include <climits>
#include <edat.h>
#include <parsers.h>

#include <filesystem>

#include "dcengine/level.h"
#include "dcengine/math.h"
#include "dcengine/tags.h"
#include "dcengine/game.h"
#include "dcengine/prefabs.h"
#include "dcengine/ui.h"

namespace fs = std::filesystem;

bool check_collision_dir(eecs::Registry& reg, const vec3f& pos, const vec2i& dir)
{
    bool res = false;
    eecs::query_entities(reg, [&](eecs::EntityId, const std::set<vec3i>& level_xWalls, const std::set<vec3i>& level_zWalls)
    {
        // First figure out which walls to check
        const std::set<vec3i>& toCheck = dir.x != 0 ? level_xWalls : level_zWalls;
        const vec3i gridPos3d = pos_to_grid3d(pos);
        const vec3i posCheck = vec3i{gridPos3d.x + (dir.x == 1 ? 1 : 0), gridPos3d.y, gridPos3d.z + (dir.y == 1 ? 1 : 0)};
        res = toCheck.contains(posCheck);
    }, COMPID(const std::set<vec3i>, level_xWalls), COMPID(const std::set<vec3i>, level_zWalls));
    return res;
}

void register_level(eecs::Registry& reg)
{
    eecs::create_entity_wrap(reg, "level_walls")
        .set(COMPID(std::set<vec3i>, level_xWalls), {})
        .set(COMPID(std::set<vec3i>, level_zWalls), {})
        .set(COMPID(std::set<vec3i>, level_floors), {});

    std::vector<eecs::EntityId> floors = load_prefabs_from_file(reg, "res/prefabs/floors.edat");
    std::vector<eecs::EntityId> walls = load_prefabs_from_file(reg, "res/prefabs/walls.edat");
    std::vector<eecs::EntityId> doors = load_prefabs_from_file(reg, "res/prefabs/doors.edat");
    std::vector<eecs::EntityId> columns = load_prefabs_from_file(reg, "res/prefabs/columns.edat");
    std::vector<eecs::EntityId> ceilings = load_prefabs_from_file(reg, "res/prefabs/ceilings.edat");
    std::vector<eecs::EntityId> entities = load_prefabs_from_file(reg, "res/prefabs/entities.edat");
    std::vector<eecs::EntityId> logic = load_prefabs_from_file(reg, "res/prefabs/logic.edat");
    std::vector<eecs::EntityId> billboards = load_prefabs_from_file(reg, "res/prefabs/billboards.edat");

    eecs::create_entity_wrap(reg, "floors").set(COMPID(std::vector<eecs::EntityId>, children), floors);
    eecs::create_entity_wrap(reg, "walls").set(COMPID(std::vector<eecs::EntityId>, children), walls);
    eecs::create_entity_wrap(reg, "doors").set(COMPID(std::vector<eecs::EntityId>, children), doors);
    eecs::create_entity_wrap(reg, "columns").set(COMPID(std::vector<eecs::EntityId>, children), columns);
    eecs::create_entity_wrap(reg, "ceilings").set(COMPID(std::vector<eecs::EntityId>, children), ceilings);
    eecs::create_entity_wrap(reg, "entities").set(COMPID(std::vector<eecs::EntityId>, children), entities);
    eecs::create_entity_wrap(reg, "logic").set(COMPID(std::vector<eecs::EntityId>, children), logic);
    eecs::create_entity_wrap(reg, "billboards").set(COMPID(std::vector<eecs::EntityId>, children), billboards);

    static auto find_door_wall_coords = [](const vec3f& pos, float rot)
    {
        const int dir = fabsf(sinf(rot * DEG2RAD)) < 0.5f ? 0 : 1;
        const int x = floorf((pos.x + (dir ? 0.5f : 0.f)) + 0.5f);
        const int z = floorf((pos.z + (dir ? 0.f : 0.5f)) + 0.5f);
        return vec3i{x, z, dir};
    };
    eecs::reg_enter(reg, [&](eecs::EntityId, const vec3f& position, float rotation, Tag wall)
    {
        eecs::query_entities(reg, [&](eecs::EntityId, std::set<vec3i>& level_xWalls, std::set<vec3i>& level_zWalls)
        {
            vec3i xzd = find_door_wall_coords(position, rotation);
            std::set<vec3i>& toAdd = xzd.z == 0 ? level_zWalls : level_xWalls;
            toAdd.insert(vec3i(xzd.x, int(floorf(position.y)), xzd.y));
        }, COMPID(std::set<vec3i>, level_xWalls), COMPID(std::set<vec3i>, level_zWalls));
    }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const Tag, wall));

    eecs::reg_enter(reg, [&](eecs::EntityId, const vec3f& position, float rotation, Tag door, Tag closed)
    {
        eecs::query_entities(reg, [&](eecs::EntityId, std::set<vec3i>& level_xWalls, std::set<vec3i>& level_zWalls)
        {
            vec3i xzd = find_door_wall_coords(position, rotation);
            std::set<vec3i>& toAdd = xzd.z == 0 ? level_zWalls : level_xWalls;
            toAdd.insert(vec3i(xzd.x, int(floorf(position.y)), xzd.y));
        }, COMPID(std::set<vec3i>, level_xWalls), COMPID(std::set<vec3i>, level_zWalls));
    }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const Tag, door), COMPID(const Tag, closed));

    eecs::reg_enter(reg, [&](eecs::EntityId, const vec3f& position, Tag floor)
    {
        eecs::query_entities(reg, [&](eecs::EntityId, std::set<vec3i>& level_floors)
        {
            level_floors.insert(pos_to_grid3d(position));
        }, COMPID(std::set<vec3i>, level_floors));
    }, COMPID(const vec3f, position), COMPID(const Tag, floor));

    eecs::reg_enter(reg, [&](eecs::EntityId, const vec3f& position, Tag ceiling)
    {
        eecs::query_entities(reg, [&](eecs::EntityId, std::set<vec3i>& level_floors)
        {
            level_floors.insert(pos_to_grid3d(position));
        }, COMPID(std::set<vec3i>, level_floors));
    }, COMPID(const vec3f, position), COMPID(const Tag, ceiling));

    eecs::reg_enter(reg, [&](eecs::EntityId, const vec3f& position, float rotation, Tag ladder)
    {
        eecs::query_entities(reg, [&](eecs::EntityId, std::set<vec3i>& level_floors)
        {
            level_floors.insert(pos_to_grid3d(position));
        }, COMPID(std::set<vec3i>, level_floors));
    }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const Tag, ladder));

    eecs::reg_exit(reg, [&](eecs::EntityId, const vec3f& position, float rotation, Tag door, Tag closed)
    {
        eecs::query_entities(reg, [&](eecs::EntityId, std::set<vec3i>& level_xWalls, std::set<vec3i>& level_zWalls)
        {
            vec3i xzd = find_door_wall_coords(position, rotation);
            std::set<vec3i>& toAdd = xzd.z == 0 ? level_zWalls : level_xWalls;
            toAdd.erase(vec3i(xzd.x, int(floorf(position.y)), xzd.y));
        }, COMPID(std::set<vec3i>, level_xWalls), COMPID(std::set<vec3i>, level_zWalls));
    }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const Tag, door), COMPID(const Tag, closed));
}

void load_level(eecs::Registry& reg, const char* filename)
{
    std::filesystem::path fullPath{"res/levels/"};
    fullPath += filename;

    if (!fs::exists(fullPath))
        return;

    restart_world(reg);
    load_entities_from_file(reg, fullPath.string().c_str());
}

eecs::Registry* change_level(eecs::Registry& reg, std::unordered_map<std::string, eecs::Registry*>& registries)
{
    eecs::EntityId eid = eecs::find_entity(reg, "Switch_Level");
    if ( eid != eecs::invalid_eid )
    {
        std::string nextLevel{};
        eecs::query_component(reg, eid, [&](std::string& nl)
        {
            nextLevel = nl;
        }, COMPID(std::string, nextLevel));

        if ( !nextLevel.empty() )
        {
            eecs::del_entity(reg, eid);
            if ( registries.count(nextLevel) )
            {
                eecs::Registry* nReg = registries[nextLevel];
                assert(nReg);
                const char* compsToCopy[] =
                {
                    "attr_strength", "attr_agility", "attr_mind", "attr_body",
                    "prevHitpoints", "hitpoints", "level", "pointsToSpend", "experience",
                    "items_regenX", "items_bearserker", "items_reflexxx", "items_mindDefoger", "items_genius", "items_bandito"
                };
                eecs::query_entities(reg, [&](eecs::EntityId rhs, Tag player)
                {
                    eecs::query_entities(*nReg, [&](eecs::EntityId lhs, Tag player)
                    {
                        for (int i = 0; i < sizeof(compsToCopy) / sizeof(compsToCopy[0]); ++i)
                        {
                            int curVal = eecs::get_comp_or(reg, rhs, eecs::ComponentId<int>(compsToCopy[i]), 0);
                            eecs::set_component(*nReg, lhs, eecs::ComponentId<int>(compsToCopy[i]), curVal);
                        }
                    }, COMPID(Tag, player));
                }, COMPID(Tag, player));
                eecs::create_entity_wrap(*nReg).set(COMPID(float, timeFromLevelChange), 0.5f);
                /*
                eecs::query_entities(*nReg, [&](eecs::EntityId eid, const vec3f& pos, const float rot, Tag spawn){
                    eecs::query_entities(*nReg, [&](eecs::EntityId pid, vec3f& position, vec3f& direction, vec3i& prevPosition, Tag player)
                    {
                        position = pos;
                        printf("pos = %f, %f, %f\n", pos.x, pos.y, pos.z);
                        direction = vec3f{sinf(rot*DEG2RAD), 0.f, cosf(rot*DEG2RAD)};
                        prevPosition = vec3i{INT_MAX, INT_MAX, INT_MAX};
                    }, COMPID(vec3f, position), COMPID(vec3f, direction), COMPID(vec3i, prevPosition), COMPID(Tag, player));
                },
                COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const Tag, spawn));
                */
                return nReg;
            }

            std::filesystem::path fullPath{"res/levels/"};
            fullPath += nextLevel;

            if (!fs::exists(fullPath))
                return nullptr; // this is an error

            eecs::Registry* rNew = new eecs::Registry();
            if (eecs::find_entity(reg, "DebugMarker")!=eecs::invalid_eid)
            {
                eecs::EntityId eid = eecs::create_entity(*rNew, "DebugMarker"); // existence of this entity means we are in debug mode
            }
            restart_world(*rNew);

            load_entities_from_file(*rNew, fullPath.string().c_str());
            registries[nextLevel] = rNew;
            return rNew;
        }
        else
        {
            return &reg;
        }
    }

    return &reg;
}

void preload_levels(std::unordered_map<std::string, eecs::Registry*>& registries, float width, float height, float scaleFactor)
{
    edat::ParserSuite psuite;
    psuite.addLambdaParser<std::string>("str", [](const std::string_view& str) -> std::string { return std::string(str); });
    fs::path levels = "res/level_list.edat";
    fs::path fullPath = fs::current_path() / levels;
    edat::Table levelsTable = edat::parseFile(fullPath, psuite);
    levelsTable.getAll<std::vector<std::string>>([&](const std::string& name, const std::vector<std::string>& level_list)
    {
        for (const std::string& lname : level_list)
        {
            std::filesystem::path fullPath{"res/levels/"};
            fullPath += lname;
            if (!fs::exists(fullPath))
                continue; // this is an error
            eecs::Registry* rNew = new eecs::Registry();
            create_ui_helper(*rNew, width, height, scaleFactor);
            restart_world(*rNew);

            load_entities_from_file(*rNew, fullPath.string().c_str());
            registries[lname] = rNew;
        }
    });
}

bool ray_hit(eecs::Registry& reg, const vec3f& sourcePos, const vec3f& targetPos, eecs::EntityId target)
{
    bool res = false;
    vec3f dir = targetPos - sourcePos;
    const float initialDist = dir.mag();
    const float maxDistSq = sqr(initialDist + 1.f);
    dir = (1.f / initialDist) * dir;
    Ray r = {toRLVec3(sourcePos), toRLVec3(dir)};
    float bestT = 1e12f;
    eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, const Model& model)
    {
        if ((sourcePos - position).mag2() > maxDistSq || eid == target || bestT < initialDist)
            return;
        if (eecs::has_comp(reg, eid, COMPID(Tag, rayTransparent)))
            return;
        Matrix matRotation = MatrixRotate(Vector3{0.f, 1.f, 0.f}, eecs::get_comp_or(reg, eid, COMPID(float, rotation),0.f));
        Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

        Matrix matTransform = MatrixMultiply(matRotation, matTranslation);
        RayCollision coll = GetRayCollisionMesh(r, model.meshes[0], matTransform);
        if (!coll.hit)
            return;
        if (coll.distance < bestT)
            bestT = coll.distance;
    }, COMPID(const vec3f, position), COMPID(const Model, model));
    res = bestT < initialDist;
    return res;
}

bool check_occupancy(eecs::Registry& reg, const vec3f& pos)
{
    bool res = false;
    vec3i pos3d = pos_to_grid3d(pos);
    eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, Tag occupiesCell)
    {
        res |= pos_to_grid3d(position) == pos3d;
    }, COMPID(const vec3f, position), COMPID(const Tag, occupiesCell));
    return res;
}

bool check_floor(eecs::Registry& reg, const vec3f& pos)
{
    bool res = false;
    eecs::query_entities(reg, [&](eecs::EntityId, const std::set<vec3i>& level_floors)
    {
        const vec3i gridPos3d = pos_to_grid3d(pos);
        res = level_floors.contains(gridPos3d);
    }, COMPID(const std::set<vec3i>, level_floors));
    return res;
}


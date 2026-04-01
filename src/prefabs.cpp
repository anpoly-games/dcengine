#include <eecs.h>
#include <edat.h>
#include <parsers.h>
#include <utility>
#include "dcengine/prefabs.h"
#include "dcengine/tags.h"
#include "dcengine/math.h"
#include "component.h"

namespace fs = std::filesystem;

std::vector<eecs::EntityId> load_entities_from_file(eecs::Registry& reg, const std::string_view& filename)
{
    std::vector<eecs::EntityId> res;
    edat::ParserSuite psuite;
    psuite.addLambdaParser<int>("int", [](const std::string_view& str) -> int { return std::stoi(std::string(str)); });
    psuite.addLambdaParser<float>("float", [](const std::string_view& str) -> float { return std::stof(std::string(str)); });
    psuite.addLambdaParser<std::string>("str", [](const std::string_view& str) -> std::string { return std::string(str); });
    psuite.addLambdaParser<Tag>("Tag", [](const std::string_view& str) -> Tag { return Tag(); });
    psuite.addLambdaParser<bool>("bool", [](const std::string_view& str) -> bool { return str == "true"; });
    psuite.addLambdaParser<eecs::EntityId>("eid", [&](const std::string_view& str) -> eecs::EntityId
    {
        return str.empty() ? eecs::invalid_eid : eecs::create_or_find_entity(reg, std::string(str).c_str());
    });
    psuite.addLambdaParser<eecs::EntityId>("eid_inst", [&](const std::string_view& str) -> eecs::EntityId
    {
        return eecs::create_entity_wrap(reg)
            .set(COMPID(std::string, prefab), std::string(str)).eid;
    });
    psuite.addLambdaParser<std::pair<eecs::EntityId, float>>("eid_float", [&](const std::string_view& str) -> std::pair<eecs::EntityId, float>
    {
        size_t separator = str.find(':');
        if (separator == std::string_view::npos)
        {
            return std::make_pair(eecs::create_or_find_entity(reg, std::string(str).c_str()), 0.f);
        }
        else
        {
            return std::make_pair(eecs::create_or_find_entity(reg, std::string(str.substr(0, separator)).c_str()), std::stof(std::string(str.substr(separator+1))));
        }
    });
    psuite.addLambdaParser<std::pair<eecs::EntityId, int>>("eid_int", [&](const std::string_view& str) -> std::pair<eecs::EntityId, int>
    {
        size_t separator = str.find(':');
        if (separator == std::string_view::npos)
        {
            return std::make_pair(eecs::create_or_find_entity(reg, std::string(str).c_str()), 0);
        }
        else
        {
            return std::make_pair(eecs::create_or_find_entity(reg, std::string(str.substr(0, separator)).c_str()), std::stoi(std::string(str.substr(separator+1))));
        }
    });

    fs::path enemies = filename;
    fs::path fullPath = fs::current_path() / enemies;
    edat::Table entitiesTable = edat::parseFile(fullPath, psuite);

    auto processPrefab = [&](eecs::EntityId eid)
    {
        eecs::query_component(reg, eid, [&](std::string& prefab)
        {
            eecs::EntityId prefabEid = eecs::find_entity(reg, prefab.c_str());
            eecs::copy_from_prefab(reg, prefabEid, eid);
        }, COMPID(std::string, prefab));
    };

    entitiesTable.getAll<edat::Table>([&](const std::string& name, const edat::Table& tbl)
    {
        eecs::EntityWrap entity = eecs::create_or_find_entity_wrap(reg, name.c_str());
        tbl.getAll<float>([&](const std::string& compName, float val)
        {
            entity.set(eecs::comp_id<float>(compName.c_str()), val);
        });
        tbl.getAll<int>([&](const std::string& compName, int val)
        {
            entity.set(eecs::comp_id<int>(compName.c_str()), val);
        });
        tbl.getAll<bool>([&](const std::string& compName, bool val)
        {
            entity.set(eecs::comp_id<bool>(compName.c_str()), val);
        });
        tbl.getAll<std::vector<float>>([&](const std::string& compName, const std::vector<float>& val)
        {
            if (val.size() == 2) // vec2f
                entity.set(eecs::comp_id<vec2f>(compName.c_str()), vec2f{val[0], val[1]});
            else if (val.size() == 3) // vec3f
                entity.set(eecs::comp_id<vec3f>(compName.c_str()), vec3f{val[0], val[1], val[2]});
            else if (val.size() == 4) // vec4f
                entity.set(eecs::comp_id<vec4f>(compName.c_str()), vec4f{val[0], val[1], val[2], val[3]});
        });
        tbl.getAll<std::vector<int>>([&](const std::string& compName, const std::vector<int>& val)
        {
            if (val.size() == 2) // vec2i
                entity.set(eecs::comp_id<vec2i>(compName.c_str()), vec2i{val[0], val[1]});
            else if (val.size() == 3) // vec3i
                entity.set(eecs::comp_id<vec3i>(compName.c_str()), vec3i{val[0], val[1], val[2]});
            else if (val.size() == 4) // vec4i
                entity.set(eecs::comp_id<vec4i>(compName.c_str()), vec4i{val[0], val[1], val[2], val[3]});
        });
        tbl.getAll<Tag>([&](const std::string& compName, Tag)
        {
            entity.tag(eecs::comp_id<Tag>(compName.c_str()));
        });
        tbl.getAll<eecs::EntityId>([&](const std::string& compName, eecs::EntityId val)
        {
            entity.set(eecs::comp_id<eecs::EntityId>(compName.c_str()), val);
            processPrefab(val);
        });
        tbl.getAll<std::vector<eecs::EntityId>>([&](const std::string& compName, const std::vector<eecs::EntityId>& val)
        {
            entity.set(eecs::comp_id<std::vector<eecs::EntityId>>(compName.c_str()), val);
            for (eecs::EntityId eid : val)
                processPrefab(eid);
        });
        tbl.getAll<std::vector<std::string>>([&](const std::string& compName, const std::vector<std::string>& val)
        {
            entity.set(eecs::comp_id<std::vector<std::string>>(compName.c_str()), val);
        });
        tbl.getAll<std::string>([&](const std::string& compName, const std::string& val)
        {
            entity.set(eecs::comp_id<std::string>(compName.c_str()), val);
        });
        tbl.getAll<std::pair<eecs::EntityId, float>>([&](const std::string& compName, const std::pair<eecs::EntityId, float>& val)
        {
            entity.set(eecs::comp_id<std::pair<eecs::EntityId, float>>(compName.c_str()), val);
        });
        tbl.getAll<std::vector<std::pair<eecs::EntityId, float>>>([&](const std::string& compName, const std::vector<std::pair<eecs::EntityId, float>>& val)
        {
            entity.set(eecs::comp_id<std::vector<std::pair<eecs::EntityId, float>>>(compName.c_str()), val);
        });
        tbl.getAll<std::pair<eecs::EntityId, int>>([&](const std::string& compName, const std::pair<eecs::EntityId, int>& val)
        {
            entity.set(eecs::comp_id<std::pair<eecs::EntityId, int>>(compName.c_str()), val);
        });
        tbl.getAll<std::vector<std::pair<eecs::EntityId, int>>>([&](const std::string& compName, const std::vector<std::pair<eecs::EntityId, int>>& val)
        {
            entity.set(eecs::comp_id<std::vector<std::pair<eecs::EntityId, int>>>(compName.c_str()), val);
        });

        processPrefab(entity.eid);
        res.push_back(entity.eid);
    });

    return res;
}

std::vector<eecs::EntityId> load_prefabs_from_file(eecs::Registry& reg, const std::string_view& filename)
{
    std::vector<eecs::EntityId> res = load_entities_from_file(reg, filename);
    for (eecs::EntityId eid : res)
        eecs::make_prefab(reg, eid);
    return res;
}


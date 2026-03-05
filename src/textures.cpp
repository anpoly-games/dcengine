#include <eecs.h>
#include <raylib.h>

#include "dcengine/textures.h"

void register_textures(eecs::Registry& reg)
{
    eecs::reg_enter(reg, [&](eecs::EntityId eid, const std::string& texture_filename)
    {
        std::string entName = TextFormat("tex:%s", texture_filename.c_str());
        eecs::EntityId texEid = eecs::find_entity(reg, entName.c_str());
        if (texEid == eecs::invalid_eid)
        {
            printf("no ent found with %s\n", entName.c_str());
            Texture2D tex = LoadTexture(texture_filename.c_str());
            eecs::create_entity_wrap(reg, entName.c_str())
                .set(COMPID(Texture2D, texture), tex);
            eecs::set_component(reg, eid, COMPID(Texture2D, texture_diff), tex);
        }
        else
        {
            eecs::query_component(reg, texEid, [&](Texture2D texture)
            {
                eecs::set_component(reg, eid, COMPID(Texture2D, texture_diff), texture);
            }, COMPID(Texture2D, texture));
        }
    }, COMPID(const std::string, texture_filename));
    eecs::reg_enter(reg, [&](eecs::EntityId eid, const std::string& texture_emissiveFilename)
    {
        std::string entName = TextFormat("tex:%d", texture_emissiveFilename.c_str());
        eecs::EntityId texEid = eecs::find_entity(reg, entName.c_str());
        if (texEid == eecs::invalid_eid)
        {
            Texture2D tex = LoadTexture(texture_emissiveFilename.c_str());
            eecs::create_entity_wrap(reg, entName.c_str())
                .set(COMPID(Texture2D, texture), tex);
            eecs::set_component(reg, eid, COMPID(Texture2D, texture_emissive), tex);
        }
        else
        {
            eecs::query_component(reg, texEid, [&](Texture2D texture)
            {
                eecs::set_component(reg, eid, COMPID(Texture2D, texture_emissive), texture);
            }, COMPID(Texture2D, texture));
        }
    }, COMPID(const std::string, texture_emissiveFilename));
}


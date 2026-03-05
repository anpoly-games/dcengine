#include <eecs.h>
#include <string>

#include "dcengine/settings.h"

void create_settings(eecs::Registry& reg)
{
    eecs::create_entity_wrap(reg, "settings")
        .set(COMPID(bool, opt_instantStep), false)
        .set(COMPID(float, opt_musicVolume), 0.5f)
        .set(COMPID(float, opt_sfxVolume), 0.5f);
}


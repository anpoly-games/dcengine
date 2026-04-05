#include <raylib.h>
#include <eecs.h>
#include <string>
#include <random>

#include "dcengine/audio.h"
#include "dcengine/math.h"

void register_audio(eecs::Registry& reg)
{
    eecs::reg_enter(reg, [&](eecs::EntityId eid, const std::string& sound_filename)
    {
        std::string entName = TextFormat("snd:%s", sound_filename.c_str());
        eecs::EntityId sndEid = eecs::find_entity(reg, entName.c_str());
        if (sndEid == eecs::invalid_eid)
        {
            Sound sound = LoadSound(sound_filename.c_str());
            eecs::create_entity_wrap(reg, entName.c_str())
                .set(COMPID(Sound, sound), sound);
            eecs::set_component(reg, eid, COMPID(Sound, sound), sound);
        }
        else
        {
            eecs::query_component(reg, sndEid, [&](Sound sound)
            {
                eecs::set_component(reg, eid, COMPID(Sound, sound), sound);
            }, COMPID(Sound, sound));
        }
    }, COMPID(const std::string, sound_filename));
    eecs::reg_enter(reg, [&](eecs::EntityId eid, float audio_timeFrom, float audio_timeTo)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.f, 1.f);
        eecs::set_component(reg, eid, COMPID(float, audio_timer), float(audio_timeFrom + (audio_timeTo - audio_timeFrom) * dis(gen)));
    }, COMPID(const float, audio_timeFrom), COMPID(const float, audio_timeTo));
    eecs::reg_system(reg, [&](eecs::EntityId, float& audio_timer, float audio_timeFrom, float audio_timeTo, const Sound& sound, const vec2f& audio_volumeRange, const vec2f& audio_pitchRange)
    {
        audio_timer -= GetFrameTime();
        if (audio_timer <= 0.f)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.f, 1.f);
            SetSoundVolume(sound, dis(gen) * (audio_volumeRange.y - audio_volumeRange.x) + audio_volumeRange.x);
            SetSoundPitch(sound, dis(gen) * (audio_pitchRange.y - audio_pitchRange.x) + audio_pitchRange.x);
            SetSoundPan(sound, dis(gen) - 0.5f);
            PlaySound(sound);
            audio_timer = audio_timeFrom + (audio_timeTo - audio_timeFrom) * dis(gen);
        }
    }, COMPID(float, audio_timer), COMPID(const float, audio_timeFrom), COMPID(const float, audio_timeTo), COMPID(const Sound, sound), COMPID(const vec2f, audio_volumeRange), COMPID(const vec2f, audio_pitchRange));
}

void play_snd(eecs::Registry& reg, eecs::EntityId eid, const char* comp_name)
{
    eecs::query_component(reg, eecs::get_comp_or(reg, eid, eecs::comp_id<eecs::EntityId>(comp_name), eecs::invalid_eid), [&](Sound& sound)
    {
        PlaySound(sound);
    }, COMPID(Sound, sound));
}


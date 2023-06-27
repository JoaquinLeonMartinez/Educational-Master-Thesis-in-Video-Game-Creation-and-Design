#pragma once

#include "modules/module.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "entity/common_msgs.h"
#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "audio/audioEvent.h"

#define VEC3_TO_FMOD(VEC3) FMOD_VECTOR {VEC3.x, VEC3.y, VEC3.z};
#define FMOD_TO_VEC3(FMOD_VECTOR) VEC3(FMOD_VECTOR.x, FMOD_VECTOR.y, FMOD_VECTOR.z);

class CModuleAudio : public IModule
{
private:
    FMOD_RESULT		res;
    /* FMOD systems */
    FMOD::Studio::System* system = NULL;
    void *_extradriverdata = 0;
    /*
    FMOD::Studio::Bank* masterBank = NULL;		// ESTOS DOS SON OBLIGATORIOS, SON EL INDICE
    FMOD::Studio::Bank* stringsBank = NULL;		// ESTOS DOS SON OBLIGATORIOS, SON EL INDICE
    FMOD::Studio::Bank* weaponsBank = NULL;		// estos son los bancos "reales" con datos
    FMOD::Studio::Bank* DaniBank = NULL;
    */

    /* FMOD banks, events, eventinstances and buses */
    std::unordered_map<std::string, FMOD::Studio::Bank*> myBanks;
    std::unordered_map<std::string, FMOD::Studio::EventDescription*> myEvents;
    std::unordered_map<unsigned int, FMOD::Studio::EventInstance*> myEventInstances;
    std::unordered_map<unsigned int, FMOD::Studio::EventInstance*> myPreloadedEventInstances;
    std::unordered_map<std::string, FMOD::Studio::Bus*> myBuses;
    std::unordered_map<std::string, float> nonRepeatableEvents;

    std::unordered_map<std::string, AudioEvent> preloadedSoundEventObjects;

    /* ID for event instances (always incremental) */
    static unsigned int sNextID;

    CHandle h_listener;

    //void registerAllSoundClipsInPath(char* path);
    const std::string getPlayingState(FMOD::Studio::EventInstance* ei);

protected:
    friend class AudioEvent;
    FMOD::Studio::EventInstance* getEventInstance(unsigned int id);
    bool playPreloadedEventInstance(unsigned int id);

public:
    /*
    0.- Pause
    1.- MainMenu
    2.- SlowTrack
    3.- MainTrack
    4.- ScaryTrack
    5.- CombatTrack
    */
    AudioEvent soundtrack;
    AudioEvent secondarySoundtrack;
    AudioEvent announcement;
    CModuleAudio(const std::string& name) : IModule(name) {}
    bool start() override;
    void update(float delta) override;
    void renderInMenu();
    void stop() override;

    void loadBank(const std::string& name);
    void unloadBank(const std::string& name);
    void unloadAllBanks();

    AudioEvent playEvent(const std::string& name);
    AudioEvent playEventNoRepeat(const std::string& name);
    AudioEvent preloadEvent(const std::string& name);

    CHandle getListener() { return h_listener; };
    void setListener(CHandle h_listener /*const CTransform& transform*/);

    float getBusVolume(const std::string& name) const;
    bool getBusPaused(const std::string& name) const;
    void setBusVolume(const std::string& name, float volume);
    void setBusPaused(const std::string& name, bool pause);

    CTransform getVirtual3DAttributes(const CTransform& soundTransform, CHandle reference = getEntityByName("The Player"));

};
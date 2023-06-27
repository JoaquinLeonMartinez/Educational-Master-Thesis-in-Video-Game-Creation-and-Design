#include "mcv_platform.h"
#include "audioEvent.h"
#include "engine.h"
#include "fmod.hpp"
#include "fmod_studio.hpp"
AudioEvent::AudioEvent() {
    myID = 0;
}

AudioEvent::AudioEvent(unsigned int id) {
    myID = id;
}

AudioEvent::AudioEvent(unsigned int id, bool being_preloaded) {
    myID = id;
    isBeingPreloaded = being_preloaded;
}

bool AudioEvent::isValid() {
    return EngineAudio.getEventInstance(myID) != nullptr;
}

void AudioEvent::restart() {
    if (!isBeingPreloaded) {
        auto event = EngineAudio.getEventInstance(myID);
        if (event) {
            event->start();
        }
    }
    else {
        isBeingPreloaded = !EngineAudio.playPreloadedEventInstance(myID);
    }
}

void AudioEvent::stop(bool allowFadeOut) {
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        FMOD_STUDIO_STOP_MODE mode = allowFadeOut ? 
            FMOD_STUDIO_STOP_ALLOWFADEOUT :
            FMOD_STUDIO_STOP_IMMEDIATE;
        event->stop(mode);
    }
}

void AudioEvent::setPaused(bool pause) {
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        event->setPaused(pause);
    }
}

void AudioEvent::setVolume(float volume) {
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        event->setVolume(volume);
    }
}

void AudioEvent::setPitch(float pitch) {
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        event->setPitch(pitch);
    }
}

void AudioEvent::setParameter(const std::string& name, float value) {
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        event->setParameterByName(name.c_str(), value);
    }
}

void AudioEvent::setParameter(const std::string& name, int value) {
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        event->setParameterByName(name.c_str(), value);
    }
}

bool AudioEvent::getPaused() const {
    bool isPaused = false;
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        event->getPaused(&isPaused);
    }
    return isPaused;
}

float AudioEvent::getVolume() const {
    float volume = 0.f;
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        event->getVolume(&volume);
    }
    return volume;
}

int AudioEvent::getLength() const {
    int length = 0;
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        FMOD::Studio::EventDescription* description = nullptr;
        event->getDescription(&description);
        (description)->getLength(&length);
    }
    return length;
}

float AudioEvent::getPitch() const {
    float pitch = 0.f;
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        event->getPitch(&pitch);
    }
    return pitch;
}

float AudioEvent::getParameter(const std::string& name) {
    float value = 0.f;
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        event->getParameterByName(name.c_str(), &value);
    }
    return value;
}

bool AudioEvent::is3D() const {
    bool value = false;
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {

        // Get event description
        FMOD::Studio::EventDescription* ed = nullptr;
        event->getDescription(&ed);
        if (ed) {
            FMOD_RESULT result = ed->is3D(&value);
        }
    }
    return value;
}

void AudioEvent::set3DAttributes(const CTransform& worldTrans, const VEC3& vel) {
    set3DAttributes(worldTrans.getPosition(), worldTrans.getFront(), worldTrans.getUp(), vel);
}

void AudioEvent::set3DAttributes(const VEC3 & pos, const VEC3 & front, const VEC3 & up, const VEC3 & vel)
{
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        FMOD_3D_ATTRIBUTES attr;
        attr.position = VEC3_TO_FMOD(pos);
        attr.forward = VEC3_TO_FMOD(front);
        attr.up = VEC3_TO_FMOD(up);
        attr.velocity = VEC3_TO_FMOD(vel);
        event->set3DAttributes(&attr);
    }
}

CTransform AudioEvent::get3DAttributes()
{
    CTransform t;
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        FMOD_3D_ATTRIBUTES attr;
        event->get3DAttributes(&attr);
        VEC3 pos = FMOD_TO_VEC3(attr.position);
        t.setPosition(pos);
        /* TODO: do the same with rotation and scale */
    }
    return t;
}

bool AudioEvent::isPlaying()
{
    FMOD_STUDIO_PLAYBACK_STATE ei_state = FMOD_STUDIO_PLAYBACK_FORCEINT;
    auto event = EngineAudio.getEventInstance(myID);
    if (event) {
        event->getPlaybackState(&ei_state);
    }
    return ei_state == FMOD_STUDIO_PLAYBACK_PLAYING;
}

bool AudioEvent::isRelativeToCameraOnly() const
{
    return relativeToCameraOnly;
}

void AudioEvent::setIsRelativeToCameraOnly(bool relativeToCamera)
{
    relativeToCameraOnly = relativeToCamera;
}

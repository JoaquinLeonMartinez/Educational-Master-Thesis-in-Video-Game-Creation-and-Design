#include "mcv_platform.h"
#include "comp_audioPlayer.h"
#include "components/common/comp_transform.h"
#include "engine.h"

using namespace physx;

DECL_OBJ_MANAGER("comp_audioplayer", TCompAudioPlayer);

void TCompAudioPlayer::debugInMenu() {
}

void TCompAudioPlayer::load(const json& j, TEntityParseContext& ctx) {
    _audioEventString = j.value("_audioEventString", _audioEventString);
    _audioEventStringSlowVersion = j.value("_audioEventStringSlowVersion", _audioEventStringSlowVersion);
    _autoPlay = j.value("_autoPlay", _autoPlay);
    
    assert(_audioEventString != "");
    _audioEvent = EngineAudio.playEvent(_audioEventString);
    if (_audioEvent.is3D()) {
        TCompTransform* c_trans = get<TCompTransform>();
        if(c_trans)
            _audioEvent.set3DAttributes(*c_trans);
    }
    if(!_autoPlay){
        _audioEvent.stop();
    }
    if (_audioEventStringSlowVersion != "") {
        TCompTransform* c_trans = get<TCompTransform>();
        _audioEventSlow = EngineAudio.playEvent(_audioEventStringSlowVersion);
        _audioEventSlow.setPaused(true);
        if (c_trans)
            _audioEventSlow.set3DAttributes(*c_trans);
    }
}

void TCompAudioPlayer::registerMsgs() {
    DECL_MSG(TCompAudioPlayer, TMsgSoundRequest, onSoundRequest);
}

void TCompAudioPlayer::onSoundRequest(const TMsgSoundRequest& msg) {
    if (msg.name != "") {
        _audioEvent = EngineAudio.playEvent(msg.name);
    }
}

AudioEvent TCompAudioPlayer::getEvent() {
    return _audioEvent;
}

void TCompAudioPlayer::play() {
    _audioEvent = EngineAudio.playEvent(_audioEventString);
}

void TCompAudioPlayer::update(float dt) {
    if (_audioEvent.is3D()) {
        TCompTransform* c_trans = get<TCompTransform>();
        if (c_trans)
            _audioEvent.set3DAttributes(*c_trans);
    }

    if (_audioEventStringSlowVersion != "") {
        if (GameController.getTimeScale() < 1.0f && _audioEventSlow.getPaused()) {
            _audioEventSlow.setPaused(false);
            _audioEvent.setPaused(true);
        }
        else if (GameController.getTimeScale() == 1.0f && _audioEvent.getPaused()) {
            _audioEvent.setPaused(false);
            _audioEventSlow.setPaused(true);
        }

        if (_audioEventSlow.is3D()) {
            TCompTransform* c_trans = get<TCompTransform>();
            if (c_trans)
                _audioEventSlow.set3DAttributes(*c_trans);
        }
    }
}
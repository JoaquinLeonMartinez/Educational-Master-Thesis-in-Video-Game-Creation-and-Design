#pragma once

#include "components/common/comp_base.h"
#include "entity/entity.h"
#include "entity/common_msgs.h"
#include "components/common/comp_transform.h"
#include "modules/game/audio/audioEvent.h"


class TCompFireController : public TCompBase {

    DECL_SIBLING_ACCESS();
    void onSoundRequest(const TMsgSoundRequest& msg);
public:
    void update(float dt);
    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    void renderDebug();
    void attack(VEC3 origin);
    void enable();
    void disable();
    void comboAttack(VEC3 pos);
    bool isEnabled() { return _enabled; }
    bool isBuffActive();
    static void registerMsgs();

private:

    bool _enabled = false;

    //Fire Values
    float _fireWidth = 0.5f;
    float _fireHeight = 0.5f;
    float _fireDistance = 5.0f;
    float _fireDamage = 12.0f;
    float _fireImpactForce = 5.f;
    float _fireDelay = 0.1f;
    float _fireTimer = 0.f;

    std::string _hitAudioName = "event:/Character/Powers/Fire/Fire_Hit_1";
    AudioEvent _hitAudio;
    //End Fire Values

    //Spheres Values
    CHandle h_skeleton;
    //This is the current bone which has attached the weapon. We'll need to swap this with the new one.
    std::string bone_name = "Bip001 Head";
    int         bone_id = -1;      // Id of the bone of the skeleton to track
    std::string parent_name = "Player";
    //End Spheres Values

    //Buff Values
    bool _isBuffed = false;
    float _buffDuration = 4.f;
    float _buffRemaining = 0.f;
    //End Buff Values

    AudioEvent audioEffect;


};
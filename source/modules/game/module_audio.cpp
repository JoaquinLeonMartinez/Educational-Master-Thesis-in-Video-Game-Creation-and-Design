#include "mcv_platform.h"
#include "engine.h"
#include "module_audio.h"
#include "entity/common_msgs.h"
#include "entity/msgs.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"
#include "windows/app.h"
#include "components/common/comp_transform.h"
#include "components/common/comp_tags.h"

#include "fmod_errors.h"
#pragma comment(lib, "fmod_vc.lib")
#pragma comment(lib, "fmodL_vc.lib")

#pragma comment(lib, "fmodstudio_vc.lib")
#pragma comment(lib, "fmodstudioL_vc.lib")

unsigned int CModuleAudio::sNextID = 0;

bool CModuleAudio::start() {

  FMOD_RESULT result;
	// arranco FMOD
  result = FMOD::Studio::System::create(&system); // Create the Studio System object.

  assert(result == FMOD_OK);
  result = system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0);
  assert(result == FMOD_OK);
  if (result != FMOD_OK) {
      fatal("Failed to initialize FMOD system %s\n", FMOD_ErrorString(result));
      return false;
  }

  loadBank("data/audio/sm/Build/Desktop/Master.strings.bank");
  loadBank("data/audio/sm/Build/Desktop/Master.bank");
  soundtrack = EngineAudio.playEvent("event:/Music/Soundtrack");
  soundtrack.setParameter("soundtrack_id", 1);
  secondarySoundtrack = EngineAudio.playEvent("event:/Music/Soundtrack");
  secondarySoundtrack.setParameter("soundtrack_id", 0);
  secondarySoundtrack.setPaused(true);
  return true;
/*
	// un par de bancos de ejemplo    
	res = system->loadBankFile("./data/Weapons.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &weaponsBank);
	res = system->loadBankFile("./data/DaniBank.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &DaniBank);


	// Cargamos los EventDescriptors
  FMOD::Studio::EventDescription* explosionDescription = NULL;
  FMOD::Studio::EventDescription* DaniDescription = NULL;
	res = system->getEvent("event:/Explosions/Single Explosion", &explosionDescription);
	res = system->getEvent("event:/Dani/OnEnter", &DaniDescription);

	// cacheo datos de la explosion para que no haya delays
	res = explosionDescription->loadSampleData();

  FMOD::Studio::EventInstance* DaniInstance = NULL;
	res = DaniDescription->createInstance(&DaniInstance);

	
	// creo una explosión
		Studio::EventInstance* eventInstance = NULL;
		res = explosionDescription->createInstance(&eventInstance);
		eventInstance->start();
		// y la libero (esto se hará al acabar)
		eventInstance->release();

		//musiquilla out
		res = DaniInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
	*/

}

void CModuleAudio::update(float delta) {

    /* Look for finished event instances */
    std::vector<unsigned int> done;
    for (auto& iter : myEventInstances) {
        FMOD::Studio::EventInstance * e = iter.second;
        FMOD_STUDIO_PLAYBACK_STATE state;
        e->getPlaybackState(&state);
        if (state == FMOD_STUDIO_PLAYBACK_STOPPED) {
            e->release();
            done.emplace_back(iter.first);
        }
    }
    /* Remove finished instances from myEventInstances */
    for (auto id : done) {
        myEventInstances.erase(id);
    }

    /* Set listener according to active camera */
    CHandle candidate_h_listener = getEntityByName("PlayerCamera");
    if (candidate_h_listener.isValid()) {
        setListener(candidate_h_listener);
    }
	system->update();

}

void CModuleAudio::stop()
{
    unloadAllBanks();

    if (system) {
        system->release();
    }
}

void CModuleAudio::renderInMenu()
{
    if (ImGui::TreeNode("Sound")) {
        if (ImGui::TreeNode("Banks")) {
            for (auto bank : myBanks) {
                char path[512];
                bank.second->getPath(path, 512, nullptr);
                ImGui::Text(path);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Event Descriptions")) {
            int index = 0;
            for (auto ed : myEvents) {
                char path[512];
                ed.second->getPath(path, 512, nullptr);
                ImGui::Text(path);
                ImGui::SameLine();
                if (ImGui::Button(("Play " + std::to_string(index)).c_str())) {
                    playEvent(path);
                }
                index++;
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Event instances")) {
            int index = 0;
            for (auto ei : myEventInstances) {
                char path[512];
                FMOD::Studio::EventDescription *mydes;
                ei.second->getDescription(&mydes);
                mydes->getPath(path, 512, nullptr);
                ImGui::Text(path);
                ImGui::SameLine();
                ImGui::Text(" - %s", getPlayingState(ei.second).c_str());
                ImGui::SameLine();
                if (ImGui::Button(("Stop " + std::to_string(index)).c_str())) {
                    AudioEvent* test;   //TODO: Not working atm
                    FMOD_RESULT result = ei.second->getUserData((void**)&test);
                    test->stop();
                    //playEvent(path);
                }
                index++;
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Buses")) {
            /* TODO: Not working as they should */
            int index = 0;
            for (auto bus : myBuses) {
                char path[512];
                bus.second->getPath(path, 512, nullptr);
                float busVol = 0.f;
                bool busPaused = false;
                bus.second->getVolume(&busVol);
                bus.second->getPaused(&busPaused);
                ImGui::Text(path);
                ImGui::SameLine();
                ImGui::Text("Vol: %f - Paused: %s", busVol, busPaused ? "Y" : "N");
                index++;
            }
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
}

void CModuleAudio::unloadAllBanks()
{
    for (auto& iter : myBanks) {
        iter.second->unloadSampleData();
        iter.second->unload();
    }

    myBanks.clear();
    myEvents.clear();
}

const std::string CModuleAudio::getPlayingState(FMOD::Studio::EventInstance* ei)
{
    FMOD_STUDIO_PLAYBACK_STATE ei_state;
    ei->getPlaybackState(&ei_state);

    std::string name;
    switch (ei_state) {
    case FMOD_STUDIO_PLAYBACK_PLAYING:
        name = "Playing";
        break;
    case FMOD_STUDIO_PLAYBACK_SUSTAINING:
        name = "Sustaining";
        break;
    case FMOD_STUDIO_PLAYBACK_STOPPED:
        name = "Stopped";
        break;
    case FMOD_STUDIO_PLAYBACK_STARTING:
        name = "Starting";
        break;
    case FMOD_STUDIO_PLAYBACK_STOPPING:
        name = "Stopping";
        break;
    }
    return name;
}

void CModuleAudio::loadBank(const std::string & name)
{
    // Avoid loading a bank twice
    if (myBanks.find(name) != myBanks.end()) {
        return;
    }

    // Load bank
    FMOD::Studio::Bank* bank = nullptr;
    FMOD_RESULT result = system->loadBankFile(name.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
    assert(result == FMOD_OK);

    const int maxPathLength = 512;
    if (result == FMOD_OK) {

        // Add bank to mybanks
        myBanks.emplace(name, bank);
        // Load non-streaming data
        bank->loadSampleData();
        // Get number of events in this bank
        int numEvents = 0;
        bank->getEventCount(&numEvents);

        if (numEvents > 0) {

            // Get list of event descriptions in this bank
            std::vector<FMOD::Studio::EventDescription*> events(numEvents);
            bank->getEventList(events.data(), numEvents, &numEvents);
            char eventName[maxPathLength];
            for (int i = 0; i < numEvents; i++) {
                FMOD::Studio::EventDescription* e = events[i];
                //get path of the event
                e->getPath(eventName, maxPathLength, nullptr);
                myEvents.emplace(eventName, e);
            }
        }

        // Get the number of buses in this bank
        int numBuses = 0;
        bank->getBusCount(&numBuses);
        if (numBuses > 0) {

            // Get list of buses in this bank
            std::vector<FMOD::Studio::Bus*> buses(numBuses);
            bank->getBusList(buses.data(), numBuses, &numBuses);
            char busName[maxPathLength];
            for (int i = 0; i < numBuses; i++) {
                FMOD::Studio::Bus* bus = buses[i];
                bus->getPath(busName, maxPathLength, nullptr);
                myBuses.emplace(busName, bus);
            }
        }
    }
}

void CModuleAudio::unloadBank(const std::string & name)
{
    // Avoid unloading a no loaded bank
    auto iter = myBanks.find(name);
    if (iter == myBanks.end()) {
        return;
    }

    // First: Remove all events from this bank
    FMOD::Studio::Bank* bank = iter->second;
    int numEvents = 0;
    bank->getEventCount(&numEvents);
    const int maxPathLength = 512;

    if (numEvents > 0) {

        // Get event descriptions for this bank
        std::vector <FMOD::Studio::EventDescription*> events(numEvents);

        // Get list of events
        bank->getEventList(events.data(), numEvents, &numEvents);
        char eventName[maxPathLength];
        for (int i = 0; i < numEvents; i++) {
            FMOD::Studio::EventDescription* e = events[i];
            e->getPath(eventName, maxPathLength, nullptr);
            auto i_event = myEvents.find(eventName);
            if (i_event != myEvents.end()) {

                // Remove event
                myEvents.erase(i_event);
            }
        }
    }

    // Get the number of buses in this bank
    int numBuses = 0;
    bank->getBusCount(&numBuses);
    if (numBuses > 0) {

        // Get list of buses in this bank
        std::vector<FMOD::Studio::Bus*> buses(numBuses);
        bank->getBusList(buses.data(), numBuses, &numBuses);
        char busName[maxPathLength];
        for (int i = 0; i < numBuses; i++) {
            FMOD::Studio::Bus* bus = buses[i];
            bus->getPath(busName, maxPathLength, nullptr);
            auto i_bus = myBuses.find(busName);
            if (i_bus != myBuses.end()) {
                myBuses.erase(i_bus);
            }
        }
    }

    // Unload sample data and bank
    bank->unloadSampleData();
    bank->unload();

    // Remove from banks map
    myBanks.erase(iter);
}

AudioEvent CModuleAudio::playEvent(const std::string & name)
{
    unsigned int retID = 0;
    auto iter = myEvents.find(name);
    AudioEvent soundEvent;
    if (iter != myEvents.end()) {

        /* Create instance of an event */
        FMOD::Studio::EventInstance* event = nullptr;
        iter->second->createInstance(&event);
        if (event) {

            /* Start the event instance */
            event->start();

            /* Get the next id and add it to map */
            sNextID++;
            retID = sNextID;
            myEventInstances.emplace(retID, event);
        }
        soundEvent = AudioEvent(retID);
        event->setUserData(&soundEvent);
    }
    else {
        soundEvent = AudioEvent(retID);
    }
    return soundEvent;
}

AudioEvent CModuleAudio::preloadEvent(const std::string & name)
{
    unsigned int retID = 0;
    auto iter = myEvents.find(name);
    AudioEvent soundEvent;
    if (iter != myEvents.end()) {

        /* Create instance of an event */
        FMOD::Studio::EventInstance* event = nullptr;
        iter->second->createInstance(&event);
        if (event) {

            /* Get the next id and add it to map */
            sNextID++;
            retID = sNextID;
            myPreloadedEventInstances.emplace(retID, event);
        }

        soundEvent = AudioEvent(retID, true);
        event->setUserData(&soundEvent);
    }
    else {
        soundEvent = AudioEvent(retID, true);
    }
    return soundEvent;
}

void CModuleAudio::setListener(CHandle h_listener /*const CTransform & transform*/)
{
    FMOD_3D_ATTRIBUTES attr;

    this->h_listener = h_listener;
    CEntity* e_listener = h_listener;
    TCompTransform* listener_pos = e_listener->get<TCompTransform>();
    VEC3 pos = listener_pos->getPosition();
    VEC3 fwd = listener_pos->getFront();
    attr.position = VEC3_TO_FMOD(listener_pos->getPosition());
    attr.forward = VEC3_TO_FMOD(listener_pos->getFront());
    attr.up = VEC3_TO_FMOD(listener_pos->getUp());
    attr.velocity = VEC3_TO_FMOD(VEC3::Zero);
    system->setListenerAttributes(0, &attr);
}

float CModuleAudio::getBusVolume(const std::string & name) const
{
    float volume = 0.f;
    const auto iter = myBuses.find(name);
    if (iter != myBuses.end()) {
        iter->second->getVolume(&volume);
    }
    return volume;
}

bool CModuleAudio::getBusPaused(const std::string & name) const
{
    bool paused = false;
    const auto iter = myBuses.find(name);
    if (iter != myBuses.end()) {
        iter->second->getPaused(&paused);
    }
    return paused;
}

void CModuleAudio::setBusVolume(const std::string & name, float volume)
{
    const auto iter = myBuses.find(name);
    if (iter != myBuses.end()) {
        iter->second->setVolume(volume);
    }
}

void CModuleAudio::setBusPaused(const std::string & name, bool pause)
{
    const auto iter = myBuses.find(name);
    if (iter != myBuses.end()) {
        iter->second->setPaused(pause);
    }
}


CTransform CModuleAudio::getVirtual3DAttributes(const CTransform& soundTransform, CHandle reference)
{
    if (h_listener.isValid()) {
        CEntity* e_listener = getListener();
        CEntity* e_reference = reference;
        TCompTransform* ppos = e_reference->get<TCompTransform>();
        TCompTransform* listenerPos = e_listener->get<TCompTransform>();

        float distance = VEC3::Distance(ppos->getPosition(), soundTransform.getPosition());
        VEC3 direction = (soundTransform.getPosition() - listenerPos->getPosition());
        direction.Normalize();
        VEC3 virtualPos = listenerPos->getPosition() + direction * distance;

        CTransform newTransform;
        newTransform.setPosition(virtualPos);
        newTransform.setRotation(soundTransform.getRotation());
        return newTransform;
    }
    else {
        return soundTransform;
    }
}

FMOD::Studio::EventInstance * CModuleAudio::getEventInstance(unsigned int id)
{
    FMOD::Studio::EventInstance* event = nullptr;
    auto iter = myEventInstances.find(id);
    if (iter != myEventInstances.end())
    {
        event = iter->second;
    }
    return event;
}

bool CModuleAudio::playPreloadedEventInstance(unsigned int id)
{
    auto iter = myPreloadedEventInstances.find(id);
    if (iter != myPreloadedEventInstances.end()) {
        iter->second->start();
        myEventInstances.emplace(id, iter->second);
        myPreloadedEventInstances.erase(id);
        return true;
    }
    return false;
}
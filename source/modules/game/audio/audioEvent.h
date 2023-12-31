#pragma once
class AudioEvent {

private:

    unsigned int myID;
    bool relativeToCameraOnly = false;
    bool isBeingPreloaded = false;

protected:

    friend class CModuleAudio;
    AudioEvent(unsigned int id);
    AudioEvent(unsigned int id, bool being_preloaded);

public:

    AudioEvent();

    bool isValid();

    void restart();

    void stop(bool allowFadeOut = true);

    void setPaused(bool pause);
    void setVolume(float volume);
    void setPitch(float pitch);
    void setParameter(const std::string& name, float value);
    void setParameter(const std::string& name, int value);

    bool getPaused() const;
    float getVolume() const;
    int getLength() const;
    float getPitch() const;
    float getParameter(const std::string& name);

    bool is3D() const;
    void set3DAttributes(const CTransform& transform, const VEC3& vel = VEC3::Zero);
    void set3DAttributes(const VEC3& pos, const VEC3& front, const VEC3& up, const VEC3& vel = VEC3::Zero);
    CTransform get3DAttributes();

    bool isPlaying();

    bool getIsBeingPreloaded() { return isBeingPreloaded; }
    void setIsBeingPreloaded(bool isBeingPreloaed) { this->isBeingPreloaded = isBeingPreloaded; }

    bool isRelativeToCameraOnly() const;
    void setIsRelativeToCameraOnly(bool relativeToCameraOnly);
};
#pragma once
#include <string>
#include <map>


// telling the compiler these structs exist so we don't have to include the whole library here
struct ma_engine;
struct ma_sound;

class AudioManager {
public:
    // getting the single global instance of the audio manager
    static AudioManager& getInstance();

    // starting up the audio engine
    void init();
    // shutting everything down and freeing memory
    void cleanup();


    // playing a sound once (fire and forget), good for effects like footsteps or gunshots
    void playOneShot(const std::string& filepath);


    // playing a sound that interrupts others or needs to be tracked (like dialogue or music)
    void playExclusive(const std::string& filepath);
    // stopping the currently playing exclusive sound
    void stopExclusive();

private:
    // private constructor so no one else can create a new instance
    AudioManager();
    ~AudioManager();
    // making sure we can't copy this manager, there can be only one
    AudioManager(const AudioManager&) = delete;
    void operator=(const AudioManager&) = delete;

    // the main audio engine pointer
    ma_engine* m_engine;
    // pointer to the sound currently playing in exclusive mode
    ma_sound* m_currentSound;
    // checking if the engine is actually running
    bool m_isInitialized;
    // checking if a track is currently playing
    bool m_isExclusivePlaying;
};
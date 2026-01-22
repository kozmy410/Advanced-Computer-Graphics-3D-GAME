#include "AudioManager.hpp"
#include <iostream>

// this definition is required by the library. it tells the compiler to actually
// build the miniaudio code in this file (otherwise it just looks like a header)
#define MINIAUDIO_IMPLEMENTATION
#include "dependencies/miniaudio.h"

// standard singleton pattern: we only want one audio manager for the whole game,
// so we create it once here and everyone else just grabs this reference.
AudioManager& AudioManager::getInstance() {
    static AudioManager instance;
    return instance;
}

// constructor: allocating memory for the main engine and the sound player wrapper
AudioManager::AudioManager() : m_engine(new ma_engine()), m_currentSound(new ma_sound()), m_isInitialized(false), m_isExclusivePlaying(false) {}

// destructor: making sure we shut everything down cleanly when the game closes
AudioManager::~AudioManager() {
    cleanup();
    delete m_engine; // freeing the memory
    delete m_currentSound;
}

void AudioManager::init() {
    // trying to start the miniaudio engine. passing nullptr uses default settings.
    if (ma_engine_init(nullptr, m_engine) != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio engine." << std::endl;
        return;
    }
    m_isInitialized = true; // flagging that we are ready to play sounds
}

void AudioManager::cleanup() {
    if (m_isInitialized) {
        // if music is playing, stop it first
        if (m_isExclusivePlaying) stopExclusive();

        // shutting down the main engine
        ma_engine_uninit(m_engine);
        m_isInitialized = false;
    }
}

// "fire and forget" - good for sound effects like gunshots or footsteps
// where we don't need to pause/stop/control them after they start. (ambient sounds, etc.)
void AudioManager::playOneShot(const std::string& filepath) {
    if (!m_isInitialized) return;
    ma_engine_play_sound(m_engine, filepath.c_str(), nullptr);
}

// this is for background music or dialogue where we only want one track playing at a time.
// it automatically stops whatever was playing before starting the new one.
void AudioManager::playExclusive(const std::string& filepath) {
    if (!m_isInitialized) return;

    // stopping whatever was playing before so we don't have overlapping music
    stopExclusive();

    // attempting to load the new file into our 'currentSound' player
    if (ma_sound_init_from_file(m_engine, filepath.c_str(), 0, nullptr, nullptr, m_currentSound) == MA_SUCCESS) {
        ma_sound_start(m_currentSound); // starting playback
        m_isExclusivePlaying = true;
    }
    else {
        std::cerr << "Failed to load exclusive sound: " << filepath << std::endl;
    }
}

void AudioManager::stopExclusive() {
    if (m_isInitialized && m_isExclusivePlaying) {
        // halts the music
        ma_sound_stop(m_currentSound);

        // frees the specific sound file from memory so we can load a different one later
        ma_sound_uninit(m_currentSound);
        m_isExclusivePlaying = false;
    }
}
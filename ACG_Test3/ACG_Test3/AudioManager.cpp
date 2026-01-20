#include "AudioManager.hpp"
#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include "dependencies/miniaudio.h"

AudioManager& AudioManager::getInstance() {
    static AudioManager instance;
    return instance;
}

AudioManager::AudioManager() : m_engine(new ma_engine()), m_currentSound(new ma_sound()), m_isInitialized(false), m_isExclusivePlaying(false) {}

AudioManager::~AudioManager() {
    cleanup();
    delete m_engine;
    delete m_currentSound;
}

void AudioManager::init() {
    if (ma_engine_init(nullptr, m_engine) != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio engine." << std::endl;
        return;
    }
    m_isInitialized = true;
}

void AudioManager::cleanup() {
    if (m_isInitialized) {
        if (m_isExclusivePlaying) stopExclusive();
        ma_engine_uninit(m_engine);
        m_isInitialized = false;
    }
}

void AudioManager::playOneShot(const std::string& filepath) {
    if (!m_isInitialized) return;
    ma_engine_play_sound(m_engine, filepath.c_str(), nullptr);
}

void AudioManager::playExclusive(const std::string& filepath) {
    if (!m_isInitialized) return;

    
    stopExclusive();

    
    if (ma_sound_init_from_file(m_engine, filepath.c_str(), 0, nullptr, nullptr, m_currentSound) == MA_SUCCESS) {
        ma_sound_start(m_currentSound);
        m_isExclusivePlaying = true;
    }
    else {
        std::cerr << "Failed to load exclusive sound: " << filepath << std::endl;
    }
}

void AudioManager::stopExclusive() {
    if (m_isInitialized && m_isExclusivePlaying) {
        
        ma_sound_stop(m_currentSound);
        
        ma_sound_uninit(m_currentSound);
        m_isExclusivePlaying = false;
    }
}
#include "AudioEngine.hpp"
#include <iostream>

AudioEngine::AudioEngine() : m_device(nullptr), m_context(nullptr) {}

AudioEngine::~AudioEngine() { shutdown(); }

AudioEngine& AudioEngine::getInstance() {
    static AudioEngine instance;
    return instance;
}

bool AudioEngine::init() {
    // Open the default audio device
    m_device = alcOpenDevice(NULL);
    if (!m_device) {
        std::cerr << "Failed to open OpenAL device.\n";
        return false;
    }

    // Create a context
    m_context = alcCreateContext(m_device, NULL);
    if (!m_context) {
        std::cerr << "Failed to create OpenAL context.\n";
        alcCloseDevice(m_device);
        return false;
    }

    // Make the context current
    if (!alcMakeContextCurrent(m_context)) {
        std::cerr << "Failed to make OpenAL context current.\n";
        return false;
    }
    std::cout << "OpenAL Initialized.\n";
    return true;
}

void AudioEngine::shutdown() {
    // Clean up all sources and buffers
    for (auto const& [key, source] : m_sources) {
        alDeleteSources(1, &source);
        alDeleteBuffers(1, &m_buffers.at(key));
    }
    m_sources.clear();
    m_buffers.clear();

    if (m_context) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(m_context);
    }
    if (m_device) {
        alcCloseDevice(m_device);
    }
    std::cout << "OpenAL Shutdown.\n";
}

void AudioEngine::loadSound(const std::string& key, const std::string& filePath) {
    WavData data = loadWavFile(filePath);
    if (data.data.empty()) {
        std::cerr << "Failed to load WAV data for: " << filePath << std::endl;
        return;
    }

    ALuint buffer, source;
    alGenBuffers(1, &buffer);
    alGenSources(1, &source);

    // Load data into the buffer
    alBufferData(buffer, data.format, data.data.data(), data.size, data.freq);

    // Attach buffer to source
    alSourcei(source, AL_BUFFER, buffer);

    m_buffers[key] = buffer;
    m_sources[key] = source;
    std::cout << "Loaded sound: " << key << std::endl;
}

void AudioEngine::playSound(const std::string& key, bool loop) {
    if (m_sources.count(key)) {
        ALuint source = m_sources.at(key);
        alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        alSourcePlay(source);
    }
}
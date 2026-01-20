//#pragma once
//
//#include "WavLoader.hpp"
//#include <map>
//#include "dependencies/al.h"
//#include "dependencies/alc.h"
//
//class AudioEngine {
//public:
//    static AudioEngine& getInstance();
//
//    // Initializes OpenAL device and context
//    bool init();
//    void shutdown();
//
//    // Loads a WAV file and stores it in an OpenAL buffer
//    void loadSound(const std::string& key, const std::string& filePath);
//
//    // Play a loaded sound from its buffer
//    void playSound(const std::string& key, bool loop = false);
//
//private:
//    AudioEngine();
//    ~AudioEngine();
//    AudioEngine(const AudioEngine&) = delete;
//    AudioEngine& operator=(const AudioEngine&) = delete;
//
//    ALCdevice* m_device;
//    ALCcontext* m_context;
//    // Maps the sound key (string) to an OpenAL Buffer ID (ALuint)
//    std::map<std::string, ALuint> m_buffers;
//    // We'll use a single source for each sound for simplicity (no overlapping)
//    std::map<std::string, ALuint> m_sources;
//};

// --- Wrong headers, paid lib ---
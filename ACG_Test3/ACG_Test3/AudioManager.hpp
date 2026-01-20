#pragma once
#include <string>
#include <map>


struct ma_engine;
struct ma_sound;

class AudioManager {
public:
    static AudioManager& getInstance();

    void init();
    void cleanup();

    
    void playOneShot(const std::string& filepath);

    
    void playExclusive(const std::string& filepath);
    void stopExclusive();

private:
    AudioManager();
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    void operator=(const AudioManager&) = delete;

    ma_engine* m_engine;
    ma_sound* m_currentSound; 
    bool m_isInitialized;
    bool m_isExclusivePlaying;
};
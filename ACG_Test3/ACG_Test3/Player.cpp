#define GLM_ENABLE_EXPERIMENTAL
#define MINIAUDIO_IMPLEMENTATION
#include "Player.hpp"
#include "GameConstants.hpp"
#include "InputManager.hpp"
#include "dependencies/miniaudio.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>

// NOTE: Assuming your project includes these GLM headers correctly
#include "dependencies/glm-1.0.2/glm/glm.hpp"
#include "dependencies/glm-1.0.2/glm/gtx/functions.hpp"
#include "dependencies/glm-1.0.2/glm/geometric.hpp"

// --- SETUP FOR HARDCODED ZONES ---

enum class RectType {
    SOLID,
    TRIGGER
};

struct CollisionRect {
    RectType type;
    int id;
    float minX, maxX, minZ, maxZ;
};

// The trigger zones for the story encounters
static const std::vector<CollisionRect> worldRectangles = {
    // Zone 0: Initial Street Cat Encounter (First Encounter)
    { RectType::TRIGGER, 0, -8.0f, -5.0f, 14.0f, 23.0f },

    // Zone 1: House Cat Encounter (First Level)
    { RectType::TRIGGER, 1, 7.0f, 12.0f, 38.0f, 49.0f },

    // Zone 2: New Zone
    { RectType::TRIGGER, 2, 42.0f, 47.0f, 42.0f, 47.0f }
};

bool initialized = false;

// Global audio engine
static ma_engine audioEngine;
static bool audioInitialized = false;
static ma_sound currentSound;
static bool soundLoaded = false;

// Simple function to play a sound
void playSound(const char* filepath) {
    if (!audioInitialized) {
        if (ma_engine_init(nullptr, &audioEngine) == MA_SUCCESS) {
            audioInitialized = true;
        }
        else {
            return;
        }
    }
    ma_engine_play_sound(&audioEngine, filepath, nullptr);
}

// Play a sound and stop any previous tracked sound
void playSoundExclusive(const char* filepath) {
    if (!audioInitialized) {
        if (ma_engine_init(nullptr, &audioEngine) == MA_SUCCESS) {
            audioInitialized = true;
        }
        else {
            return;
        }
    }

    // Stop and cleanup previous sound
    if (soundLoaded) {
        ma_sound_stop(&currentSound);
        ma_sound_uninit(&currentSound);
        soundLoaded = false;
    }

    // Load and play new sound
    if (ma_sound_init_from_file(&audioEngine, filepath, 0, nullptr, nullptr, &currentSound) == MA_SUCCESS) {
        ma_sound_start(&currentSound);
        soundLoaded = true;
    }
}

// Stop the current tracked sound
void stopCurrentSound() {
    if (soundLoaded) {
        ma_sound_stop(&currentSound);
        ma_sound_uninit(&currentSound);
        soundLoaded = false;
    }
}

// --- IMPLEMENTATION OF PLAYER NAME INPUT ---
void Player::initializePlayerName() {
    std::cout << "\n======================================================\n";
    std::cout << "WELCOME TO WHISKERS' HOOD: A Feline Robin Hood Tale\n";
    std::cout << "======================================================\n";
    std::cout << "Before you begin your work, what is the name of this heroic orange cat?\n";
    std::cout << "Enter your character's name: ";

    // Use std::getline to allow spaces in the name
    std::getline(std::cin, m_playerName);

    if (m_playerName.empty()) {
        m_playerName = "Whiskers"; // Default name if none is entered
        std::cout << "\nNo name entered. Your hero shall be known as **Whiskers**.\n";
    }
    else {
        std::cout << "\nYour hero, **" << m_playerName << "**, is ready to prowl the streets.\n";
    }
    std::cout << "======================================================\n\n";
    initialized = true;
}

// --- CLASS IMPLEMENTATION ---

Player::Player(glm::vec3 pos, const std::string& texPath)
    : GameObject(pos, texPath) {
    scale = glm::vec3(3.0f, 1.0f, 3.0f);

    // Initialize the m_zonesEntered map in the constructor
    for (const auto& rect : worldRectangles) {
        if (rect.type == RectType::TRIGGER) {
            // Initialize all trigger zones as not yet rewarded
            m_zonesEntered[rect.id] = false;
        }
    }
}

void Player::update(float deltaTime) {
    auto& input = InputManager::getInstance();
    glm::vec3 velocity(0.0f);

    // --- Movement Input ---
    if (input.isKeyHeld(GLFW_KEY_W)) { velocity.z -= 1.0f; }
    if (input.isKeyHeld(GLFW_KEY_S)) { velocity.z += 1.0f; }
    if (input.isKeyHeld(GLFW_KEY_A)) { velocity.x -= 1.0f; }
    if (input.isKeyHeld(GLFW_KEY_D)) { velocity.x += 1.0f; }

    if (glm::length(velocity) > 0.0f) {
        velocity = glm::normalize(velocity) * m_speed * deltaTime;
    }

    // --- Collision and Movement Logic ---

    // 1. Calculate the potential new position.
    glm::vec3 newPosition = position + velocity;
    newPosition.x = glm::clamp(newPosition.x, MAP_BOUNDS_MIN, MAP_BOUNDS_MAX);
    newPosition.z = glm::clamp(newPosition.z, MAP_BOUNDS_MIN, MAP_BOUNDS_MAX);

    // 2. Check for SOLID collisions (logic from original code, now always false)
    bool isCollision = false;
    for (const auto& rect : worldRectangles) {
        if (rect.type == RectType::SOLID) {
            if (newPosition.x >= rect.minX && newPosition.x <= rect.maxX &&
                newPosition.z >= rect.minZ && newPosition.z <= rect.maxZ) {
                isCollision = true;
                break;
            }
        }
    }

    if (!isCollision) {
        position = newPosition;
    }

    std::vector<int> currentZonesInside;

    // --- MODIFIED TRIGGER DETECTION AND REWARD/DIALOGUE LOGIC ---
    for (const auto& rect : worldRectangles) {
        if (rect.type == RectType::TRIGGER) {

            // Check if the player is currently inside the zone
            if (position.x >= rect.minX && position.x <= rect.maxX &&
                position.z >= rect.minZ && position.z <= rect.maxZ) {

                currentZonesInside.push_back(rect.id);

                // Check if this zone has NOT been processed yet
                if (!m_zonesEntered.at(rect.id)) {

                    std::cout << "\n------------------------------------------------------\n";

                    if (rect.id == 2) {
                        playSoundExclusive("resources/easteregg.mp3");
                    }
                    else
                    if (rect.id == 1) {
                        playSoundExclusive("resources/intro.wav");
                        if (!initialized) {
                           
                            initializePlayerName();

                            // daca nu eu atunci cine sound here
                           
                        }
                        
                        // ZONE 0: Full Story Introduction
                        std::cout << "**The Turning Point**\n";
                        std::cout << "In a rough little neighborhood, **" << m_playerName << "** watched the contrast: fancy indoor cats versus struggling strays.\n";
                        std::cout << "One evening, you look over at a toppled trash bag...\n";

                        // --- NEW KITTEN DETAIL ---
                        std::cout << "You see a pair of **skinny kittens** frantically searching the refuse for a meager meal.\n";
                        std::cout << "Kitten: 'Are you... the orange cat they whisper about? **" << m_playerName << "**?'\n";
                        std::cout << "Your courage grows from their recognition.\n";
                        std::cout << "This sight ignites a fierce determination. You can't just stand there anymore.\n";
                        std::cout << "It's time to start working for the hood.\n";
                        // -------------------------

                        // Set the flag to true to prevent this long intro from repeating.
                        m_zonesEntered.at(rect.id) = true;
                        std::cout << "------------------------------------------------------\n";
                    }
                    else if (rect.id == 0) {
                        stopCurrentSound();
                        playSoundExclusive("resources/lvl1.mp3");
                        // ZONE 1: First Combined Mission/Encounter (Gives Reward)
                        std::cout << "**First Heist: Salmon and a Smile**\n";

                        // Dialogue: The House Cat and the Kittens
                        std::cout << "You move like a shadow, slipping past a cracked door and into a fancy kitchen.\n";
                        std::cout << "A giant, fluffy Persian cat glares at you from a velvet pillow.\n";
                        std::cout << "Persian Cat: 'How DARE you, stray! This salmon is MINE!'\n";
                        std::cout << m_playerName << " swipes the salmon and quickly leaves, dropping it near the kittens.\n";
                        std::cout << "Kitten: 'You did it! You're the mysterious orange hero, " << m_playerName << "!'\n";

                        // Give the reward for entering the zone
                        m_score += 1000;
                        m_xp += 600;

                        // Set the flag for this zone to true to prevent repeat rewards
                        m_zonesEntered.at(rect.id) = true;

                        // Message and Stats
                        std::cout << "\n**[REWARD]** First Mission Complete: +1000 Score, +600 XP.\n";
                        std::cout << "-> Current Score: " << m_score
                            << ", Current XP: " << m_xp << std::endl;
                        std::cout << "------------------------------------------------------\n";
                    }
                }
            }
        }
    }

    // --- ZONE EXIT LOGIC: Reset the flag when the player leaves a zone ---
    // This allows the player to re-enter for another reward/dialogue
    for (auto const& [id, rewarded] : m_zonesEntered) {
        bool isCurrentlyInside = false;

        // Check if the current zone ID is in the list of zones the player is inside
        for (int currentID : currentZonesInside) {
            if (currentID == id) {
                isCurrentlyInside = true;
                break;
            }
        }

        // Only reset the flag for the REWARD ZONE (ID 1)
        if (rewarded && !isCurrentlyInside && id == 1) {
            // Player has left the rewarded zone, reset the entry flag
            m_zonesEntered.at(id) = false;
            std::cout << "[ZONE EXIT] Left Zone ID " << id << ". Reward is now available again." << std::endl;
        }
    }

    m_isInTriggerZone = !currentZonesInside.empty();
}
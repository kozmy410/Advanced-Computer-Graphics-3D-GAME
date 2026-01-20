#include <iostream>
#include "Game.hpp"

int main() {
    try {
        
        Game game("Minimap Project", 800, 600);

        
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

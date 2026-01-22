#include <iostream>
#include "Game.hpp"


//should be obvious: this is where the program starts running
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


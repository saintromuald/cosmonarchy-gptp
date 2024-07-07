#include "game.h"

void QuickCasting::tick() {
    if (this->timeout && !g_is_targeting) {
        this->timeout--;
    }
    if (this->building_timeout && !g_is_placing_building) {
        this->building_timeout--;
    }
}

Game::Game() {
}

void Game::tick() {
    this->quick_casting.tick();
}
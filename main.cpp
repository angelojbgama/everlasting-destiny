#include "Game.h"

Game* game = nullptr;

const int FPS = 60;
const int frameDelay = 1000 / FPS;

int main(int argc, char* argv[]) {
    game = new Game();
    game->init("Everlasting Destiny", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 640, false);

    Uint32 frameStart;
    int frameTime;

    while (game->running()) {
        frameStart = SDL_GetTicks();

        game->handleEvents();
        game->update();
        game->render();

        frameTime = SDL_GetTicks() - frameStart;

        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    game->clean();
    delete game;
    return 0;
}

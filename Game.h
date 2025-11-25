#ifndef GAME_H
#define GAME_H

#include <vector>
#include <memory>
#include <string>
#include <SDL2/SDL.h>
#include "Map.h"
#include "Entity.h"
#include "TurnManager.h"
#include "Dice.h"
#include "UIManager.h"
#include "GameState.h"
#include "GameDataLoader.h"
#include "Mission.h"
#include "EventLog.h"
#include "CombatSystem.h"

class Game {
public:
    Game();
    ~Game();

    bool init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
    void handleEvents();
    void update();
    void render();
    void clean();

    bool running() { return isRunning; }

private:
    void startPlayerTurn(Entity* entity);
    void startEnemyTurn(Entity* entity);
    void endCurrentTurn();
    void evaluateMissions();
    void handleBoardClick(int cellX, int cellY);
    void setCurrentAction(UIActionType action);
    void updateHighlights();
    void updateHoverInfo(int mouseX, int mouseY);
    void processEnemyTurn();
    std::vector<SDL_Point> calculateReachableCells(const Entity& entity, int ap);
    std::vector<std::vector<int>> calculateMovementCost(const Entity& entity, int ap);
    std::vector<SDL_Point> calculateRange(const Entity& entity, int distance) const;
    Entity* getEntityAt(int x, int y);
    bool isTileOccupied(int x, int y) const;
    void applyTileEffect(Entity& entity);
    std::string buildHoverText(int cellX, int cellY) const;
    void refreshAbilityButtons(const Entity* entity);
    const AbilityDefinition* getAbilityDefinition(const std::string& id) const;
    std::string serializeState() const;

    bool isRunning;
    SDL_Window* window;
    SDL_Renderer* renderer;
    Map map;
    std::vector<std::unique_ptr<PlayerEntity>> players;
    std::vector<std::unique_ptr<EnemyEntity>> enemies;
    std::vector<std::unique_ptr<NpcEntity>> npcs;
    std::vector<Entity*> initiativeOrder;
    TurnManager turnManager;
    Dice dice;
    UIManager* uiManager;
    GameState gameState;
    GameDataLoader dataLoader;
    GameContent content;
    MapDefinition currentMap;
    Mission mission;
    EventLog eventLog;
    CombatSystem* combatSystem;

    UIActionType currentAction;
    int selectedAbilityIndex;
    std::string selectedAbilityId;
    std::vector<SDL_Point> movementHighlights;
    std::vector<std::vector<int>> movementCosts;
    std::vector<SDL_Point> attackHighlights;
    std::vector<SDL_Point> abilityHighlights;
    std::string hoverText;
    bool enemyTurnPrepared;
    bool waitingForRoll;
    bool gameOverDisplayed;
    int lastRoundRecorded;

    int boardPixelWidth;
    int boardPixelHeight;
};

#endif // GAME_H

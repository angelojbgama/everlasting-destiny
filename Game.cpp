#include "Game.h"
#include <iostream>
#include <queue>
#include <sstream>
#include <cmath>

namespace {
const int kTileSize = 32;
const int kSidebarWidth = 320;
const int kAttackCost = 2;

bool containsCell(const std::vector<SDL_Point>& cells, int x, int y) {
    for (const auto& cell : cells) {
        if (cell.x == x && cell.y == y) {
            return true;
        }
    }
    return false;
}

SDL_Color factionColor(const Entity& entity) {
    switch (entity.getFaction()) {
    case EntityFaction::Players: return SDL_Color{20, 200, 255, 255};
    case EntityFaction::Enemies: return SDL_Color{200, 60, 60, 255};
    case EntityFaction::Neutral: return SDL_Color{200, 200, 0, 255};
    }
    return SDL_Color{255, 255, 255, 255};
}
}

Game::Game()
    : isRunning(false),
      window(nullptr),
      renderer(nullptr),
      uiManager(nullptr),
      gameState(GameState::AwaitingRoll),
      combatSystem(nullptr),
      currentAction(UIActionType::None),
      selectedAbilityIndex(-1),
      enemyTurnPrepared(false),
      waitingForRoll(true),
      gameOverDisplayed(false),
      lastRoundRecorded(1),
      boardPixelWidth(640),
      boardPixelHeight(640) {}

Game::~Game() {}

bool Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen) {
    int flags = 0;
    if (fullscreen) flags = SDL_WINDOW_FULLSCREEN;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF init failed: " << TTF_GetError() << std::endl;
        return false;
    }

    if (!dataLoader.loadFromFile("data/game_data.json")) {
        return false;
    }
    content = dataLoader.getContent();
    if (content.maps.empty()) {
        std::cerr << "Nenhum mapa encontrado nos dados." << std::endl;
        return false;
    }
    currentMap = content.maps.begin()->second;
    boardPixelWidth = currentMap.width * kTileSize;
    boardPixelHeight = currentMap.height * kTileSize;

    window = SDL_CreateWindow(title, xpos, ypos, boardPixelWidth + kSidebarWidth, boardPixelHeight, flags);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    map.loadFromDefinition(currentMap, content.terrainTypes);
    mission = Mission(currentMap.objectives);
    combatSystem = new CombatSystem(&dice);
    uiManager = new UIManager(boardPixelWidth, 0, kSidebarWidth, boardPixelHeight);

    for (size_t i = 0; i < currentMap.playerIds.size() && i < currentMap.playerSpawns.size(); ++i) {
        const auto& id = currentMap.playerIds[i];
        auto entityIt = content.entities.find(id);
        if (entityIt == content.entities.end()) continue;
        players.push_back(std::unique_ptr<PlayerEntity>(new PlayerEntity(entityIt->second, currentMap.playerSpawns[i])));
        initiativeOrder.push_back(players.back().get());
    }

    for (size_t i = 0; i < currentMap.enemyIds.size() && i < currentMap.enemySpawns.size(); ++i) {
        const auto& id = currentMap.enemyIds[i];
        auto entityIt = content.entities.find(id);
        if (entityIt == content.entities.end()) continue;
        enemies.push_back(std::unique_ptr<EnemyEntity>(new EnemyEntity(entityIt->second, currentMap.enemySpawns[i])));
        initiativeOrder.push_back(enemies.back().get());
    }

    for (size_t i = 0; i < currentMap.npcIds.size() && i < currentMap.npcSpawns.size(); ++i) {
        const auto& id = currentMap.npcIds[i];
        auto entityIt = content.entities.find(id);
        if (entityIt == content.entities.end()) continue;
        npcs.push_back(std::unique_ptr<NpcEntity>(new NpcEntity(entityIt->second, currentMap.npcSpawns[i])));
    }

    turnManager.setParticipants(initiativeOrder);
    Entity* initial = turnManager.getCurrent();
    refreshAbilityButtons(initial);
    if (initial && initial->getFaction() == EntityFaction::Enemies) {
        startEnemyTurn(initial);
    } else {
        startPlayerTurn(initial);
    }
    eventLog.addEntry("Missao iniciada: " + currentMap.name);

    isRunning = true;
    hoverText = "Passe o mouse sobre o tabuleiro";
    return true;
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        }
        uiManager->handleEvent(event);
        if (event.type == SDL_MOUSEMOTION) {
            updateHoverInfo(event.motion.x, event.motion.y);
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            if (event.button.x < boardPixelWidth && gameState != GameState::EnemyTurn) {
                int cellX = event.button.x / kTileSize;
                int cellY = event.button.y / kTileSize;
                handleBoardClick(cellX, cellY);
            }
        }
    }

    if (!isRunning) return;

    if (uiManager->consumeRollRequest() && gameState == GameState::AwaitingRoll) {
        Entity* entity = turnManager.getCurrent();
        if (entity) {
            int roll = dice.roll(6) + std::max(0, entity->getAttributes().agility / 2);
            entity->setActionPoints(roll);
            waitingForRoll = false;
            gameState = (entity->getFaction() == EntityFaction::Enemies) ? GameState::EnemyTurn : GameState::ActionSelection;
            eventLog.addEntry(entity->getName() + " recebeu " + std::to_string(roll) + " AP");
            updateHighlights();
        }
    }

    if (uiManager->consumeEndTurnRequest() && gameState != GameState::GameOver) {
        endCurrentTurn();
    }

    UIActionType request = uiManager->consumeActionRequest();
    if (request != UIActionType::None) {
        setCurrentAction(request);
    }

    int abilityIndex = uiManager->consumeAbilitySelection();
    if (abilityIndex >= 0) {
        selectedAbilityIndex = abilityIndex;
        Entity* entity = turnManager.getCurrent();
        if (entity && selectedAbilityIndex < static_cast<int>(entity->getAbilityIds().size())) {
            selectedAbilityId = entity->getAbilityIds()[selectedAbilityIndex];
            setCurrentAction(UIActionType::Ability);
        }
    }
}

void Game::update() {
    for (auto& player : players) player->update();
    for (auto& enemy : enemies) enemy->update();
    for (auto& npc : npcs) npc->update();

    if (gameState == GameState::EnemyTurn) {
        processEnemyTurn();
    }

    evaluateMissions();
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
    SDL_RenderClear(renderer);

    map.drawMap(renderer, true);
    if (!movementHighlights.empty()) {
        map.drawHighlights(renderer, movementHighlights, SDL_Color{255, 255, 0, 80});
    }
    if (!attackHighlights.empty()) {
        map.drawHighlights(renderer, attackHighlights, SDL_Color{255, 80, 80, 80});
    }
    if (!abilityHighlights.empty()) {
        map.drawHighlights(renderer, abilityHighlights, SDL_Color{80, 120, 255, 80});
    }

    auto drawEntity = [&](const Entity& entity) {
        if (!entity.isAlive()) return;
        SDL_Rect rect = {entity.getPosition().x * kTileSize, entity.getPosition().y * kTileSize, kTileSize, kTileSize};
        SDL_Color color = factionColor(entity);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);
    };

    for (auto& player : players) drawEntity(*player);
    for (auto& enemy : enemies) drawEntity(*enemy);
    for (auto& npc : npcs) drawEntity(*npc);

    uiManager->render(renderer, turnManager.getCurrent(), mission, eventLog.getEntries(), hoverText);

    if (gameState == GameState::GameOver && !gameOverDisplayed) {
        eventLog.addEntry("Estado serializado: " + serializeState());
        gameOverDisplayed = true;
    }

    SDL_RenderPresent(renderer);
}

void Game::clean() {
    players.clear();
    enemies.clear();
    npcs.clear();
    delete combatSystem;
    delete uiManager;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void Game::startPlayerTurn(Entity* entity) {
    if (!entity) return;
    gameState = GameState::AwaitingRoll;
    waitingForRoll = true;
    currentAction = UIActionType::None;
    selectedAbilityIndex = -1;
    selectedAbilityId.clear();
    refreshAbilityButtons(entity);
    movementHighlights.clear();
    attackHighlights.clear();
    abilityHighlights.clear();
}

void Game::startEnemyTurn(Entity* entity) {
    gameState = GameState::EnemyTurn;
    waitingForRoll = true;
    enemyTurnPrepared = false;
}

void Game::endCurrentTurn() {
    Entity* previous = turnManager.getCurrent();
    if (previous) {
        previous->setActionPoints(0);
    }
    int previousRound = turnManager.getRoundNumber();
    turnManager.nextTurn();
    if (turnManager.getRoundNumber() > previousRound) {
        mission.registerSurvivedTurn();
        lastRoundRecorded = turnManager.getRoundNumber();
    }
    turnManager.removeEliminated();

    Entity* current = turnManager.getCurrent();
    if (!current) {
        gameState = GameState::GameOver;
        return;
    }

    if (current->getFaction() == EntityFaction::Enemies) {
        startEnemyTurn(current);
    } else {
        startPlayerTurn(current);
    }
    updateHighlights();
}

void Game::evaluateMissions() {
    bool playersAlive = false;
    for (const auto& player : players) {
        if (player->isAlive()) {
            playersAlive = true;
            break;
        }
    }

    bool enemiesAlive = false;
    for (const auto& enemy : enemies) {
        if (enemy->isAlive()) {
            enemiesAlive = true;
            break;
        }
    }

    if (!playersAlive) {
        gameState = GameState::GameOver;
        eventLog.addEntry("Derrota! Todos os herois foram derrotados.");
        return;
    }

    if (!enemiesAlive) {
        mission.registerEnemyDefeated("");
    }

    if (mission.isComplete()) {
        gameState = GameState::GameOver;
        eventLog.addEntry("Vitoria! Objetivos concluidos.");
        return;
    }

    if (currentMap.turnLimit > 0 && turnManager.getRoundNumber() > currentMap.turnLimit) {
        gameState = GameState::GameOver;
        eventLog.addEntry("A partida terminou: limite de turnos atingido.");
    }
}

void Game::handleBoardClick(int cellX, int cellY) {
    Entity* current = turnManager.getCurrent();
    if (!current || current->getFaction() == EntityFaction::Enemies || waitingForRoll) {
        return;
    }

    switch (currentAction) {
    case UIActionType::Move:
        if (cellY >= 0 && cellY < static_cast<int>(movementCosts.size()) &&
            cellX >= 0 && cellX < static_cast<int>(movementCosts[cellY].size())) {
            int cost = movementCosts[cellY][cellX];
            if (cost > 0 && current->hasActionPoints(cost) && !isTileOccupied(cellX, cellY)) {
                current->consumeActionPoints(cost);
                current->setPosition(cellX, cellY);
                eventLog.addEntry(current->getName() + " moveu para (" + std::to_string(cellX) + "," + std::to_string(cellY) + ")");
                applyTileEffect(*current);
                updateHighlights();
            }
        }
        break;
    case UIActionType::Attack: {
        Entity* target = getEntityAt(cellX, cellY);
        if (target && containsCell(attackHighlights, cellX, cellY) &&
            target->getFaction() == EntityFaction::Enemies && current->hasActionPoints(kAttackCost)) {
            current->consumeActionPoints(kAttackCost);
            combatSystem->performBasicAttack(*current, *target, map, eventLog);
            mission.registerEnemyDefeated(target->getId());
            updateHighlights();
        }
        break;
    }
    case UIActionType::Ability: {
        const AbilityDefinition* ability = getAbilityDefinition(selectedAbilityId);
        if (!ability) break;
        if (ability->targetType != AbilityTargetType::Self &&
            !containsCell(abilityHighlights, cellX, cellY)) {
            break;
        }
        Entity* target = nullptr;
        if (ability->targetType == AbilityTargetType::Self) {
            target = current;
        } else {
            target = getEntityAt(cellX, cellY);
        }
        if (combatSystem->useAbility(*ability, *current, target, map, eventLog)) {
            if (target && target->getFaction() == EntityFaction::Enemies && !target->isAlive()) {
                mission.registerEnemyDefeated(target->getId());
            }
            updateHighlights();
        }
        break;
    }
    case UIActionType::Interact: {
        if (!containsCell(abilityHighlights, cellX, cellY)) break;
        Entity* npc = getEntityAt(cellX, cellY);
        if (npc && npc->getFaction() == EntityFaction::Neutral) {
            mission.registerNpcConversation(npc->getId());
            eventLog.addEntry("Conversa com " + npc->getName());
        } else {
            TileSpecialType specialType = map.getSpecialType(cellX, cellY);
            if (specialType == TileSpecialType::Item) {
                mission.registerItemCollected(map.getSpecialDefinition(cellX, cellY).targetId);
                eventLog.addEntry("Item recuperado.");
                map.removeItemAt(cellX, cellY);
            }
        }
        break;
    }
    case UIActionType::Pass: {
        current->setActionPoints(0);
        break;
    }
    case UIActionType::None:
    default:
        break;
    }

    if (current->getActionPoints() <= 0 && gameState != GameState::GameOver) {
        endCurrentTurn();
    }
}

void Game::setCurrentAction(UIActionType action) {
    currentAction = action;
    if (action == UIActionType::Pass) {
        endCurrentTurn();
        return;
    }
    updateHighlights();
}

void Game::updateHighlights() {
    movementHighlights.clear();
    movementCosts.clear();
    attackHighlights.clear();
    abilityHighlights.clear();

    Entity* current = turnManager.getCurrent();
    if (!current) return;

    if (currentAction == UIActionType::Move) {
        movementCosts = calculateMovementCost(*current, current->getActionPoints());
        for (int y = 0; y < static_cast<int>(movementCosts.size()); ++y) {
            for (int x = 0; x < static_cast<int>(movementCosts[y].size()); ++x) {
                if (movementCosts[y][x] > 0) {
                    movementHighlights.push_back({x, y});
                }
            }
        }
    } else if (currentAction == UIActionType::Attack) {
        std::vector<SDL_Point> range = calculateRange(*current, current->getAttackRange());
        for (const auto& cell : range) {
            Entity* target = getEntityAt(cell.x, cell.y);
            if (target && target->getFaction() != current->getFaction()) {
                attackHighlights.push_back(cell);
            }
        }
    } else if (currentAction == UIActionType::Ability) {
        const AbilityDefinition* ability = getAbilityDefinition(selectedAbilityId);
        if (ability) {
            std::vector<SDL_Point> range = calculateRange(*current, ability->range);
            abilityHighlights = range;
            if (ability->targetType == AbilityTargetType::Self) {
                abilityHighlights.push_back(current->getPosition());
            }
        }
    } else if (currentAction == UIActionType::Interact) {
        std::vector<SDL_Point> neighbors = calculateRange(*current, 1);
        for (const auto& cell : neighbors) {
            if (map.getSpecialType(cell.x, cell.y) != TileSpecialType::None || getEntityAt(cell.x, cell.y)) {
                abilityHighlights.push_back(cell);
            }
        }
    }
}

void Game::updateHoverInfo(int mouseX, int mouseY) {
    if (mouseX >= boardPixelWidth) {
        hoverText = "";
    } else {
        int cellX = mouseX / kTileSize;
        int cellY = mouseY / kTileSize;
        hoverText = buildHoverText(cellX, cellY);
    }
}

void Game::processEnemyTurn() {
    Entity* enemy = turnManager.getCurrent();
    if (!enemy || enemy->getFaction() != EntityFaction::Enemies) {
        gameState = GameState::ActionSelection;
        waitingForRoll = true;
        return;
    }

    if (waitingForRoll) {
        int roll = dice.roll(6);
        enemy->setActionPoints(roll);
        waitingForRoll = false;
        enemyTurnPrepared = true;
        eventLog.addEntry(enemy->getName() + " (IA) ganhou " + std::to_string(roll) + " AP");
    }

    if (!enemyTurnPrepared) {
        enemyTurnPrepared = true;
    }

    while (enemy->getActionPoints() > 0) {
        PlayerEntity* closestPlayer = nullptr;
        int bestDistance = 999;
        for (auto& player : players) {
            if (!player->isAlive()) continue;
            int dist = std::abs(player->getPosition().x - enemy->getPosition().x) +
                       std::abs(player->getPosition().y - enemy->getPosition().y);
            if (dist < bestDistance) {
                bestDistance = dist;
                closestPlayer = player.get();
            }
        }
        if (!closestPlayer) {
            enemy->setActionPoints(0);
            break;
        }

        if (bestDistance <= enemy->getAttackRange() && enemy->hasActionPoints(kAttackCost)) {
            enemy->consumeActionPoints(kAttackCost);
            combatSystem->performBasicAttack(*enemy, *closestPlayer, map, eventLog);
            if (!closestPlayer->isAlive()) {
                eventLog.addEntry(closestPlayer->getName() + " caiu em combate.");
            }
        } else {
            int dx = (closestPlayer->getPosition().x > enemy->getPosition().x) ? 1 : (closestPlayer->getPosition().x < enemy->getPosition().x ? -1 : 0);
            int dy = (closestPlayer->getPosition().y > enemy->getPosition().y) ? 1 : (closestPlayer->getPosition().y < enemy->getPosition().y ? -1 : 0);
            int targetX = enemy->getPosition().x + (dx != 0 ? dx : 0);
            int targetY = enemy->getPosition().y + ((dx == 0 && dy != 0) ? dy : 0);
            if (map.isInside(targetX, targetY) && !map.blocksMovement(targetX, targetY) && !isTileOccupied(targetX, targetY)) {
                int cost = map.getMovementCost(targetX, targetY);
                if (enemy->hasActionPoints(cost)) {
                    enemy->consumeActionPoints(cost);
                    enemy->setPosition(targetX, targetY);
                } else {
                    enemy->setActionPoints(0);
                }
            } else {
                enemy->setActionPoints(0);
            }
        }
    }

    endCurrentTurn();
}

std::vector<SDL_Point> Game::calculateReachableCells(const Entity& entity, int ap) {
    std::vector<std::vector<int>> costs = calculateMovementCost(entity, ap);
    std::vector<SDL_Point> cells;
    for (int y = 0; y < static_cast<int>(costs.size()); ++y) {
        for (int x = 0; x < static_cast<int>(costs[y].size()); ++x) {
            if (costs[y][x] > 0) {
                cells.push_back({x, y});
            }
        }
    }
    return cells;
}

std::vector<std::vector<int>> Game::calculateMovementCost(const Entity& entity, int ap) {
    std::vector<std::vector<int>> costs(currentMap.height, std::vector<int>(currentMap.width, -1));
    std::queue<SDL_Point> frontier;
    frontier.push(entity.getPosition());
    costs[entity.getPosition().y][entity.getPosition().x] = 0;

    const int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    while (!frontier.empty()) {
        SDL_Point current = frontier.front();
        frontier.pop();
        for (const auto& dir : dirs) {
            int nx = current.x + dir[0];
            int ny = current.y + dir[1];
            if (!map.isInside(nx, ny) || map.blocksMovement(nx, ny)) continue;
            if (isTileOccupied(nx, ny)) continue;
            int moveCost = map.getMovementCost(nx, ny);
            int newCost = costs[current.y][current.x] + moveCost;
            if (newCost == 0) newCost = moveCost;
            if (newCost <= ap && (costs[ny][nx] == -1 || newCost < costs[ny][nx])) {
                costs[ny][nx] = newCost;
                frontier.push({nx, ny});
            }
        }
    }

    return costs;
}

std::vector<SDL_Point> Game::calculateRange(const Entity& entity, int distance) const {
    std::vector<SDL_Point> result;
    SDL_Point pos = entity.getPosition();
    for (int y = pos.y - distance; y <= pos.y + distance; ++y) {
        for (int x = pos.x - distance; x <= pos.x + distance; ++x) {
            if (!map.isInside(x, y)) continue;
            if (std::abs(x - pos.x) + std::abs(y - pos.y) <= distance && !(x == pos.x && y == pos.y)) {
                result.push_back({x, y});
            }
        }
    }
    return result;
}

Entity* Game::getEntityAt(int x, int y) {
    for (auto& player : players) if (player->isAlive() && player->getPosition().x == x && player->getPosition().y == y) return player.get();
    for (auto& enemy : enemies) if (enemy->isAlive() && enemy->getPosition().x == x && enemy->getPosition().y == y) return enemy.get();
    for (auto& npc : npcs) if (npc->isAlive() && npc->getPosition().x == x && npc->getPosition().y == y) return npc.get();
    return nullptr;
}

bool Game::isTileOccupied(int x, int y) const {
    for (const auto& player : players) if (player->isAlive() && player->getPosition().x == x && player->getPosition().y == y) return true;
    for (const auto& enemy : enemies) if (enemy->isAlive() && enemy->getPosition().x == x && enemy->getPosition().y == y) return true;
    for (const auto& npc : npcs) if (npc->isAlive() && npc->getPosition().x == x && npc->getPosition().y == y) return true;
    return false;
}

void Game::applyTileEffect(Entity& entity) {
    TileSpecialType type = map.getSpecialType(entity.getPosition().x, entity.getPosition().y);
    SpecialTileDefinition def = map.getSpecialDefinition(entity.getPosition().x, entity.getPosition().y);
    switch (type) {
    case TileSpecialType::Trap:
        entity.takeDamage(def.value);
        eventLog.addEntry(entity.getName() + " sofreu " + std::to_string(def.value) + " de dano de armadilha.");
        break;
    case TileSpecialType::Heal:
        entity.heal(def.value);
        eventLog.addEntry(entity.getName() + " recuperou " + std::to_string(def.value) + " HP.");
        break;
    case TileSpecialType::Portal:
        if (!def.targetId.empty()) {
            size_t comma = def.targetId.find(',');
            if (comma != std::string::npos) {
                int tx = std::stoi(def.targetId.substr(0, comma));
                int ty = std::stoi(def.targetId.substr(comma + 1));
                if (map.isInside(tx, ty)) {
                    entity.setPosition(tx, ty);
                    eventLog.addEntry("Portal transportou " + entity.getName());
                }
            }
        }
        break;
    case TileSpecialType::Item:
        mission.registerItemCollected(def.targetId);
        map.removeItemAt(entity.getPosition().x, entity.getPosition().y);
        eventLog.addEntry(entity.getName() + " coletou um item.");
        break;
    case TileSpecialType::Objective:
        mission.registerTileReached(entity.getPosition().x, entity.getPosition().y);
        break;
    case TileSpecialType::None:
        break;
    }
}

std::string Game::buildHoverText(int cellX, int cellY) const {
    if (!map.isInside(cellX, cellY)) return "";
    std::ostringstream info;
    info << "Celula (" << cellX << "," << cellY << ")";
    Entity* entity = const_cast<Game*>(this)->getEntityAt(cellX, cellY);
    if (entity) {
        info << " - " << entity->getName() << " HP " << entity->getCurrentHP();
    }
    TileSpecialType special = map.getSpecialType(cellX, cellY);
    if (special != TileSpecialType::None) {
        info << " [Especial]";
    }
    return info.str();
}

void Game::refreshAbilityButtons(const Entity* entity) {
    if (!entity) return;
    std::vector<AbilityButtonEntry> entries;
    for (const auto& abilityId : entity->getAbilityIds()) {
        auto abilityIt = content.abilities.find(abilityId);
        if (abilityIt != content.abilities.end()) {
            entries.push_back({abilityId, abilityIt->second.name});
        }
    }
    uiManager->setAbilities(entries);
}

const AbilityDefinition* Game::getAbilityDefinition(const std::string& id) const {
    auto it = content.abilities.find(id);
    if (it == content.abilities.end()) return nullptr;
    return &it->second;
}

std::string Game::serializeState() const {
    std::ostringstream out;
    out << "{";
    out << "\"map\":\"" << currentMap.id << "\",";
    out << "\"turn\":" << turnManager.getRoundNumber() << ",";
    out << "\"entities\":[";
    bool first = true;
    auto writeEntity = [&](const Entity& entity) {
        if (!first) out << ",";
        first = false;
        out << "{";
        out << "\"id\":\"" << entity.getId() << "\",";
        out << "\"hp\":" << entity.getCurrentHP() << ",";
        out << "\"energy\":" << entity.getCurrentEnergy() << ",";
        out << "\"level\":" << entity.getLevel() << ",";
        out << "\"x\":" << entity.getPosition().x << ",";
        out << "\"y\":" << entity.getPosition().y;
        out << "}";
    };
    for (const auto& player : players) writeEntity(*player);
    for (const auto& enemy : enemies) writeEntity(*enemy);
    for (const auto& npc : npcs) writeEntity(*npc);
    out << "],";
    out << "\"objectives\":[";
    for (size_t i = 0; i < mission.getObjectives().size(); ++i) {
        if (i > 0) out << ",";
        const auto& objective = mission.getObjectives()[i];
        out << "{\"desc\":\"" << objective.definition.description << "\",\"completed\":" << (objective.completed ? "true" : "false") << "}";
    }
    out << "]";
    out << "}";
    return out.str();
}

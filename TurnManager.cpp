#include "TurnManager.h"
#include <algorithm>

TurnManager::TurnManager() : currentIndex(0), roundNumber(1) {}

void TurnManager::setParticipants(const std::vector<Entity*>& entities) {
    turnOrder = entities;
    std::sort(turnOrder.begin(), turnOrder.end(), [](Entity* a, Entity* b) {
        return a->getAttributes().agility > b->getAttributes().agility;
    });
    currentIndex = 0;
    roundNumber = 1;
}

void TurnManager::nextTurn() {
    if (turnOrder.empty()) return;
    currentIndex = (currentIndex + 1) % turnOrder.size();
    if (currentIndex == 0) {
        roundNumber++;
    }
}

Entity* TurnManager::getCurrent() const {
    if (turnOrder.empty()) return nullptr;
    return turnOrder[currentIndex];
}

void TurnManager::removeEliminated() {
    turnOrder.erase(std::remove_if(turnOrder.begin(), turnOrder.end(),
        [](Entity* e){ return !e->isAlive(); }), turnOrder.end());
    if (currentIndex >= static_cast<int>(turnOrder.size())) {
        currentIndex = 0;
    }
}

#ifndef TURNMANAGER_H
#define TURNMANAGER_H

#include <vector>
#include "Entity.h"

class TurnManager {
public:
    TurnManager();

    void setParticipants(const std::vector<Entity*>& entities);
    void nextTurn();
    Entity* getCurrent() const;
    void removeEliminated();
    int getRoundNumber() const { return roundNumber; }

private:
    std::vector<Entity*> turnOrder;
    int currentIndex;
    int roundNumber;
};

#endif // TURNMANAGER_H

#ifndef MISSION_H
#define MISSION_H

#include <vector>
#include <string>
#include "GameContent.h"

struct ObjectiveState {
    MissionObjectiveDefinition definition;
    int progress = 0;
    bool completed = false;
};

class Mission {
public:
    Mission();
    explicit Mission(const std::vector<MissionObjectiveDefinition>& definitions);

    void registerEnemyDefeated(const std::string& enemyId);
    void registerNpcConversation(const std::string& npcId);
    void registerItemCollected(const std::string& itemId);
    void registerTileReached(int x, int y);
    void registerSurvivedTurn();

    bool isComplete() const;
    const std::vector<ObjectiveState>& getObjectives() const { return objectives; }

private:
    std::vector<ObjectiveState> objectives;
};

#endif

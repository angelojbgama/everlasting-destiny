#include "Mission.h"

Mission::Mission() {}

Mission::Mission(const std::vector<MissionObjectiveDefinition>& definitions) {
    for (const auto& def : definitions) {
        objectives.push_back({def, 0, false});
    }
}

void Mission::registerEnemyDefeated(const std::string& enemyId) {
    for (auto& objective : objectives) {
        if (objective.definition.type == ObjectiveType::DefeatEnemies &&
            (objective.definition.targetId.empty() || objective.definition.targetId == enemyId)) {
            objective.progress++;
            if (objective.definition.amount == 0 || objective.progress >= objective.definition.amount) {
                objective.completed = true;
            }
        }
    }
}

void Mission::registerNpcConversation(const std::string& npcId) {
    for (auto& objective : objectives) {
        if (objective.definition.type == ObjectiveType::TalkToNpc &&
            (objective.definition.targetId.empty() || objective.definition.targetId == npcId)) {
            objective.progress = 1;
            objective.completed = true;
        }
    }
}

void Mission::registerItemCollected(const std::string& itemId) {
    for (auto& objective : objectives) {
        if (objective.definition.type == ObjectiveType::CollectItem &&
            (objective.definition.targetId.empty() || objective.definition.targetId == itemId)) {
            objective.progress++;
            if (objective.definition.amount == 0 || objective.progress >= objective.definition.amount) {
                objective.completed = true;
            }
        }
    }
}

void Mission::registerTileReached(int x, int y) {
    for (auto& objective : objectives) {
        if (objective.definition.type == ObjectiveType::ReachTile &&
            objective.definition.targetX == x && objective.definition.targetY == y) {
            objective.progress = 1;
            objective.completed = true;
        }
    }
}

void Mission::registerSurvivedTurn() {
    for (auto& objective : objectives) {
        if (objective.definition.type == ObjectiveType::SurviveTurns) {
            objective.progress++;
            if (objective.definition.turnLimit == 0 || objective.progress >= objective.definition.turnLimit) {
                objective.completed = true;
            }
        }
    }
}

bool Mission::isComplete() const {
    if (objectives.empty()) {
        return false;
    }
    for (const auto& objective : objectives) {
        if (!objective.completed) {
            return false;
        }
    }
    return true;
}

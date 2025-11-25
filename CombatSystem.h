#ifndef COMBATSYSTEM_H
#define COMBATSYSTEM_H

#include "Entity.h"
#include "Map.h"
#include "GameContent.h"
#include "Dice.h"
#include "EventLog.h"

class CombatSystem {
public:
    explicit CombatSystem(Dice* dice);

    int performBasicAttack(Entity& attacker, Entity& defender, const Map& map, EventLog& log);
    bool useAbility(const AbilityDefinition& ability, Entity& user, Entity* target, const Map& map, EventLog& log);

private:
    Dice* dice;

    bool didDodge(const Entity& defender, const Map& map) const;
};

#endif

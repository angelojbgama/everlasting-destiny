#include "CombatSystem.h"
#include <algorithm>

CombatSystem::CombatSystem(Dice* dicePtr) : dice(dicePtr) {}

int CombatSystem::performBasicAttack(Entity& attacker, Entity& defender, const Map& map, EventLog& log) {
    if (!attacker.isAlive() || !defender.isAlive()) {
        return 0;
    }
    if (didDodge(defender, map)) {
        log.addEntry(defender.getName() + " dodged the attack!");
        return 0;
    }

    int terrainBonus = map.getDefenseModifier(defender.getPosition().x, defender.getPosition().y);
    int attackPower = attacker.getBaseAttack() + attacker.getAttributes().strength * 2 + dice->roll(6);
    int defensePower = defender.getAttributes().defense + terrainBonus;
    int damage = std::max(1, attackPower - defensePower);
    defender.takeDamage(damage);
    log.addEntry(attacker.getName() + " dealt " + std::to_string(damage) + " damage to " + defender.getName());
    if (!defender.isAlive()) {
        log.addEntry(defender.getName() + " has been defeated.");
        attacker.grantExperience(40);
    }
    return damage;
}

bool CombatSystem::useAbility(const AbilityDefinition& ability, Entity& user, Entity* target, const Map& map, EventLog& log) {
    if (!user.hasActionPoints(ability.apCost) || !user.hasEnergy(ability.energyCost)) {
        return false;
    }
    user.consumeActionPoints(ability.apCost);
    user.spendEnergy(ability.energyCost);

    switch (ability.effectType) {
    case AbilityEffectType::Damage:
        if (target) {
            int damage = ability.power + user.getAttributes().intelligence;
            target->takeDamage(damage);
            log.addEntry(user.getName() + " used " + ability.name + " on " + target->getName() + " for " + std::to_string(damage) + " damage");
            if (!target->isAlive()) {
                log.addEntry(target->getName() + " was eliminated.");
            }
        }
        break;
    case AbilityEffectType::Heal:
        if (target) {
            target->heal(ability.power);
            log.addEntry(user.getName() + " healed " + target->getName() + " for " + std::to_string(ability.power));
        }
        break;
    case AbilityEffectType::Buff:
        user.addStatus("buff_" + ability.id, 3);
        log.addEntry(user.getName() + " gains a buff from " + ability.name);
        break;
    case AbilityEffectType::Debuff:
        if (target) {
            target->addStatus("debuff_" + ability.id, 3);
            log.addEntry(target->getName() + " suffers a debuff from " + ability.name);
        }
        break;
    case AbilityEffectType::Status:
        user.addStatus(ability.id, 2);
        log.addEntry(user.getName() + " activates " + ability.name);
        break;
    }
    return true;
}

bool CombatSystem::didDodge(const Entity& defender, const Map& map) const {
    int dodgeScore = defender.getAttributes().agility * 2 + map.getDodgeModifier(defender.getPosition().x, defender.getPosition().y);
    int roll = dice->roll(100);
    return roll <= dodgeScore;
}

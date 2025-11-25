#include "Entity.h"
#include <algorithm>

Entity::Entity(const EntityDefinition& definition, const SDL_Point& spawn)
    : id(definition.id),
      name(definition.name),
      kind(definition.kind),
      faction(definition.faction),
      attributes(definition.attributes),
      level(1),
      experience(0),
      experienceToNext(100),
      maxHP(definition.maxHP),
      currentHP(definition.maxHP),
      maxEnergy(definition.maxEnergy),
      currentEnergy(definition.maxEnergy),
      actionPoints(0),
      baseAttack(definition.baseAttack),
      attackRange(definition.attackRange),
      abilityIds(definition.abilityIds),
      passiveEffects(definition.passiveEffects),
      position(spawn) {
}

void Entity::update() {
    tickStatuses();
}

void Entity::setPosition(int x, int y) {
    position.x = x;
    position.y = y;
}

void Entity::consumeActionPoints(int value) {
    actionPoints = std::max(0, actionPoints - value);
}

void Entity::grantExperience(int amount) {
    experience += amount;
    while (experience >= experienceToNext) {
        experience -= experienceToNext;
        levelUp();
    }
}

void Entity::takeDamage(int amount) {
    currentHP = std::max(0, currentHP - amount);
}

void Entity::heal(int amount) {
    currentHP = std::min(maxHP, currentHP + amount);
}

void Entity::spendEnergy(int amount) {
    currentEnergy = std::max(0, currentEnergy - amount);
}

void Entity::restoreEnergy(int amount) {
    currentEnergy = std::min(maxEnergy, currentEnergy + amount);
}

void Entity::addStatus(const std::string& statusId, int duration) {
    statuses.push_back({statusId, duration});
}

void Entity::tickStatuses() {
    for (auto& status : statuses) {
        if (status.remainingTurns > 0) {
            status.remainingTurns--;
        }
    }
    statuses.erase(std::remove_if(statuses.begin(), statuses.end(),
        [](const StatusEffectState& s) { return s.remainingTurns <= 0; }), statuses.end());
}

void Entity::levelUp() {
    level++;
    experienceToNext += level * 50;
    attributes.strength += 1;
    attributes.agility += 1;
    attributes.intelligence += 1;
    attributes.defense += 1;
    maxHP += 10;
    maxEnergy += 5;
    currentHP = maxHP;
    currentEnergy = maxEnergy;
}

PlayerEntity::PlayerEntity(const EntityDefinition& definition, const SDL_Point& spawn)
    : Entity(definition, spawn) {
}

EnemyEntity::EnemyEntity(const EntityDefinition& definition, const SDL_Point& spawn)
    : Entity(definition, spawn) {
}

NpcEntity::NpcEntity(const EntityDefinition& definition, const SDL_Point& spawn)
    : Entity(definition, spawn),
      dialog(definition.dialog.empty() ? "..." : definition.dialog) {
}

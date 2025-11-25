#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include "GameContent.h"

struct StatusEffectState {
    std::string id;
    int remainingTurns = 0;
};

class Entity {
public:
    Entity(const EntityDefinition& definition, const SDL_Point& spawn);
    virtual ~Entity() = default;

    virtual void update();

    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    EntityKind getKind() const { return kind; }
    EntityFaction getFaction() const { return faction; }
    SDL_Point getPosition() const { return position; }
    void setPosition(int x, int y);

    int getCurrentHP() const { return currentHP; }
    int getMaxHP() const { return maxHP; }
    int getCurrentEnergy() const { return currentEnergy; }
    int getMaxEnergy() const { return maxEnergy; }
    int getActionPoints() const { return actionPoints; }
    void setActionPoints(int value) { actionPoints = value; }
    void consumeActionPoints(int value);
    bool hasActionPoints(int value) const { return actionPoints >= value; }

    int getBaseAttack() const { return baseAttack; }
    int getAttackRange() const { return attackRange; }
    const Attributes& getAttributes() const { return attributes; }

    int getLevel() const { return level; }
    int getExperience() const { return experience; }
    int getExperienceToNext() const { return experienceToNext; }
    void grantExperience(int amount);

    bool isAlive() const { return currentHP > 0; }
    void takeDamage(int amount);
    void heal(int amount);
    void spendEnergy(int amount);
    bool hasEnergy(int amount) const { return currentEnergy >= amount; }
    void restoreEnergy(int amount);

    void addStatus(const std::string& statusId, int duration);
    void tickStatuses();
    const std::vector<StatusEffectState>& getStatuses() const { return statuses; }

    const std::vector<std::string>& getAbilityIds() const { return abilityIds; }
    const std::vector<std::string>& getPassiveEffects() const { return passiveEffects; }

protected:
    std::string id;
    std::string name;
    EntityKind kind;
    EntityFaction faction;
    Attributes attributes;
    int level;
    int experience;
    int experienceToNext;
    int maxHP;
    int currentHP;
    int maxEnergy;
    int currentEnergy;
    int actionPoints;
    int baseAttack;
    int attackRange;
    std::vector<std::string> abilityIds;
    std::vector<std::string> passiveEffects;
    SDL_Point position;
    std::vector<StatusEffectState> statuses;

    void levelUp();
};

class PlayerEntity : public Entity {
public:
    PlayerEntity(const EntityDefinition& definition, const SDL_Point& spawn);
};

class EnemyEntity : public Entity {
public:
    EnemyEntity(const EntityDefinition& definition, const SDL_Point& spawn);
};

class NpcEntity : public Entity {
public:
    NpcEntity(const EntityDefinition& definition, const SDL_Point& spawn);
    const std::string& getDialog() const { return dialog; }
private:
    std::string dialog;
};

#endif

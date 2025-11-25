#ifndef GAMECONTENT_H
#define GAMECONTENT_H

#include <string>
#include <vector>
#include <unordered_map>
#include <SDL2/SDL.h>

enum class EntityFaction {
    Players,
    Enemies,
    Neutral
};

enum class EntityKind {
    Player,
    Enemy,
    Npc
};

enum class ObjectiveType {
    DefeatEnemies,
    TalkToNpc,
    CollectItem,
    ReachTile,
    SurviveTurns
};

enum class GameModeType {
    FreeForAll,
    Cooperative,
    Survival,
    Points
};

enum class AbilityTargetType {
    Self,
    Ally,
    Enemy,
    Area
};

enum class AbilityEffectType {
    Damage,
    Heal,
    Buff,
    Debuff,
    Status
};

enum class TileSpecialType {
    None,
    Trap,
    Heal,
    Portal,
    Objective,
    Item
};

struct Attributes {
    int strength = 0;
    int agility = 0;
    int intelligence = 0;
    int defense = 0;
};

struct AbilityDefinition {
    std::string id;
    std::string name;
    std::string description;
    int apCost = 0;
    int energyCost = 0;
    int range = 1;
    AbilityTargetType targetType = AbilityTargetType::Enemy;
    AbilityEffectType effectType = AbilityEffectType::Damage;
    int power = 0;
};

struct ItemDefinition {
    std::string id;
    std::string name;
    std::string description;
};

struct EntityDefinition {
    std::string id;
    std::string name;
    std::string dialog;
    EntityKind kind = EntityKind::Player;
    EntityFaction faction = EntityFaction::Players;
    Attributes attributes;
    int maxHP = 100;
    int maxEnergy = 50;
    int baseAttack = 5;
    int attackRange = 1;
    std::vector<std::string> abilityIds;
    std::vector<std::string> passiveEffects;
};

struct TerrainTypeDefinition {
    std::string id;
    std::string name;
    int movementCost = 1;
    int defenseModifier = 0;
    int dodgeModifier = 0;
    bool blocksMovement = false;
    bool blocksLineOfSight = false;
    SDL_Color color = {0, 128, 0, 255};
};

struct SpecialTileDefinition {
    TileSpecialType type = TileSpecialType::None;
    int x = 0;
    int y = 0;
    int value = 0;
    std::string targetId;
};

struct MissionObjectiveDefinition {
    ObjectiveType type = ObjectiveType::DefeatEnemies;
    std::string description;
    std::string targetId;
    int amount = 0;
    int turnLimit = 0;
    int targetX = -1;
    int targetY = -1;
};

struct MapDefinition {
    std::string id;
    std::string name;
    int width = 0;
    int height = 0;
    std::vector<std::vector<std::string>> terrainIds;
    std::string rhythm;
    GameModeType mode = GameModeType::Cooperative;
    int turnLimit = 0;
    std::vector<SpecialTileDefinition> specials;
    std::vector<MissionObjectiveDefinition> objectives;
    std::vector<std::string> loseConditions;
    std::vector<std::string> playerIds;
    std::vector<std::string> enemyIds;
    std::vector<std::string> npcIds;
    std::vector<SDL_Point> playerSpawns;
    std::vector<SDL_Point> enemySpawns;
    std::vector<SDL_Point> npcSpawns;
};

struct GameContent {
    std::unordered_map<std::string, TerrainTypeDefinition> terrainTypes;
    std::unordered_map<std::string, AbilityDefinition> abilities;
    std::unordered_map<std::string, ItemDefinition> items;
    std::unordered_map<std::string, EntityDefinition> entities;
    std::unordered_map<std::string, MapDefinition> maps;
};

#endif

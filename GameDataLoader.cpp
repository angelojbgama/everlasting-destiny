#include "GameDataLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

GameDataLoader::GameDataLoader() {}

bool GameDataLoader::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Unable to open data file: " << path << std::endl;
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    SimpleJsonParser parser(buffer.str());
    SimpleJsonValue root = parser.parse();

    loadTerrain(root["terrain_types"]);
    loadAbilities(root["abilities"]);
    loadItems(root["items"]);
    loadEntities(root["entities"]);
    loadMaps(root["maps"]);

    return true;
}

void GameDataLoader::loadTerrain(const SimpleJsonValue& node) {
    if (node.getType() != SimpleJsonValue::Type::Array) return;
    for (const auto& entry : node.asArray()) {
        TerrainTypeDefinition def;
        def.id = entry["id"].asString();
        def.name = entry["name"].asString(def.id);
        def.movementCost = entry["movement_cost"].asInt(1);
        def.defenseModifier = entry["defense_bonus"].asInt(0);
        def.dodgeModifier = entry["evasion_bonus"].asInt(0);
        def.blocksMovement = entry["blocks_movement"].asBool(false);
        def.blocksLineOfSight = entry["blocks_los"].asBool(false);
        const auto& colorNode = entry["color"];
        if (colorNode.getType() == SimpleJsonValue::Type::Array) {
            const auto& arr = colorNode.asArray();
            if (arr.size() >= 3) {
                def.color.r = arr[0].asInt(0);
                def.color.g = arr[1].asInt(0);
                def.color.b = arr[2].asInt(0);
                def.color.a = arr.size() > 3 ? arr[3].asInt(255) : 255;
            }
        }
        content.terrainTypes[def.id] = def;
    }
}

void GameDataLoader::loadAbilities(const SimpleJsonValue& node) {
    if (node.getType() != SimpleJsonValue::Type::Array) return;
    for (const auto& entry : node.asArray()) {
        AbilityDefinition def;
        def.id = entry["id"].asString();
        def.name = entry["name"].asString(def.id);
        def.description = entry["description"].asString();
        def.apCost = entry["ap_cost"].asInt(0);
        def.energyCost = entry["energy_cost"].asInt(0);
        def.range = entry["range"].asInt(1);
        def.targetType = parseAbilityTarget(entry["target"].asString("enemy"));
        def.effectType = parseEffectType(entry["effect"].asString("damage"));
        def.power = entry["power"].asInt(0);
        content.abilities[def.id] = def;
    }
}

void GameDataLoader::loadItems(const SimpleJsonValue& node) {
    if (node.getType() != SimpleJsonValue::Type::Array) return;
    for (const auto& entry : node.asArray()) {
        ItemDefinition def;
        def.id = entry["id"].asString();
        def.name = entry["name"].asString(def.id);
        def.description = entry["description"].asString();
        content.items[def.id] = def;
    }
}

void GameDataLoader::loadEntities(const SimpleJsonValue& node) {
    if (node.getType() != SimpleJsonValue::Type::Array) return;
    for (const auto& entry : node.asArray()) {
        EntityDefinition def;
        def.id = entry["id"].asString();
        def.name = entry["name"].asString(def.id);
        def.dialog = entry["dialog"].asString();
        def.kind = parseEntityKind(entry["kind"].asString("player"));
        def.faction = parseFaction(entry["faction"].asString("players"));
        def.attributes.strength = entry["strength"].asInt(3);
        def.attributes.agility = entry["agility"].asInt(3);
        def.attributes.intelligence = entry["intelligence"].asInt(3);
        def.attributes.defense = entry["defense"].asInt(3);
        def.maxHP = entry["hp"].asInt(100);
        def.maxEnergy = entry["energy"].asInt(50);
        def.baseAttack = entry["attack"].asInt(10);
        def.attackRange = entry["range"].asInt(1);
        if (entry["abilities"].getType() == SimpleJsonValue::Type::Array) {
            for (const auto& abilityNode : entry["abilities"].asArray()) {
                def.abilityIds.push_back(abilityNode.asString());
            }
        }
        if (entry["passives"].getType() == SimpleJsonValue::Type::Array) {
            for (const auto& passiveNode : entry["passives"].asArray()) {
                def.passiveEffects.push_back(passiveNode.asString());
            }
        }
        content.entities[def.id] = def;
    }
}

void GameDataLoader::loadMaps(const SimpleJsonValue& node) {
    if (node.getType() != SimpleJsonValue::Type::Array) return;
    for (const auto& entry : node.asArray()) {
        MapDefinition def;
        def.id = entry["id"].asString();
        def.name = entry["name"].asString(def.id);
        def.width = entry["width"].asInt(20);
        def.height = entry["height"].asInt(20);
        def.rhythm = entry["rhythm"].asString("short");
        def.mode = parseGameMode(entry["mode"].asString("cooperative"));
        def.turnLimit = entry["turn_limit"].asInt(0);

        const auto& rowsNode = entry["rows"];
        if (rowsNode.getType() == SimpleJsonValue::Type::Array) {
            for (const auto& row : rowsNode.asArray()) {
                std::vector<std::string> rowIds;
                const auto& rowValues = row.asArray();
                for (const auto& value : rowValues) {
                    rowIds.push_back(value.asString("plain"));
                }
                def.terrainIds.push_back(rowIds);
            }
        }

        const auto& specialsNode = entry["special_tiles"];
        if (specialsNode.getType() == SimpleJsonValue::Type::Array) {
            for (const auto& special : specialsNode.asArray()) {
                SpecialTileDefinition specialDef;
                specialDef.type = parseSpecialType(special["type"].asString("none"));
                specialDef.x = special["x"].asInt(0);
                specialDef.y = special["y"].asInt(0);
                specialDef.value = special["value"].asInt(0);
                specialDef.targetId = special["target"].asString();
                def.specials.push_back(specialDef);
            }
        }

        const auto& objectivesNode = entry["objectives"];
        if (objectivesNode.getType() == SimpleJsonValue::Type::Array) {
            for (const auto& obj : objectivesNode.asArray()) {
                MissionObjectiveDefinition missionDef;
                missionDef.type = parseObjectiveType(obj["type"].asString("defeat"));
                missionDef.description = obj["description"].asString();
                missionDef.targetId = obj["target"].asString();
                missionDef.amount = obj["amount"].asInt(0);
                missionDef.turnLimit = obj["turns"].asInt(0);
                missionDef.targetX = obj["x"].asInt(-1);
                missionDef.targetY = obj["y"].asInt(-1);
                def.objectives.push_back(missionDef);
            }
        }

        const auto& losesNode = entry["lose_conditions"];
        if (losesNode.getType() == SimpleJsonValue::Type::Array) {
            for (const auto& lose : losesNode.asArray()) {
                def.loseConditions.push_back(lose.asString());
            }
        }

        const auto& playersNode = entry["player_ids"];
        if (playersNode.getType() == SimpleJsonValue::Type::Array) {
            for (const auto& pid : playersNode.asArray()) {
                def.playerIds.push_back(pid.asString());
            }
        }

        const auto& enemiesNode = entry["enemy_ids"];
        if (enemiesNode.getType() == SimpleJsonValue::Type::Array) {
            for (const auto& eid : enemiesNode.asArray()) {
                def.enemyIds.push_back(eid.asString());
            }
        }

        const auto& npcsNode = entry["npc_ids"];
        if (npcsNode.getType() == SimpleJsonValue::Type::Array) {
            for (const auto& nid : npcsNode.asArray()) {
                def.npcIds.push_back(nid.asString());
            }
        }

        const auto& playerSpawnsNode = entry["player_spawns"];
        if (playerSpawnsNode.getType() == SimpleJsonValue::Type::Array) {
            for (const auto& spawnNode : playerSpawnsNode.asArray()) {
                SDL_Point spawn = {spawnNode["x"].asInt(0), spawnNode["y"].asInt(0)};
                def.playerSpawns.push_back(spawn);
            }
        }

        const auto& enemySpawnsNode = entry["enemy_spawns"];
        if (enemySpawnsNode.getType() == SimpleJsonValue::Type::Array) {
            for (const auto& spawnNode : enemySpawnsNode.asArray()) {
                SDL_Point spawn = {spawnNode["x"].asInt(0), spawnNode["y"].asInt(0)};
                def.enemySpawns.push_back(spawn);
            }
        }

        const auto& npcSpawnsNode = entry["npc_spawns"];
        if (npcSpawnsNode.getType() == SimpleJsonValue::Type::Array) {
            for (const auto& spawnNode : npcSpawnsNode.asArray()) {
                SDL_Point spawn = {spawnNode["x"].asInt(0), spawnNode["y"].asInt(0)};
                def.npcSpawns.push_back(spawn);
            }
        }

        content.maps[def.id] = def;
    }
}

EntityKind GameDataLoader::parseEntityKind(const std::string& value) {
    if (value == "enemy") return EntityKind::Enemy;
    if (value == "npc") return EntityKind::Npc;
    return EntityKind::Player;
}

EntityFaction GameDataLoader::parseFaction(const std::string& value) {
    if (value == "enemies") return EntityFaction::Enemies;
    if (value == "neutral") return EntityFaction::Neutral;
    return EntityFaction::Players;
}

AbilityTargetType GameDataLoader::parseAbilityTarget(const std::string& value) {
    if (value == "self") return AbilityTargetType::Self;
    if (value == "ally") return AbilityTargetType::Ally;
    if (value == "area") return AbilityTargetType::Area;
    return AbilityTargetType::Enemy;
}

AbilityEffectType GameDataLoader::parseEffectType(const std::string& value) {
    if (value == "heal") return AbilityEffectType::Heal;
    if (value == "buff") return AbilityEffectType::Buff;
    if (value == "debuff") return AbilityEffectType::Debuff;
    if (value == "status") return AbilityEffectType::Status;
    return AbilityEffectType::Damage;
}

GameModeType GameDataLoader::parseGameMode(const std::string& value) {
    if (value == "free_for_all") return GameModeType::FreeForAll;
    if (value == "survival") return GameModeType::Survival;
    if (value == "points") return GameModeType::Points;
    return GameModeType::Cooperative;
}

ObjectiveType GameDataLoader::parseObjectiveType(const std::string& value) {
    if (value == "talk") return ObjectiveType::TalkToNpc;
    if (value == "collect") return ObjectiveType::CollectItem;
    if (value == "reach") return ObjectiveType::ReachTile;
    if (value == "survive") return ObjectiveType::SurviveTurns;
    return ObjectiveType::DefeatEnemies;
}

TileSpecialType GameDataLoader::parseSpecialType(const std::string& value) {
    if (value == "trap") return TileSpecialType::Trap;
    if (value == "heal") return TileSpecialType::Heal;
    if (value == "portal") return TileSpecialType::Portal;
    if (value == "objective") return TileSpecialType::Objective;
    if (value == "item") return TileSpecialType::Item;
    return TileSpecialType::None;
}

#ifndef GAMEDATALOADER_H
#define GAMEDATALOADER_H

#include <string>
#include "GameContent.h"
#include "SimpleJson.h"

class GameDataLoader {
public:
    GameDataLoader();
    bool loadFromFile(const std::string& path);
    const GameContent& getContent() const { return content; }

private:
    GameContent content;

    void loadTerrain(const SimpleJsonValue& root);
    void loadAbilities(const SimpleJsonValue& root);
    void loadItems(const SimpleJsonValue& root);
    void loadEntities(const SimpleJsonValue& root);
    void loadMaps(const SimpleJsonValue& root);

    static EntityKind parseEntityKind(const std::string& value);
    static EntityFaction parseFaction(const std::string& value);
    static AbilityTargetType parseAbilityTarget(const std::string& value);
    static AbilityEffectType parseEffectType(const std::string& value);
    static GameModeType parseGameMode(const std::string& value);
    static ObjectiveType parseObjectiveType(const std::string& value);
    static TileSpecialType parseSpecialType(const std::string& value);
};

#endif

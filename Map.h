#ifndef MAP_H
#define MAP_H

#include <SDL2/SDL.h>
#include <vector>
#include <unordered_map>
#include "GameContent.h"

struct TileData {
    TerrainTypeDefinition terrain;
    TileSpecialType specialType = TileSpecialType::None;
    SpecialTileDefinition special;
};

class Map {
public:
    Map();

    bool loadFromDefinition(const MapDefinition& definition,
                            const std::unordered_map<std::string, TerrainTypeDefinition>& terrainTypes);

    void drawMap(SDL_Renderer* renderer, bool drawGrid) const;
    void drawHighlights(SDL_Renderer* renderer, const std::vector<SDL_Point>& cells, const SDL_Color& color) const;

    bool isInside(int x, int y) const;
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    int getMovementCost(int x, int y) const;
    int getDefenseModifier(int x, int y) const;
    int getDodgeModifier(int x, int y) const;
    bool blocksMovement(int x, int y) const;
    bool blocksLineOfSight(int x, int y) const;

    TileSpecialType getSpecialType(int x, int y) const;
    SpecialTileDefinition getSpecialDefinition(int x, int y) const;
    void removeItemAt(int x, int y);

private:
    int width;
    int height;
    int tileSize;
    std::vector<std::vector<TileData>> tiles;
};

#endif // MAP_H

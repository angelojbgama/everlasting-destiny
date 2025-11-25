#include "Map.h"
#include <algorithm>

Map::Map() : width(0), height(0), tileSize(32) {}

bool Map::loadFromDefinition(const MapDefinition& definition,
                             const std::unordered_map<std::string, TerrainTypeDefinition>& terrainTypes) {
    width = definition.width;
    height = definition.height;
    tiles.clear();
    tiles.resize(height, std::vector<TileData>(width));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            TileData tile;
            std::string terrainId = "plain";
            if (y < static_cast<int>(definition.terrainIds.size()) &&
                x < static_cast<int>(definition.terrainIds[y].size())) {
                terrainId = definition.terrainIds[y][x];
            }
            auto terrainIt = terrainTypes.find(terrainId);
            if (terrainIt != terrainTypes.end()) {
                tile.terrain = terrainIt->second;
            } else {
                tile.terrain = terrainTypes.begin()->second;
            }
            tiles[y][x] = tile;
        }
    }

    for (const auto& special : definition.specials) {
        if (isInside(special.x, special.y)) {
            tiles[special.y][special.x].specialType = special.type;
            tiles[special.y][special.x].special = special;
        }
    }

    return true;
}

void Map::drawMap(SDL_Renderer* renderer, bool drawGrid) const {
    SDL_Rect rect = {0, 0, tileSize, tileSize};
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            rect.x = x * tileSize;
            rect.y = y * tileSize;
            const TileData& tile = tiles[y][x];
            SDL_SetRenderDrawColor(renderer, tile.terrain.color.r, tile.terrain.color.g, tile.terrain.color.b, 255);
            SDL_RenderFillRect(renderer, &rect);
            if (tile.specialType != TileSpecialType::None) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 60);
                SDL_RenderFillRect(renderer, &rect);
            }
            if (drawGrid) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &rect);
            }
        }
    }
}

void Map::drawHighlights(SDL_Renderer* renderer, const std::vector<SDL_Point>& cells, const SDL_Color& color) const {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (const SDL_Point& cell : cells) {
        if (!isInside(cell.x, cell.y)) {
            continue;
        }
        SDL_Rect rect = {cell.x * tileSize, cell.y * tileSize, tileSize, tileSize};
        SDL_RenderFillRect(renderer, &rect);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

bool Map::isInside(int x, int y) const {
    return x >= 0 && y >= 0 && x < width && y < height;
}

int Map::getMovementCost(int x, int y) const {
    if (!isInside(x, y)) return 9999;
    return std::max(1, tiles[y][x].terrain.movementCost);
}

int Map::getDefenseModifier(int x, int y) const {
    if (!isInside(x, y)) return 0;
    return tiles[y][x].terrain.defenseModifier;
}

int Map::getDodgeModifier(int x, int y) const {
    if (!isInside(x, y)) return 0;
    return tiles[y][x].terrain.dodgeModifier;
}

bool Map::blocksMovement(int x, int y) const {
    if (!isInside(x, y)) return true;
    return tiles[y][x].terrain.blocksMovement;
}

bool Map::blocksLineOfSight(int x, int y) const {
    if (!isInside(x, y)) return true;
    return tiles[y][x].terrain.blocksLineOfSight;
}

TileSpecialType Map::getSpecialType(int x, int y) const {
    if (!isInside(x, y)) return TileSpecialType::None;
    return tiles[y][x].specialType;
}

SpecialTileDefinition Map::getSpecialDefinition(int x, int y) const {
    if (!isInside(x, y)) return SpecialTileDefinition();
    return tiles[y][x].special;
}

void Map::removeItemAt(int x, int y) {
    if (!isInside(x, y)) return;
    tiles[y][x].specialType = TileSpecialType::None;
}

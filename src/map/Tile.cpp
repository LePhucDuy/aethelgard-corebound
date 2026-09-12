#include "map/Tile.h"

Tile::Tile(TileType type) : type(type), walkable(false), sourceRect{0, 0, 0, 0} {
    setType(type);
}

void Tile::setType(TileType newType) {
    type = newType;
    switch (type) {
        case TileType::EMPTY:
            walkable = false;
            sourceRect = Rectangle{ 0.0f, 0.0f, 0.0f, 0.0f };
            break;

        case TileType::FLOOR:
            // Bề mặt thảm cỏ xanh (x=32, y=16)
            walkable = true;
            sourceRect = Rectangle{ 32.0f, 16.0f, 16.0f, 16.0f };
            break;

        case TileType::WALL:
            // Lòng đất / vách đất ngầm (x=32, y=32)
            walkable = false;
            sourceRect = Rectangle{ 32.0f, 32.0f, 16.0f, 16.0f };
            break;

        case TileType::DOOR:
            walkable = true;
            sourceRect = Rectangle{ 48.0f, 16.0f, 16.0f, 16.0f };
            break;

        case TileType::STAIRS_DOWN:
            // Bệ đá cổ chuyển tầng (x=112, y=16)
            walkable = true;
            sourceRect = Rectangle{ 112.0f, 16.0f, 16.0f, 16.0f };
            break;

        case TileType::WATER:
            // Vực nước sâu nguy hiểm (vẽ hiệu ứng hoạt họa riêng trong Dungeon::render)
            walkable = false;
            sourceRect = Rectangle{ 0.0f, 0.0f, 0.0f, 0.0f };
            break;
    }
}

void Tile::setCustom(TileType newType, const Rectangle& srcRect, bool isWalkable) {
    type = newType;
    sourceRect = srcRect;
    walkable = isWalkable;
}

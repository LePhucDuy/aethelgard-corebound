#ifndef TILE_H
#define TILE_H

#include <raylib.h>

/**
 * @brief Định danh các loại ô địa hình 2D Side của Aethelgard.
 */
enum class TileType {
    EMPTY,          // Không khí / trời trống (để lộ cây cối và nền trời)
    FLOOR,          // Bề mặt cỏ / lối đi bệ đá (nhân vật đứng và bước đi)
    WALL,           // Khối đất đá ngầm / vách núi (chặn va chạm)
    DOOR,           // Cổng gỗ / cửa đá
    STAIRS_DOWN     // Bệ đá cổ chuyển tiếp xuống tầng sâu hơn
};

/**
 * @brief Lớp Tile đại diện cho một ô đơn lẻ trên bản đồ địa hình 2D Side.
 */
class Tile {
private:
    TileType type;
    bool walkable;
    Rectangle sourceRect; // Tọa độ cắt frame 16x16 từ tiles.png

public:
    explicit Tile(TileType type = TileType::EMPTY);

    TileType getType() const { return type; }
    void setType(TileType newType);
    void setCustom(TileType newType, const Rectangle& srcRect, bool isWalkable);

    bool isWalkable() const { return walkable; }
    void setWalkable(bool state) { walkable = state; }

    const Rectangle& getSourceRect() const { return sourceRect; }
    void setSourceRect(const Rectangle& rect) { sourceRect = rect; }
};

#endif // TILE_H

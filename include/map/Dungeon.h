#ifndef DUNGEON_H
#define DUNGEON_H

#include <vector>
#include <memory>
#include <string>
#include "map/Tile.h"
#include "entities/Monster.h"
#include "items/Item.h"
#include "core/Position.h"

// Forward declaration
class Player;

// Loại quái (định nghĩa đầy đủ trong systems/MonsterFactory.h)
enum class MonsterType;

/**
 * @brief Lớp Dungeon đại diện cho một tầng hầm ngục hoàn chỉnh.
 * 
 * ĐẶC ĐIỂM OOP:
 * 1. Quan hệ hợp thành (Composition):
 *    - Dungeon sở hữu ma trận 2D các Tile: `std::vector<std::vector<Tile>> grid;`
 *    - Dungeon sở hữu danh sách quái vật: `std::vector<std::unique_ptr<Monster>> monsters;`
 *    - Dungeon sở hữu danh sách vật phẩm rơi: `std::vector<std::unique_ptr<Item>> groundItems;`
 *    Khi tầng Dungeon bị reset hoặc sang tầng mới, toàn bộ các đối tượng bên trong tự động
 *    được dọn dẹp sạch sẽ nhờ con trỏ thông minh (Smart Pointers).
 * 
 * 2. Đa hình (Polymorphism):
 *    - `monsters` lưu trữ con trỏ lớp cha `std::unique_ptr<Monster>`, cho phép chứa lẫn lộn
 *      nhiều chủng loài quái khác nhau (`Boar`, `SmallBee`, `Snail`) và gọi hành vi `act()` qua dynamic binding.
 */
class Dungeon {
private:
    int width;
    int height;
    int floorLevel;
    Position playerStartPos;
    Position stairsPos;

    // Quan hệ hợp thành (Composition)
    std::vector<std::vector<Tile>> grid;
    std::vector<std::unique_ptr<Monster>> monsters;
    std::vector<std::unique_ptr<Item>> groundItems;

    // Trạng thái kịch bản màn chơi
    bool hasBossFlag;
    bool bossDefeated;

    void carveRoom(int x, int y, int w, int h);
    void carveCorridor(int x1, int y1, int x2, int y2);

public:
    Dungeon(int width = 30, int height = 20);
    ~Dungeon() = default;

    // Sinh tầng ngục ngẫu nhiên
    void generate(int floor);

    // Kiểm tra tính hợp lệ của ô
    bool isValidPos(const Position& pos) const;
    bool isWalkable(const Position& pos) const;
    bool isWater(const Position& pos) const;

    // Quản lý thực thể trên bản đồ
    Monster* getMonsterAt(const Position& pos);
    Item* getItemAt(const Position& pos);
    std::unique_ptr<Item> takeItemAt(const Position& pos);

    void removeDeadMonsters(Player& player);

    // ===== Kịch bản màn chơi: phân khu, spawn có kiểm tra, boss gate =====

    // Tên khu vực theo tọa độ x (4 khu: Trại - Rừng - Vách Đá - Đền Thờ)
    const char* getZoneName(int x) const;

    // Loại ô tại vị trí (EMPTY không khí / FLOOR cỏ / WALL đất / STAIRS_DOWN)
    TileType getTileType(const Position& pos) const;

    // Sinh quái tại vị trí mong muốn; tự dời sang ô hợp lệ gần nhất nếu ô gốc không
    // đi được (chống lỗi quái spawn chui vào lòng đất). Ong bay chấp nhận ô EMPTY.
    bool spawnMonster(MonsterType type, const Position& desiredPos);

    // Boss gate: có boss đang canh cổng / đã hạ boss chưa
    bool hasBoss() const { return hasBossFlag; }
    bool isBossDefeated() const { return bossDefeated; }
    void setBossDefeated(bool defeated) { bossDefeated = defeated; }
    Monster* getBossMonster() const;

    // Gọi sau khi dọn xác quái: trả về true đúng 1 lần khi phát hiện boss đã bị tiêu diệt
    bool checkBossDefeated();

    // Cập nhật và vẽ
    void update(float deltaTime, Player& player, std::vector<std::string>& combatLog);
    void renderBackground(Vector2 offset = {0.0f, 0.0f}) const;
    void render(Vector2 offset = {0.0f, 0.0f}) const;
    void renderMonsters(Vector2 offset = {0.0f, 0.0f}) const;
    void renderItems(Vector2 offset = {0.0f, 0.0f}) const;

    // Getters
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getFloorLevel() const { return floorLevel; }
    const Position& getPlayerStartPos() const { return playerStartPos; }
    const Position& getStairsPos() const { return stairsPos; }

    std::vector<std::unique_ptr<Monster>>& getMonsters() { return monsters; }
    const std::vector<std::unique_ptr<Monster>>& getMonsters() const { return monsters; }

    std::vector<std::unique_ptr<Item>>& getGroundItems() { return groundItems; }
    const std::vector<std::unique_ptr<Item>>& getGroundItems() const { return groundItems; }
};

#endif // DUNGEON_H

#ifndef MONSTER_FACTORY_H
#define MONSTER_FACTORY_H

#include <memory>
#include "entities/Monster.h"
#include "entities/Boar.h"
#include "entities/SmallBee.h"
#include "entities/Snail.h"

/**
 * @brief Định danh các loài quái vật trong game.
 * Khi muốn thêm quái mới, chỉ cần thêm 1 giá trị enum vào đây (ví dụ: SKELETON, GOBLIN).
 */
enum class MonsterType {
    BOAR,
    SMALL_BEE,
    SNAIL
    // Sau này thêm quái mới chỉ cần thêm 1 dòng ở đây
};

/**
 * @brief Mẫu thiết kế Factory Method (Factory Pattern)
 * Quản lý tập trung việc tạo lập đối tượng Monster.
 * 
 * Ưu điểm kiến trúc OOP (Open/Closed Principle):
 * - Giúp mở rộng thêm các loài quái mới cực kỳ dễ dàng mà không làm ảnh hưởng đến Dungeon hay GameEngine.
 * - Che giấu logic khởi tạo cụ thể đằng sau giao diện trừu tượng.
 */
class MonsterFactory {
public:
    // Tạo quái vật theo loại cụ thể
    static std::unique_ptr<Monster> create(MonsterType type, const Position& pos);

    // Tạo quái vật ngẫu nhiên cho màn chơi
    static std::unique_ptr<Monster> createRandom(const Position& pos);
};

#endif // MONSTER_FACTORY_H

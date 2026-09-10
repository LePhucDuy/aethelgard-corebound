#ifndef MONSTER_H
#define MONSTER_H

#include "entities/Entity.h"

// Forward declaration
class Player;

/**
 * @brief Lớp trừu tượng Monster (Abstract Base Class 2)
 * Kế thừa từ Entity (Inheritance: IS-A), làm lớp cha cho toàn bộ quái vật trong game.
 * 
 * ĐẶC ĐIỂM OOP:
 * 1. Tính trừu tượng (Abstraction):
 *    - Chứa 2 phương thức thuần ảo:
 *      + `virtual void act(Dungeon& dungeon) = 0;`: AI điều khiển hành vi của quái vật theo lượt.
 *      + `virtual void onDeath(Player& player) = 0;`: Xử lý phần thưởng kinh nghiệm, tiền vàng hoặc rớt đồ khi quái bị tiêu diệt.
 * 
 * 2. Tính đa hình (Polymorphism):
 *    - Danh sách quái vật trong tầng Dungeon được quản lý bằng con trỏ lớp cha:
 *      `std::vector<std::unique_ptr<Monster>> monsters;`
 *    - Trong mỗi vòng lặp lượt đi, Engine chỉ cần gọi:
 *      `monster->act(dungeon);`
 *      Hệ thống tự động liên kết động (Dynamic Binding) để gọi đúng hành vi riêng biệt
 *      của từng loài quái (Boar ủi húc, Bee bay né đòn, Snail rút vào vỏ thủ).
 */
class Monster : public Entity {
protected:
    int expReward;
    int goldReward;

public:
    Monster(const std::string& name, const Position& pos, int hp, int attack, int defense, 
            int expReward, int goldReward);
    virtual ~Monster() override = default;

    // Phương thức thuần ảo bắt buộc các quái vật cụ thể phải override
    void act(Dungeon& dungeon) override = 0;
    virtual void onDeath(Player& player) = 0;

    int getExpReward() const { return expReward; }
    int getGoldReward() const { return goldReward; }
};

#endif // MONSTER_H

#ifndef SNAIL_H
#define SNAIL_H

#include "entities/Monster.h"

/**
 * @brief Lớp Snail (Ốc sên thiết giáp) kế thừa từ Monster
 * Quái phòng thủ (Tanker): Giáp dày.
 * Kỹ năng đặc biệt: Khi máu xuống dưới 50%, kích hoạt chế độ "Rút vào vỏ" (Hide),
 * tăng đột biến giáp và chuyển sang hoạt họa Hide-Sheet.png!
 */
class Snail : public Monster {
private:
    bool isHiding;
    int turnsToMove; // Ốc sên bò chậm (2 lượt mới di chuyển 1 ô)

public:
    explicit Snail(const Position& pos);
    ~Snail() override = default;

    // Override phương thức thuần ảo từ Monster & Entity (Polymorphism)
    void act(Dungeon& dungeon) override;
    void onDeath(Player& player) override;

    // Override takeDamage để tự động kích hoạt kỹ năng Rút Vào Vỏ
    void takeDamage(int amount) override;

    bool getIsHiding() const { return isHiding; }
};

#endif // SNAIL_H

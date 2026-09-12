#ifndef FLYING_MONSTER_H
#define FLYING_MONSTER_H

#include "entities/Monster.h"

/**
 * @brief Lớp FlyingMonster kế thừa từ Monster (Kế thừa phân cấp - Hierarchical Inheritance)
 * Đại diện cho các loài quái vật bay lượn trên không (Ong bắp cày nhỏ - SmallBee).
 * Đặc điểm: Miễn nhiễm với đầm lầy, không rơi xuống hố, bay lơ lửng trên không.
 */
class FlyingMonster : public Monster {
public:
    FlyingMonster(const std::string& name, const Position& pos, int hp, int attack, int defense,
                  int expReward, int goldReward, int aggroRange = 5, int patrolRange = 3)
        : Monster(name, pos, hp, attack, defense, expReward, goldReward, aggroRange, patrolRange, true) {}

    virtual ~FlyingMonster() override = default;

    // Kiểm tra đặc tính quái bay
    bool isFlyingUnit() const { return true; }
};

#endif // FLYING_MONSTER_H


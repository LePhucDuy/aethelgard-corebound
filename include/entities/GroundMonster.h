#ifndef GROUND_MONSTER_H
#define GROUND_MONSTER_H

#include "entities/Monster.h"

/**
 * @brief Lớp GroundMonster kế thừa từ Monster (Kế thừa phân cấp - Hierarchical Inheritance)
 * Đại diện cho các loài quái vật di chuyển trên mặt đất (Heo rừng, Ốc sên).
 * Đặc điểm: Chịu ảnh hưởng trọng lực, phải có sàn đỡ dưới chân, chịu tác động của đầm lầy.
 */
class GroundMonster : public Monster {
public:
    GroundMonster(const std::string& name, const Position& pos, int hp, int attack, int defense,
                  int expReward, int goldReward, int aggroRange = 5, int patrolRange = 3)
        : Monster(name, pos, hp, attack, defense, expReward, goldReward, aggroRange, patrolRange, false) {}

    virtual ~GroundMonster() override = default;

    // Kiểm tra đặc tính quái bộ
    bool isGroundUnit() const { return true; }
};

#endif // GROUND_MONSTER_H


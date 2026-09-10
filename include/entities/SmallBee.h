#ifndef SMALL_BEE_H
#define SMALL_BEE_H

#include "entities/Monster.h"

/**
 * @brief Lớp SmallBee (Ong sát thủ) kế thừa từ Monster
 * Quái cơ động bay: tốc độ cao, né đòn tốt, gây sát thương độc chích.
 */
class SmallBee : public Monster {
private:
    int evasionChance; // Tỉ lệ phần trăm né đòn (%)

public:
    explicit SmallBee(const Position& pos);
    ~SmallBee() override = default;

    // Override phương thức thuần ảo từ Monster & Entity (Polymorphism)
    void act(Dungeon& dungeon) override;
    void onDeath(Player& player) override;

    // Override takeDamage để thể hiện khả năng né đòn của loài ong
    void takeDamage(int amount) override;
};

#endif // SMALL_BEE_H

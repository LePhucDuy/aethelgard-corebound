#ifndef BOAR_H
#define BOAR_H

#include "entities/GroundMonster.h"

/**
 * @brief Lớp Boar (Lợn rừng) kế thừa từ GroundMonster (Kế thừa đa mức)
 * Quái cận chiến hung hăng: máu trâu, ủi húc trực diện về phía người chơi.
 */
class Boar : public GroundMonster {
protected:
    Boar(const std::string& name, const Position& pos, int hp, int attack, int defense,
         int expReward, int goldReward, int aggroRange = 6, int patrolRange = 3)
        : GroundMonster(name, pos, hp, attack, defense, expReward, goldReward, aggroRange, patrolRange) {}

public:
    explicit Boar(const Position& pos);
    ~Boar() override = default;

    // Override phương thức thuần ảo từ Monster & Entity (Polymorphism)
    void act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) override;
    void onDeath(Player& player) override;
};

#endif // BOAR_H

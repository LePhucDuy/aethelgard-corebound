#ifndef BOAR_H
#define BOAR_H

#include "entities/Monster.h"

/**
 * @brief Lớp Boar (Lợn rừng) kế thừa từ Monster
 * Quái cận chiến hung hăng: máu trâu, ủi húc trực diện về phía người chơi.
 */
class Boar : public Monster {
public:
    explicit Boar(const Position& pos);
    ~Boar() override = default;

    // Override phương thức thuần ảo từ Monster & Entity (Polymorphism)
    void act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) override;
    void onDeath(Player& player) override;
};

#endif // BOAR_H

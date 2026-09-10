#ifndef WEAPON_H
#define WEAPON_H

#include "items/Item.h"

/**
 * @brief Lớp Weapon kế thừa Item
 * Đại diện cho vũ khí có thể trang bị để tăng sức tấn công (Attack).
 */
class Weapon : public Item {
private:
    int bonusAttack;

public:
    Weapon(const std::string& name, const std::string& description, int bonusAttack, const Position& pos = {0, 0});

    // Override phương thức thuần ảo từ lớp cha Item (Polymorphism)
    bool use(Player* target) override;

    int getBonusAttack() const { return bonusAttack; }
};

#endif // WEAPON_H

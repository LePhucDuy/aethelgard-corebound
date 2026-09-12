#include "items/Weapon.h"
#include "entities/Player.h"
#include <iostream>

Weapon::Weapon(const std::string& name, const std::string& description, int bonusAttack, const Position& pos)
    : Item(name, description, pos), bonusAttack(bonusAttack) {}
Weapon::Weapon(const std::string& name, const std::string& description, int bonusAttack, const Position& pos, const std::string& textureId)
    : Item(name, description, pos, textureId), bonusAttack(bonusAttack) {}

bool Weapon::use(Player* target) {
    if (!target) return false;

    // Tăng sức tấn công cho người chơi
    target->addAttack(bonusAttack);
    std::cout << "[Trang bị] " << target->getName() << " đã trang bị " << name 
              << ", tăng +" << bonusAttack << " ATK! (Tổng ATK: " << target->getAttack() << ")" << std::endl;
    return true;
}

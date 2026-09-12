#include "items/Armor.h"
#include "entities/Player.h"
#include <iostream>

Armor::Armor(const std::string& name, const std::string& description, int bonusDefense, 
             const Position& pos, const std::string& textureId)
    : Item(name, description, pos, textureId), bonusDefense(bonusDefense) {}

bool Armor::use(Player* target) {
    if (!target) return false;

    // Tăng chỉ số phòng thủ cho nhân vật
    target->addDefense(bonusDefense);
    std::cout << "[Trang bi] " << target->getName() << " da trang bi " << name 
              << ", tang +" << bonusDefense << " DEF! (Tong DEF: " << target->getDefense() << ")" << std::endl;
    return true;
}


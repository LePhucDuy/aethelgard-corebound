#include "items/Accessory.h"
#include "entities/Player.h"
#include <iostream>

Accessory::Accessory(const std::string& name, const std::string& description,
                     int bonusAttack, int bonusDefense, int bonusMaxHp,
                     const Position& pos, const std::string& textureId)
    : Item(name, description, pos, textureId),
      bonusAttack(bonusAttack), bonusDefense(bonusDefense), bonusMaxHp(bonusMaxHp) {}

bool Accessory::use(Player* target) {
    if (!target) return false;

    if (bonusAttack > 0) target->addAttack(bonusAttack);
    if (bonusDefense > 0) target->addDefense(bonusDefense);
    if (bonusMaxHp > 0) {
        target->setMaxHp(target->getMaxHp() + bonusMaxHp);
        target->heal(bonusMaxHp);
    }

    std::cout << "[Trang bi] " << target->getName() << " da deo " << name 
              << ", tang +" << bonusAttack << " ATK, +" << bonusDefense << " DEF, +" 
              << bonusMaxHp << " MaxHP! (Tong ATK: " << target->getAttack() 
              << ", DEF: " << target->getDefense() << ", HP: " << target->getHp() 
              << "/" << target->getMaxHp() << ")" << std::endl;
    return true;
}


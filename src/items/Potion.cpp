#include "items/Potion.h"
#include "entities/Player.h"
#include <iostream>

Potion::Potion(const std::string& name, const std::string& description, int healAmount, const Position& pos, const std::string& textureId)
    : Item(name, description, pos, textureId), healAmount(healAmount) {}

bool Potion::use(Player* target) {
    if (!target) return false;

    if (target->getHp() >= target->getMaxHp()) {
        std::cout << "[Vật phẩm] " << target->getName() << " đang đầy máu, không cần dùng " << name << "!" << std::endl;
        return false;
    }

    int oldHp = target->getHp();
    target->heal(healAmount);
    int healed = target->getHp() - oldHp;

    std::cout << "[Hồi phục] " << target->getName() << " đã uống " << name 
              << ", hồi phục +" << healed << " HP! (" << target->getHp() << "/" << target->getMaxHp() << ")" << std::endl;
    return true;
}

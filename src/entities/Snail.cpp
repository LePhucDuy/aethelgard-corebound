#include "entities/Snail.h"
#include "entities/Player.h"
#include <iostream>

Snail::Snail(const Position& pos)
    : Monster("Snail (Oc sen giap)", pos, 30, 4, 3, 30, 15),
      isHiding(false), turnsToMove(0) {
    setAnimation(std::make_unique<Animation>("snail_walk", 8, 48, 32, 0.14f));
}

void Snail::takeDamage(int amount) {
    Entity::takeDamage(amount);

    if (alive && hp <= (maxHp / 2) && !isHiding) {
        isHiding = true;
        defense += 5; // Tăng thêm 5 giáp (tổng 8 DEF)
        setAnimation(std::make_unique<Animation>("snail_hide", 8, 48, 32, 0.16f));
        std::cout << "[KY NANG] " << name << " tai " << pos 
                  << " da kich hoat [RUT VAO VO]! Thu minh trong vo va ngung tan cong!" << std::endl;
    }
}

void Snail::act(Dungeon& dungeon) {
    (void)dungeon;
    turnsToMove++;
    if (turnsToMove % 2 == 0) {
        std::cout << "[Snail AI] Oc sen bo cham rai tai " << pos << std::endl;
    }
}

void Snail::onDeath(Player& player) {
    std::cout << "[Ha guc] Snail da vo vo! Ban tang +" << expReward << " EXP va +" << goldReward << " Vang!" << std::endl;
    player.addExp(expReward);
    player.addGold(goldReward);
}

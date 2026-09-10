#include "entities/SmallBee.h"
#include "entities/Player.h"
#include <iostream>
#include <cstdlib>

SmallBee::SmallBee(const Position& pos)
    : Monster("Small Bee (Ong sat thu)", pos, 25, 10, 1, 15, 6),
      evasionChance(35) {
    // Khởi tạo animation bay cho Bee: 4 frames, 64x64
    setAnimation(std::make_unique<Animation>("bee_fly", 4, 64, 64, 0.10f));
}

void SmallBee::takeDamage(int amount) {
    if ((std::rand() % 100) < evasionChance) {
        std::cout << "[Ne don!] " << name << " tai " << pos << " da bay luon ne sach don danh!" << std::endl;
        return;
    }
    Entity::takeDamage(amount);
}

void SmallBee::act(Dungeon& dungeon) {
    (void)dungeon;
    std::cout << "[Bee AI] Ong bay vo ve tai " << pos << std::endl;
}

void SmallBee::onDeath(Player& player) {
    std::cout << "[Ha guc] Small Bee roi rung! Ban tang +" << expReward << " EXP va +" << goldReward << " Vang!" << std::endl;
    player.addExp(expReward);
    player.addGold(goldReward);
}

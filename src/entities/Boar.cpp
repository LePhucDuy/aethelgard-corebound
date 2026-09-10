#include "entities/Boar.h"
#include "entities/Player.h"
#include <iostream>

Boar::Boar(const Position& pos)
    : Monster("Boar (Lon rung)", pos, 45, 12, 3, 25, 10) {
    // Khởi tạo animation mặc định cho Boar: 6 frames, 48x32
    setAnimation(std::make_unique<Animation>("boar_walk", 6, 48, 32, 0.13f));
}

void Boar::act(Dungeon& dungeon) {
    (void)dungeon;
    std::cout << "[Boar AI] Lon rung gam ru tai " << pos << " chuan bi ui huc!" << std::endl;
}

void Boar::onDeath(Player& player) {
    std::cout << "[Ha guc] Boar da bi tieu diet! Ban tang +" << expReward << " EXP va +" << goldReward << " Vang!" << std::endl;
    player.addExp(expReward);
    player.addGold(goldReward);
}

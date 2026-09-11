#include "entities/Snail.h"
#include "entities/Player.h"
#include "systems/CombatSystem.h"
#include "map/Dungeon.h"
#include <iostream>

Snail::Snail(const Position& pos)
    : Monster("Snail (Oc sen giap)", pos, 30, 4, 3, 30, 15,
              /*aggroRange*/ 2, /*patrolRange*/ 1, /*flying*/ false),
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

void Snail::act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    Position pPos = player.getPosition();

    // 1. RÚT VÀO VỎ: không tấn công, hồi 2 HP mỗi lượt
    //    -> tạo áp lực buộc người chơi phải dồn sát thương liên tục thay vì chờ đợi
    if (isHiding) {
        heal(2);
        return;
    }

    // 2. PHẢN ĐÒN: chỉ tấn công khi người chơi chủ động đứng kề ngang
    bool adjacent = (pPos.y == pos.y) && std::abs(pPos.x - pos.x) == 1;
    if (adjacent) {
        CombatSystem::attack(*this, player, combatLog);
        return;
    }

    // 3. BÒ CHẬM RÃI: mỗi 3 lượt di chuyển 1 ô quanh điểm sinh (patrolRange = 1)
    turnsToMove++;
    if (turnsToMove % 3 == 0) {
        patrolStep(dungeon);
    }
}

void Snail::onDeath(Player& player) {
    std::cout << "[Ha guc] Snail da vo vo! Ban tang +" << expReward << " EXP va +" << goldReward << " Vang!" << std::endl;
    player.addExp(expReward);
    player.addGold(goldReward);
}

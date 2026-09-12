#include "entities/Snail.h"
#include "entities/Player.h"
#include "systems/CombatSystem.h"
#include "map/Dungeon.h"
#include <iostream>

Snail::Snail(const Position& pos)
    : Monster("Snail (Oc sen giap)", pos, 30, 4, 3, 30, 15,
              /*aggroRange*/ 2, /*patrolRange*/ 1, /*flying*/ false),
      isHiding(false), turnsToMove(0) {
    // Hoạt họa đa trạng thái: idle (bò ra), hide (rút vỏ), dead (chết)
    addAnimation("idle", std::make_unique<Animation>("snail_walk", 8, 48, 32, 0.18f, true));
    addAnimation("hide", std::make_unique<Animation>("snail_hide", 8, 48, 32, 0.16f, true));
    addAnimation("dead", std::make_unique<Animation>("snail_dead", 8, 48, 32, 0.10f, false));
    setState("idle");
}

void Snail::takeDamage(int amount) {
    Monster::takeDamage(amount);

    if (alive && hp <= (maxHp / 2) && !isHiding) {
        isHiding = true;
        defense += 5; // Tăng thêm 5 giáp (tổng 8 DEF)
        setState("hide");  // Chuyển sang animation rút vỏ
        std::cout << "[KY NANG] " << name << " tai " << pos
                  << " da kich hoat [RUT VAO VO]! Thu minh trong vo va ngung tan cong!" << std::endl;
    }
}

void Snail::act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;

    // 1. RÚT VÀO VỎ: không tấn công, hồi 2 HP định kỳ
    if (isHiding) {
        heal(2);
        actionTimer = 2.0f;
        return;
    }

    // 2. PHẢN ĐÒN / CẬN CHIẾN: đứng kề ngang người chơi
    bool adjacent = (std::abs(dx) <= 1 && dy == 0);
    if (adjacent && player.isAlive()) {
        faceTowards(pPos);
        if (attackCooldown <= 0.0f) {
            CombatSystem::attack(*this, player, combatLog);
            attackCooldown = 1.2f;
        }
        actionTimer = 0.5f;
        return;
    }

    // 3. NẾU NHÌN THẤY NGƯỜI CHƠI PHÍA TRƯỚC (tầm nhìn 3 ô, không quay lưng)
    if (canSeePlayer(dungeon, player)) {
        setState("idle");
        faceTowards(pPos);
        int step = (dx > 0) ? 1 : -1;
        tryStepTo(dungeon, Position(pos.x + step, pos.y), player);
        actionTimer = 0.9f;
        return;
    }

    // 4. TUẦN TRA BÒ CHẬM RÃI
    setState("idle");
    patrolStep(dungeon);
    actionTimer = 1.4f;
}

void Snail::onDeath(Player& player) {
    std::cout << "[Ha guc] Snail da vo vo! Ban tang +" << expReward << " EXP va +" << goldReward << " Vang!" << std::endl;
    player.addExp(expReward);
    player.addGold(goldReward);
}

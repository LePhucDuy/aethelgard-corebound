#include "entities/Boar.h"
#include "entities/Player.h"
#include "systems/CombatSystem.h"
#include "map/Dungeon.h"
#include <iostream>
#include <cstdlib>

Boar::Boar(const Position& pos)
    : Monster("Boar (Lon rung)", pos, 45, 12, 3, 25, 10,
              /*aggroRange*/ 6, /*patrolRange*/ 3, /*flying*/ false) {
    // Hoạt họa đa trạng thái: idle (đứng yên), walk (tuần tra), run (chạy/đuổi), dead (Hit-Vanish)
    addAnimation("idle", std::make_unique<Animation>("boar_idle", 4, 48, 32, 0.15f, true));
    addAnimation("walk", std::make_unique<Animation>("boar_walk", 6, 48, 32, 0.12f, true));
    addAnimation("run",  std::make_unique<Animation>("boar_run",  6, 48, 32, 0.09f, true));
    addAnimation("dead", std::make_unique<Animation>("boar_hit",  4, 48, 32, 0.06f, false));
    setState("idle");
}

void Boar::act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;
    bool adjacent = (std::abs(dx) <= 1 && std::abs(dy) <= 1);

    // 1. Nếu đang ở cự ly cận chiến: TẤN CÔNG
    if (adjacent && player.isAlive()) {
        faceTowards(pPos);
        setState("run");
        if (attackCooldown <= 0.0f) {
            turnCount++;
            int baseAtk = getAttack();
            if (turnCount % 4 == 0) {
                combatLog.push_back("[HUC!] Boar hung rap ha thap dau lao toi ban!");
                attack = baseAtk * 3 / 2;
            }
            CombatSystem::attack(*this, player, combatLog);
            attack = baseAtk;
            attackCooldown = 0.8f;
        }
        actionTimer = 0.35f;
        return;
    }

    // 2. Trạng thái TUẦN TRA (PATROL)
    if (aiState == MonsterAIState::PATROL) {
        if (canSeePlayer(dungeon, player)) {
            // Phát hiện người chơi trong tầm nhìn -> Báo động và chuyển sang lao tới tấn công!
            aiState = MonsterAIState::CHASE;
            isAlerted = true;
            faceTowards(pPos);
            setState("run");
            combatLog.push_back("[BAO DONG] Boar phat hien ban va gam len lao toi!");
            actionTimer = 0.15f;
            return;
        }

        // Người chơi ở sau lưng hoặc quá xa -> Tiếp tục tuần tra nhịp nhàng
        setState("walk");
        patrolStep(dungeon);
        actionTimer = 0.65f;
        return;
    }

    // 3. Trạng thái TRUY ĐUỔI (CHASE) - Lao tới tấn công
    if (aiState == MonsterAIState::CHASE) {
        // Mất dấu nếu khoảng cách vượt quá xa
        if (std::abs(dx) > aggroRange + 3 || std::abs(dy) > 2) {
            aiState = MonsterAIState::RETURNING;
            isAlerted = false;
            combatLog.push_back("Boar mat dau ban va nguoi ngoai quay ve.");
            actionTimer = 0.6f;
            return;
        }

        setState("run");
        faceTowards(pPos);
        int step = (dx > 0) ? 1 : -1;
        tryStepTo(dungeon, Position(pos.x + step, pos.y), player);
        actionTimer = 0.22f; // Bứt tốc lao nhanh 0.22s/bước
        return;
    }

    // 4. Trạng thái QUAY VỀ (RETURNING)
    if (aiState == MonsterAIState::RETURNING) {
        if (canSeePlayer(dungeon, player)) {
            aiState = MonsterAIState::CHASE;
            isAlerted = true;
            faceTowards(pPos);
            setState("run");
            actionTimer = 0.15f;
            return;
        }

        if (pos.x == homePos.x) {
            aiState = MonsterAIState::PATROL;
            setState("idle");
            actionTimer = 0.8f;
            return;
        }

        setState("walk");
        int step = (homePos.x > pos.x) ? 1 : -1;
        tryStepTo(dungeon, Position(pos.x + step, pos.y), player);
        actionTimer = 0.5f;
        return;
    }
}

void Boar::onDeath(Player& player) {
    std::cout << "[Ha guc] Boar da bi tieu diet! Ban tang +" << expReward << " EXP va +" << goldReward << " Vang!" << std::endl;
    player.addExp(expReward);
    player.addGold(goldReward);
}

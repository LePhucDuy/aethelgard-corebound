#include "entities/BoarKing.h"
#include "entities/Player.h"
#include "systems/CombatSystem.h"
#include "map/Dungeon.h"
#include "core/Constants.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>

BoarKing::BoarKing(const Position& pos)
    : Monster("Boar King (Chua Heo Rung)", pos, 130, 18, 6, 100, 50,
              /*aggroRange*/ 6, /*patrolRange*/ 3, /*flying*/ false),
      enraged(false) {
    // Boss dùng chung texture với Boar nhưng frame nhanh hơn (đe dọa hơn)
    addAnimation("idle", std::make_unique<Animation>("boar_idle", 4, 48, 32, 0.12f, true));
    addAnimation("walk", std::make_unique<Animation>("boar_walk", 6, 48, 32, 0.10f, true));
    addAnimation("run",  std::make_unique<Animation>("boar_run",  6, 48, 32, 0.08f, true));
    addAnimation("dead", std::make_unique<Animation>("boar_hit",  4, 48, 32, 0.05f, false));
    setState("idle");
}

void BoarKing::act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    // 1. CỰC GIẬN (một lần duy nhất khi HP <= 30%): tăng sức mạnh vĩnh viễn
    if (!enraged && hp <= maxHp * 3 / 10) {
        enraged = true;
        attack += 6;
        aggroRange = 8;
        combatLog.push_back("[CUC GIAN!] Boar King phat dien - ATK tang manh va tam nhin rong hon!");
    }

    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;
    bool adjacent = (std::abs(dx) <= 1 && std::abs(dy) <= 1);

    // 2. CẬN CHIẾN: đứng kề cạnh -> Húc dữ dội
    if (adjacent && player.isAlive()) {
        faceTowards(pPos);
        setState("run");
        if (attackCooldown <= 0.0f) {
            turnCount++;
            int baseAtk = getAttack();
            bool isSlam = (turnCount % 3 == 0);
            if (isSlam) combatLog.push_back("[LAN HUC!] Boar King phong sat thuong manh me!");
            if (isSlam) attack = baseAtk * 3 / 2;
            CombatSystem::attack(*this, player, combatLog);
            attack = baseAtk;
            attackCooldown = 0.7f;

            // Knockback: đẩy người chơi lùi 1 ô nếu ô phía sau trống
            if (isSlam) {
                int push = (pos.x > pPos.x) ? -1 : 1;
                Position back(pPos.x + push, pPos.y);
                if (dungeon.isWalkable(back) && dungeon.getMonsterAt(back) == nullptr && back != pos) {
                    player.setPosition(back);
                    combatLog.push_back("Ban bi Boar King huc bay lui 1 o!");
                }
            }
        }
        actionTimer = 0.3f;
        return;
    }

    // 3. Trạng thái TUẦN TRA (PATROL)
    if (aiState == MonsterAIState::PATROL) {
        if (canSeePlayer(dungeon, player)) {
            aiState = MonsterAIState::CHASE;
            isAlerted = true;
            faceTowards(pPos);
            setState("run");
            combatLog.push_back("[GAM RO!] Boar King phat hien ban va gao thet lao toi!");
            actionTimer = 0.15f;
            return;
        }

        setState("walk");
        patrolStep(dungeon);
        actionTimer = 0.7f;
        return;
    }

    // 4. Trạng thái TRUY ĐUỔI (CHASE)
    if (aiState == MonsterAIState::CHASE) {
        if (std::abs(dx) > aggroRange + 3 || std::abs(dy) > 2) {
            aiState = MonsterAIState::RETURNING;
            isAlerted = false;
            combatLog.push_back("Boar King nguoi con gian va quay ve be phong an.");
            actionTimer = 0.6f;
            return;
        }

        setState("run");
        faceTowards(pPos);
        turnCount++;
        int steps = (turnCount % 3 == 0) ? 2 : 1;
        for (int i = 0; i < steps; ++i) {
            int step = (pPos.x > pos.x) ? 1 : -1;
            if (!tryStepTo(dungeon, Position(pos.x + step, pos.y), player)) break;
        }
        actionTimer = 0.18f; // Rất nhanh khi lao tới
        return;
    }

    // 5. Trạng thái QUAY VỀ (RETURNING)
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

void BoarKing::onDeath(Player& player) {
    std::cout << "[BOSS HA GUC] Boar King da bi tieu diet!" << std::endl;
    player.addExp(expReward);
    player.addGold(goldReward);
}

void BoarKing::render(float scale, Vector2 offset) const {
    if (!currentAnim) return;

    // Boss to hơn (x1.35) và ánh vàng để phân biệt — neo chân đồng bộ quái
    // (FOOT_SINK 6 + TRIM 2, theo tỉ lệ scale boss s).
    float s = scale * 1.35f;
    float fWidth = (float)currentAnim->getFrameWidth() * s;
    float fHeight = (float)currentAnim->getFrameHeight() * s;
    constexpr float FOOT_SINK = 6.0f;
    constexpr float TRIM_BOTTOM = 2.0f;

    Vector2 screenPos = {
        visualPos.x + ((float)Constants::TILE_SIZE - fWidth) / 2.0f + offset.x,
        visualPos.y + FOOT_SINK + TRIM_BOTTOM * s - fHeight + offset.y
    };

    Color tint = Color{ 255, 225, 160, 255 };
    if (dying && currentAnim->getTotalFrames() > 1) {
        float progress = (float)currentAnim->getCurrentFrame() / (float)(currentAnim->getTotalFrames() - 1);
        if (progress > 1.0f) progress = 1.0f;
        tint.a = (unsigned char)(255.0f * (1.0f - progress));
    }

    currentAnim->draw(screenPos, s, tint);
}
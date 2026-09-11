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
    addAnimation("run",  std::make_unique<Animation>("boar_run",  6, 48, 32, 0.08f, true));
    addAnimation("dead", std::make_unique<Animation>("boar_hit",  4, 48, 32, 0.05f, false));
    setState("idle");
}

void BoarKing::act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    turnCount++;

    // 1. CỰC GIẢN (một lần duy nhất khi HP <= 30%): tăng sức mạnh vĩnh viễn
    if (!enraged && hp <= maxHp * 3 / 10) {
        enraged = true;
        attack += 6;
        aggroRange = 8;
        combatLog.push_back("[CUC GIAN!] Boar King phat dien - ATK tang manh va tam nhin rong hon!");
    }

    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    bool sameFloor = (pPos.y == pos.y);

    // 2. Đòn chốt khi đã kề cạnh: mỗi lượt thứ 3 là đòn HÚC x1.5 + đẩy lùi 1 ô
    if (sameFloor && std::abs(pPos.x - pos.x) == 1) {
        setState("run");  // Húc lao vào người
        int baseAtk = getAttack();
        bool isSlam = (turnCount % 3 == 0);
        if (isSlam) combatLog.push_back("[LAN HUC!] Boar King hung va phong sat thuong lien hoan!");
        if (isSlam) attack = baseAtk * 3 / 2;
        CombatSystem::attack(*this, player, combatLog);
        attack = baseAtk;

        // Knockback: đẩy người chơi lùi 1 ô (nếu ô phía sau trống)
        if (isSlam) {
            int push = (pos.x > pPos.x) ? 1 : -1;
            Position back(pPos.x + push, pPos.y);
            if (dungeon.isWalkable(back) && dungeon.getMonsterAt(back) == nullptr && back != pos) {
                player.setPosition(back);
                combatLog.push_back("Ban bi huc bay lui 1 o!");
            }
        }
        return;
    }

    // 3. ĐUỔI/TUẦN TRA: aggro 6 ô cùng tầng (8 khi Cực Giản)
    if (sameFloor && std::abs(dx) <= aggroRange && hasLineOfSight(dungeon, pPos)) {
        setState("run");  // Animation chạy khi đuổi
        faceTowards(pPos);
        // Mỗi lượt thứ 3: LÃO HÚC — lao tới tối đa 3 ô liên tiếp
        int steps = (turnCount % 3 == 0) ? 3 : 1;
        for (int i = 0; i < steps; ++i) {
            int step = (pPos.x > pos.x) ? 1 : -1;
            if (!tryStepTo(dungeon, Position(pos.x + step, pos.y), player)) break;
        }
        return;
    }

    // 4. Tuần tra quanh Cổng Cửa khi người chơi chưa tới gần
    setState("idle");  // Animation đứng gầm gừ
    patrolStep(dungeon);
}

void BoarKing::onDeath(Player& player) {
    std::cout << "[BOSS HA GUC] Boar King da bi tieu diet!" << std::endl;
    player.addExp(expReward);
    player.addGold(goldReward);
}

void BoarKing::render(float scale, Vector2 offset) const {
    if (!currentAnim) return;

    // Boss to hơn (x1.35) và ánh vàng để phân biệt
    float s = scale * 1.35f;
    float fWidth = (float)currentAnim->getFrameWidth() * s;
    float fHeight = (float)currentAnim->getFrameHeight() * s;

    Vector2 screenPos = {
        (float)(pos.x * Constants::TILE_SIZE) + ((float)Constants::TILE_SIZE - fWidth) / 2.0f + offset.x,
        (float)((pos.y + 1) * Constants::TILE_SIZE) - fHeight + 10.0f + offset.y
    };

    Color tint = Color{ 255, 225, 160, 255 };
    if (dying && currentAnim->getTotalFrames() > 1) {
        float progress = (float)currentAnim->getCurrentFrame() / (float)(currentAnim->getTotalFrames() - 1);
        if (progress > 1.0f) progress = 1.0f;
        tint.a = (unsigned char)(255.0f * (1.0f - progress));
    }

    currentAnim->draw(screenPos, s, tint);
}
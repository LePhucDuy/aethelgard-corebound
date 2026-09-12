#include "entities/SmallBee.h"
#include "entities/Player.h"
#include "systems/CombatSystem.h"
#include "map/Dungeon.h"
#include "core/Constants.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>

SmallBee::SmallBee(const Position& pos)
    : Monster("Small Bee (Ong sat thu)", pos, 25, 10, 1, 15, 6,
              /*aggroRange*/ 6, /*patrolRange*/ 2, /*flying*/ true),
      evasionChance(35) {
    // Hoạt họa đa trạng thái: idle (lượn lờ), run (bay đuổi), attack (lao chích), dead (Hit-Vanish)
    addAnimation("idle",   std::make_unique<Animation>("bee_fly",    4, 64, 64, 0.12f, true));
    addAnimation("run",    std::make_unique<Animation>("bee_fly",    4, 64, 64, 0.08f, true));
    addAnimation("attack", std::make_unique<Animation>("bee_attack", 4, 64, 64, 0.06f, false));
    addAnimation("dead",   std::make_unique<Animation>("bee_hit",    4, 64, 64, 0.06f, false));
    setState("idle");
}

void SmallBee::takeDamage(int amount) {
    if ((std::rand() % 100) < evasionChance) {
        std::cout << "[Ne don!] " << name << " tai " << pos << " da bay luon ne sach don danh!" << std::endl;
        return;
    }
    Monster::takeDamage(amount);
}

void SmallBee::act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;

    auto canFlyTo = [&](const Position& c) {
        return dungeon.isValidPos(c)
            && dungeon.getTileType(c) == TileType::EMPTY
            && dungeon.getMonsterAt(c) == nullptr
            && c != pPos;
    };

    // 1. TẤN CÔNG CHÍCH (ở vị trí chéo trên đầu người chơi: |dx| <= 1, dy == -1)
    if (std::abs(dx) <= 1 && dy == -1 && player.isAlive()) {
        faceTowards(pPos);
        if (attackCooldown <= 0.0f) {
            setState("attack");
            combatLog.push_back("[CHICH!] Small Bee lao toi chich roi bay lui!");
            CombatSystem::attack(*this, player, combatLog);
            attackCooldown = 1.0f;

            // Bay lùi xa người chơi ra 2 ô chéo phía trên
            int away = (dx > 0) ? 1 : -1;
            Position retreat(pos.x + away, pos.y - 1);
            if (canFlyTo(retreat)) setPosition(retreat);
        }
        actionTimer = 0.35f;
        return;
    }

    // 2. NẾU ĐANG TUẦN TRA (PATROL)
    if (aiState == MonsterAIState::PATROL) {
        if (canSeePlayer(dungeon, player)) {
            // Thấy người chơi ở đúng hướng bay -> Chuyển sang đuổi bắt
            aiState = MonsterAIState::CHASE;
            isAlerted = true;
            faceTowards(pPos);
            setState("run");
            combatLog.push_back("[VO VE] Small Bee phat hien ban va lao toi!");
            actionTimer = 0.15f;
            return;
        }

        // Tuần tra bay lượn trên không
        setState("idle");
        Position next(pos.x + patrolDir, pos.y);
        bool outOfRange = std::abs(next.x - homePos.x) > patrolRange;
        if (outOfRange || !canFlyTo(next)) {
            patrolDir = -patrolDir;
            setFacing(patrolDir > 0);
            pauseTimer = 0.6f;
            return;
        }
        setFacing(patrolDir > 0);
        setPosition(next);
        actionTimer = 0.5f;
        return;
    }

    // 3. NẾU ĐANG TRUY ĐUỔI (CHASE)
    if (aiState == MonsterAIState::CHASE) {
        int cheb = std::max(std::abs(dx), std::abs(dy));
        if (cheb > aggroRange + 3) {
            aiState = MonsterAIState::RETURNING;
            isAlerted = false;
            actionTimer = 0.6f;
            return;
        }

        setState("run");
        setFacing(pPos.x > pos.x);

        // Nhắm đến vị trí chéo trên đầu người chơi
        int side = (pos.x >= pPos.x) ? 1 : -1;
        Position strike(pPos.x + side, pPos.y - 1);

        int sdx = strike.x - pos.x;
        int sdy = strike.y - pos.y;
        Position step(pos.x + (sdx == 0 ? 0 : (sdx > 0 ? 1 : -1)),
                      pos.y + (sdy == 0 ? 0 : (sdy > 0 ? 1 : -1)));

        if (step != pos && canFlyTo(step)) {
            setPosition(step);
            actionTimer = 0.22f; // Bứt tốc bay nhanh
            return;
        }

        Position altX(pos.x + (sdx > 0 ? 1 : -1), pos.y);
        Position altY(pos.x, pos.y + (sdy > 0 ? 1 : -1));
        if (sdx != 0 && canFlyTo(altX)) setPosition(altX);
        else if (canFlyTo(altY)) setPosition(altY);
        actionTimer = 0.22f;
        return;
    }

    // 4. QUAY VỀ (RETURNING)
    if (aiState == MonsterAIState::RETURNING) {
        if (canSeePlayer(dungeon, player)) {
            aiState = MonsterAIState::CHASE;
            isAlerted = true;
            actionTimer = 0.15f;
            return;
        }

        if (std::abs(pos.x - homePos.x) <= 1 && std::abs(pos.y - homePos.y) <= 1) {
            aiState = MonsterAIState::PATROL;
            actionTimer = 0.6f;
            return;
        }

        int hdx = homePos.x - pos.x;
        int hdy = homePos.y - pos.y;
        Position step(pos.x + (hdx == 0 ? 0 : (hdx > 0 ? 1 : -1)),
                      pos.y + (hdy == 0 ? 0 : (hdy > 0 ? 1 : -1)));
        if (canFlyTo(step)) setPosition(step);
        actionTimer = 0.4f;
        return;
    }
}

void SmallBee::onDeath(Player& player) {
    std::cout << "[Ha guc] Small Bee roi rung! Ban tang +" << expReward << " EXP va +" << goldReward << " Vang!" << std::endl;
    player.addExp(expReward);
    player.addGold(goldReward);
}

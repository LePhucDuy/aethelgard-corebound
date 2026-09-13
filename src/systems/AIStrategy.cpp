#include "systems/AIStrategy.h"
#include "entities/Monster.h"
#include "entities/Player.h"
#include "map/Dungeon.h"
#include "systems/CombatSystem.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// =============================================================================
// 1. GROUND PATROL STRATEGY
// =============================================================================

void GroundPatrolStrategy::execute(Monster& monster, Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    if (!monster.isAlive()) return;

    Position pos = monster.getPosition();
    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;
    int dist = std::max(std::abs(dx), std::abs(dy));

    // A. Nếu ở cự ly cận chiến cùng tầng -> tấn công
    if (dist <= 1 && dy == 0 && player.isAlive()) {
        monster.faceTowards(pPos);
        monster.setState("attack");
        CombatSystem::attack(monster, player, combatLog);
        return;
    }

    // B. Kiểm tra phát hiện người chơi
    if (monster.canSeePlayer(dungeon, player)) {
        monster.faceTowards(pPos);
        monster.setState("run");
        int stepX = (dx > 0) ? 1 : -1;
        Position next(pos.x + stepX, pos.y);
        monster.tryStepTo(dungeon, next, player);
        return;
    }

    // C. Tuần tra đi lại bình thường
    monster.setState("run");
    monster.patrolStep(dungeon);
}

// =============================================================================
// 2. SWARM AERIAL STRATEGY
// =============================================================================

void SwarmAerialStrategy::execute(Monster& monster, Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    if (!monster.isAlive()) return;

    Position pos = monster.getPosition();
    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;

    auto canFlyTo = [&](const Position& c) {
        return dungeon.isValidPos(c)
            && dungeon.getTileType(c) == TileType::EMPTY
            && dungeon.getMonsterAt(c) == nullptr
            && c != pPos;
    };

    // A. Cự ly chọc nọc độc trên đỉnh đầu hoặc ngang ngực
    bool inStrikeRange = (std::abs(dx) <= 1 && dy >= -1 && dy <= 2);
    if (inStrikeRange && player.isAlive()) {
        monster.faceTowards(pPos);
        monster.setState("attack");
        combatLog.push_back("[" + monster.getName() + "] lao xuong chich noc doc!");
        CombatSystem::attack(monster, player, combatLog);
        return;
    }

    // B. Truy đuổi bổ nhào
    monster.setState("run");
    monster.faceTowards(pPos);

    Position targetSpots[] = {
        Position(pPos.x, pPos.y - 1),
        Position(pPos.x + (pos.x >= pPos.x ? 1 : -1), pPos.y - 1),
        Position(pPos.x, pPos.y - 2),
        Position(pPos.x - (pos.x >= pPos.x ? 1 : -1), pPos.y - 1)
    };

    Position bestSpot = targetSpots[0];
    for (const auto& ts : targetSpots) {
        if (canFlyTo(ts) || ts == pos) {
            bestSpot = ts;
            break;
        }
    }

    int tdx = bestSpot.x - pos.x;
    int tdy = bestSpot.y - pos.y;
    int stepX = (tdx == 0) ? 0 : (tdx > 0 ? 1 : -1);
    int stepY = (tdy == 0) ? 0 : (tdy > 0 ? 1 : -1);

    Position stepDiag(pos.x + stepX, pos.y + stepY);
    Position stepVert(pos.x, pos.y + stepY);
    Position stepHoriz(pos.x + stepX, pos.y);

    if (stepDiag != pos && canFlyTo(stepDiag)) {
        monster.setPosition(stepDiag);
    } else if (stepVert != pos && canFlyTo(stepVert)) {
        monster.setPosition(stepVert);
    } else if (stepHoriz != pos && canFlyTo(stepHoriz)) {
        monster.setPosition(stepHoriz);
    }
}

// =============================================================================
// 3. BOSS RAGE STRATEGY
// =============================================================================

void BossRageStrategy::execute(Monster& monster, Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    if (!monster.isAlive()) return;

    Position pos = monster.getPosition();
    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;
    int dist = std::max(std::abs(dx), std::abs(dy));

    monster.faceTowards(pPos);
    monster.setState("run");
    monster.setMoveLerpSpeed(16.0f); // Tốc độ di chuyển bão táp khi cuồng nộ

    // Nếu ở cận chiến -> đòn chém cuồng nộ bạo kích
    if (dist <= 1 && player.isAlive()) {
        monster.setState("attack");
        combatLog.push_back(">>> [" + monster.getName() + "] CUONG NO tung don huy diet! <<<");
        CombatSystem::attack(monster, player, combatLog);
        return;
    }

    // Lao trực diện rút ngắn khoảng cách với người chơi
    int stepX = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
    int stepY = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);

    if (monster.isFlying()) {
        Position nextAir(pos.x + stepX, pos.y + stepY);
        if (dungeon.isValidPos(nextAir) && dungeon.getTileType(nextAir) == TileType::EMPTY && nextAir != pPos) {
            monster.setPosition(nextAir);
        }
    } else {
        Position nextGround(pos.x + stepX, pos.y);
        monster.tryStepTo(dungeon, nextGround, player);
    }
}


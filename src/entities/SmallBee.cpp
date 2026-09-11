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
    turnCount++;
    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;

    // Ong CHỈ đứng trên ô không khí (EMPTY) — không bao giờ đáp xuống mặt cỏ
    auto canFlyTo = [&](const Position& c) {
        return dungeon.isValidPos(c)
            && dungeon.getTileType(c) == TileType::EMPTY
            && dungeon.getMonsterAt(c) == nullptr
            && c != pPos;
    };

    // 1. TẤN CÔNG CHIA GIAO (cheo phia tren): đứng chéo trên đầu người chơi
    //    (|dx|==1 && dy==-1) — vị trí này người chơi đánh phản đòn được qua
    //    ô chéo phía trước, nên cuộc đấu luôn công bằng (hit-and-run)
    if (std::abs(dx) == 1 && dy == -1) {
        setState("attack");  // Animation lao chích
        combatLog.push_back("[CHICH!] Small Bee lao toi chich mot don roi bay di!");
        CombatSystem::attack(*this, player, combatLog);

        // Bay lui xa người chơi ra 2 ô chéo phía trên
        int away = (dx > 0) ? 1 : -1;
        Position retreat(pos.x + away, pos.y - 1);
        if (canFlyTo(retreat)) setPosition(retreat);
        return;
    }

    // 2. ĐUỔI THEO: phát hiện người chơi trong bán kính aggro (quái bay nhìn mọi hướng)
    int cheb = std::max(std::abs(dx), std::abs(dy));
    if (cheb <= aggroRange) {
        setState("run");  // Animation bay nhanh khi đuổi
        setFacing(pPos.x > pos.x);

        // Nếu đang THẤP hơn người chơi -> ưu tiên bay lên trước
        if (dy > 0) {
            Position up(pos.x, pos.y - 1);
            if (canFlyTo(up)) { setPosition(up); return; }
        }

        // Nhắm đến vị trí chia giao chéo phía trên người chơi
        int side = (pos.x >= pPos.x) ? 1 : -1;
        Position strike(pPos.x + side, pPos.y - 1);

        int sdx = strike.x - pos.x;
        int sdy = strike.y - pos.y;
        Position step(pos.x + (sdx == 0 ? 0 : (sdx > 0 ? 1 : -1)),
                      pos.y + (sdy == 0 ? 0 : (sdy > 0 ? 1 : -1)));
        if (step != pos && canFlyTo(step)) { setPosition(step); return; }

        // Bị chắn: thử bay tách trục
        Position altX(pos.x + (sdx > 0 ? 1 : -1), pos.y);
        Position altY(pos.x, pos.y + (sdy > 0 ? 1 : -1));
        if (sdx != 0 && canFlyTo(altX)) setPosition(altX);
        else if (canFlyTo(altY)) setPosition(altY);
        return;
    }

    // 3. LƯỚN LỌ QUANH ĐIỂM SINH: bay theo hình vuông nhỏ 4 điểm
    setState("idle");  // Animation lượn lờ nhẹ nhàng
    static const Position hoverOffsets[4] = {
        Position(-1, 0), Position(0, -1), Position(1, 0), Position(0, 1)
    };
    const Position& off = hoverOffsets[turnCount % 4];
    Position target(homePos.x + off.x, homePos.y + off.y);
    if (canFlyTo(target)) {
        setFacing(target.x > pos.x);
        setPosition(target);
    }
}

void SmallBee::onDeath(Player& player) {
    std::cout << "[Ha guc] Small Bee roi rung! Ban tang +" << expReward << " EXP va +" << goldReward << " Vang!" << std::endl;
    player.addExp(expReward);
    player.addGold(goldReward);
}

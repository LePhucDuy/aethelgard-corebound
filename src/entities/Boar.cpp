#include "entities/Boar.h"
#include "entities/Player.h"
#include "systems/CombatSystem.h"
#include "map/Dungeon.h"
#include <iostream>
#include <cstdlib>

Boar::Boar(const Position& pos)
    : Monster("Boar (Lon rung)", pos, 45, 12, 3, 25, 10,
              /*aggroRange*/ 5, /*patrolRange*/ 3, /*flying*/ false) {
    // Hoạt họa đa trạng thái: idle (đứng yên), run (chạy/đuổi), dead (Hit-Vanish biến mất)
    addAnimation("idle", std::make_unique<Animation>("boar_idle", 4, 48, 32, 0.15f, true));
    addAnimation("run",  std::make_unique<Animation>("boar_run",  6, 48, 32, 0.10f, true));
    addAnimation("dead", std::make_unique<Animation>("boar_hit",  4, 48, 32, 0.06f, false));
    setState("idle");
}

void Boar::act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    turnCount++;
    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    bool sameFloor = (pPos.y == pos.y);

    // 1. CẬN CHIẾN: đứng kề ngang trên cùng tầng -> tấn công
    //    Mỗi lượt thứ 4 kích hoạt đòn [HÚC] mạnh hơn (x1.5 sát thương)
    if (sameFloor && std::abs(dx) == 1) {
        setState("run");  // Húc lao vào người: animation chạy nhanh
        int baseAtk = getAttack();
        if (turnCount % 4 == 0) {
            combatLog.push_back("[HUC!] Boar hung rap lai va lao ve phia ban!");
            attack = baseAtk * 3 / 2;
        }
        CombatSystem::attack(*this, player, combatLog);
        attack = baseAtk;
        return;
    }

    // 2. ĐUỔI THEO: phát hiện người chơi cùng tầng, trong tầm aggro và không bị tường chắn
    if (sameFloor && std::abs(dx) <= aggroRange && hasLineOfSight(dungeon, pPos)) {
        setState("run");  // Animation chạy khi đuổi theo
        faceTowards(pPos);
        int step = (dx > 0) ? 1 : -1;
        // Chỉ bước khi ô đích đi được, không đè ai và CÓ SÀN ĐỠ dưới chân (không lơ lửng)
        tryStepTo(dungeon, Position(pos.x + step, pos.y), player);
        return;
    }

    // 3. TUẦN TRA: đi qua lại quanh điểm sinh khi không thấy người chơi
    setState("idle");  // Animation đứng yên khi tuần tra (hoặc có thể dùng idle qua lại)
    patrolStep(dungeon);
}

void Boar::onDeath(Player& player) {
    std::cout << "[Ha guc] Boar da bi tieu diet! Ban tang +" << expReward << " EXP va +" << goldReward << " Vang!" << std::endl;
    player.addExp(expReward);
    player.addGold(goldReward);
}

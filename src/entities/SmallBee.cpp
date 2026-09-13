#include "entities/SmallBee.h"
#include "entities/Player.h"
#include "systems/CombatSystem.h"
#include "map/Dungeon.h"
#include "core/Constants.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>

SmallBee::SmallBee(const Position& pos)
    : FlyingMonster("Small Bee (Ong sat thu)", pos, /*hp*/ 45, /*atk*/ 16, /*def*/ 3,
                    /*expReward*/ 25, /*goldReward*/ 15,
                    /*aggroRange*/ 8, /*patrolRange*/ 2),
      evasionChance(30),
      isAggro(false) {
    // Hoạt họa đa trạng thái theo yêu cầu:
    // - Bay lượn bình thường / truy đuổi: bee_fly (Fly-Sheet.png)
    // - Tấn công người chơi: bee_attack (Attack-Sheet.png)
    // - Khi bị đánh trúng / chết: bee_hit (Hit-Sheet.png)
    addAnimation("idle",   std::make_unique<Animation>("bee_fly",    4, 64, 64, 0.12f, true));
    addAnimation("run",    std::make_unique<Animation>("bee_fly",    4, 64, 64, 0.08f, true));
    addAnimation("attack", std::make_unique<Animation>("bee_attack", 4, 64, 64, 0.08f, false));
    addAnimation("hit",    std::make_unique<Animation>("bee_hit",    4, 64, 64, 0.08f, false));
    addAnimation("dead",   std::make_unique<Animation>("bee_hit",    4, 64, 64, 0.06f, false));
    setState("idle");
}

void SmallBee::takeDamage(int amount) {
    // 1. Khả năng né đòn của loài ong
    if ((std::rand() % 100) < evasionChance) {
        std::cout << "[Ne don!] " << name << " tai " << pos << " da bay luon ne sach don danh!" << std::endl;
        return;
    }

    // 2. KHI BỊ ĐÁNH TRÚNG: Kích hoạt trạng thái phản công (Revenge Aggro)
    isAggro = true;
    isAlerted = true;
    aiState = MonsterAIState::CHASE;
    std::cout << "[ONG PHAN KICH] " << name << " tai " << pos
              << " bi danh trung! Noi gian kich hoat phan cong!" << std::endl;

    // Monster::takeDamage tự động trừ HP, chuyển state "hit", và gọi kill() nếu HP <= 0
    Monster::takeDamage(amount);
}

void SmallBee::act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    if (dying || !alive) return;

    // A. Nếu đang phát hoạt ảnh bị đánh (hit) thì giữ nguyên cho animation chạy trọn vẹn
    if (animState == "hit" && currentAnim && !currentAnim->hasFinished()) {
        return;
    }

    // B. Nếu đang phát hoạt ảnh tấn công (attack) thì chờ chạy hết đòn chích
    if (animState == "attack" && currentAnim && !currentAnim->hasFinished()) {
        return;
    }

    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;
    bool inMeleeRange = (std::abs(dx) <= 1 && std::abs(dy) <= 1);

    auto canFlyTo = [&](const Position& c) {
        return dungeon.isValidPos(c)
            && dungeon.getTileType(c) == TileType::EMPTY
            && dungeon.getMonsterAt(c) == nullptr
            && c != pPos;
    };

    // -------------------------------------------------------------------------
    // TRƯỜNG HỢP 1: CHƯA BỊ ĐÁNH TRÚNG (!isAggro) -> HOÀN TOÀN HÒA BÌNH TUẦN TRA
    // Quái ong chỉ bay lượn lơ lửng, KHÔNG chủ động tấn công dù người chơi ở cạnh
    // -------------------------------------------------------------------------
    if (!isAggro) {
        setState("idle");
        setMoveLerpSpeed(4.5f);

        Position next(pos.x + patrolDir, pos.y);
        bool outOfRange = std::abs(next.x - homePos.x) > patrolRange;
        if (outOfRange || !canFlyTo(next)) {
            patrolDir = -patrolDir;
            setFacing(patrolDir > 0);
            pauseTimer = 0.5f;
            return;
        }
        setFacing(patrolDir > 0);
        setPosition(next);
        actionTimer = 0.45f;
        return;
    }

    // -------------------------------------------------------------------------
    // TRƯỜNG HỢP 2: ĐÃ BỊ ĐÁNH TRÚNG (isAggro == true) -> PHẢN CÔNG & TRUY ĐUỔI
    // -------------------------------------------------------------------------

    // 2.1. Đòn tấn công chích nọc độc khi ở cạnh người chơi (Khoảng cách <= 1 ô)
    if (inMeleeRange && player.isAlive()) {
        faceTowards(pPos);
        if (attackCooldown <= 0.0f) {
            setState("attack");
            combatLog.push_back("[CHICH NOC!] Small Bee hung han lao toi chich noc doc!");
            CombatSystem::attack(*this, player, combatLog);
            attackCooldown = 1.0f;
            actionTimer = 0.32f; // Giữ trạng thái cho hoạt ảnh chích Attack-Sheet hoàn thành

            // Sau khi chích, bay lùi nhẹ 1 ô tạo khoảng cách lượn
            int awayX = (dx > 0) ? -1 : (dx < 0 ? 1 : 0);
            Position retreat(pos.x + awayX, pos.y - 1);
            if (canFlyTo(retreat)) {
                setPosition(retreat);
            }
            return;
        }
        actionTimer = 0.25f;
        return;
    }

    // 2.2. Nếu người chơi chạy quá xa -> bỏ truy đuổi, quay về vị trí ban đầu
    int cheb = std::max(std::abs(dx), std::abs(dy));
    if (cheb > aggroRange + 5) {
        aiState = MonsterAIState::RETURNING;
        isAlerted = false;
        isAggro = false; // Bình tĩnh lại sau khi người chơi đã thoát xa
        actionTimer = 0.5f;
        return;
    }

    // 2.3. Đang truy đuổi (CHASE): bay nhanh áp sát người chơi
    if (aiState == MonsterAIState::CHASE) {
        setState("run");
        setMoveLerpSpeed(13.0f);
        setFacing(pPos.x > pos.x);

        // Mục tiêu bay: áp sát quanh người chơi (ưu tiên phía trên đầu hoặc ngang hông)
        int side = (pos.x >= pPos.x) ? 1 : -1;
        Position targetSpot(pPos.x + side, pPos.y - 1);
        if (!canFlyTo(targetSpot)) {
            targetSpot = Position(pPos.x + side, pPos.y);
        }

        int tdx = targetSpot.x - pos.x;
        int tdy = targetSpot.y - pos.y;
        Position step(pos.x + (tdx == 0 ? 0 : (tdx > 0 ? 1 : -1)),
                      pos.y + (tdy == 0 ? 0 : (tdy > 0 ? 1 : -1)));

        if (step != pos && canFlyTo(step)) {
            setPosition(step);
            actionTimer = 0.18f;
            return;
        }

        Position altX(pos.x + (tdx > 0 ? 1 : -1), pos.y);
        Position altY(pos.x, pos.y + (tdy > 0 ? 1 : -1));
        if (tdx != 0 && canFlyTo(altX)) setPosition(altX);
        else if (canFlyTo(altY)) setPosition(altY);
        actionTimer = 0.20f;
        return;
    }

    // 2.4. Đang quay về tổ (RETURNING)
    if (aiState == MonsterAIState::RETURNING) {
        setState("idle");
        setMoveLerpSpeed(6.0f);
        if (std::abs(pos.x - homePos.x) <= 1 && std::abs(pos.y - homePos.y) <= 1) {
            aiState = MonsterAIState::PATROL;
            actionTimer = 0.5f;
            return;
        }

        int hdx = homePos.x - pos.x;
        int hdy = homePos.y - pos.y;
        Position step(pos.x + (hdx == 0 ? 0 : (hdx > 0 ? 1 : -1)),
                      pos.y + (hdy == 0 ? 0 : (hdy > 0 ? 1 : -1)));
        if (canFlyTo(step)) setPosition(step);
        actionTimer = 0.35f;
        return;
    }
}

void SmallBee::onDeath(Player& player) {
    std::cout << "[Ha guc] Small Bee roi rung! Ban tang +" << expReward << " EXP va +" << goldReward << " Vang!" << std::endl;
    player.addExp(expReward);
    if (!goldDropped) {
        player.addGold(goldReward);
    }
}

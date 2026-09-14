#include "entities/SmallBee.h"
#include "entities/Player.h"
#include "systems/CombatSystem.h"
#include "map/Dungeon.h"
#include "core/Constants.h"
#include "core/Templates.h"
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
    // Ong lập tức thù địch, chuyển AI sang truy sát người chơi đến cùng
    isAggro = true;
    isAlerted = true;
    aiState = MonsterAIState::CHASE;
    std::cout << "[ONG PHAN KICH] " << name << " tai " << pos
              << " bi danh trung! Noi gian kich hoat phan cong!" << std::endl;

    FlyingMonster::takeDamage(amount);
}

void SmallBee::act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    if (!alive) return;

    if (actionTimer > 0.0f) {
        actionTimer -= 0.016f;
        return;
    }

    if (attackCooldown > 0.0f) {
        attackCooldown -= 0.016f;
    }

    // Nếu đang trong hoạt ảnh tấn công (chích), đợi hoạt ảnh chạy xong mới di chuyển tiếp
    if (animState == "attack" && currentAnim && !currentAnim->hasFinished()) {
        return;
    }

    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;

    // Quái ong bay trên không, người chơi cao 2.5 ô (chân ở y, thân ở y-1, đầu ở y-2)
    // Cự ly chích nọc độc: khi ong ở ngang đầu, ngang ngực hoặc chân người chơi (dy từ -1 đến 2, |dx| <= 1)
    bool inStrikeRange = (std::abs(dx) <= 1 && CoreTemplates::isInRange(dy, -1, 2));

    auto canFlyTo = [&](const Position& c) {
        return dungeon.isValidPos(c)
            && dungeon.getTileType(c) == TileType::EMPTY
            && dungeon.getMonsterAt(c) == nullptr
            && c != pPos;
    };

    // -------------------------------------------------------------------------
    // TRƯỜNG HỢP 1: CHƯA BỊ KÍCH HOẠT THÙ ĐỊCH (!isAggro)
    // Tự động phát hiện người chơi khi vào tầm nhìn HOẶC khi người chơi tới gần (<= 8 ô)
    // -------------------------------------------------------------------------
    if (!isAggro) {
        bool playerDetected = canSeePlayer(dungeon, player) || (std::abs(dx) <= 8 && std::abs(dy) <= 6);
        if (playerDetected) {
            isAggro = true;
            isAlerted = true;
            aiState = MonsterAIState::CHASE;
            faceTowards(pPos);
            combatLog.push_back("[VO VE!] Small Bee phat hien ban va lao toi chich!");

            // Nếu người chơi đã đứng ngay trong tầm chích -> tấn công lập tức!
            if (inStrikeRange && player.isAlive() && attackCooldown <= 0.0f) {
                setState("attack");
                combatLog.push_back("[CHICH NOC!] Small Bee lao toi chich noc doc!");
                CombatSystem::attack(*this, player, combatLog);
                attackCooldown = 0.9f;
                actionTimer = 0.32f;
                return;
            }

            setState("run");
            actionTimer = 0.08f;
            return;
        }

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
    // TRƯỜNG HỢP 2: ĐANG TRUY ĐUỔI / PHẢN CÔNG (isAggro == true)
    // -------------------------------------------------------------------------

    // 2.1. TẤN CÔNG CHÍCH NỌC ĐỘC NGAY KHI VÀO TẦM ĐÁNH (TRÊN ĐẦU HOẶC BÊN CẠNH NGƯỜI CHƠI)
    if (inStrikeRange && player.isAlive()) {
        faceTowards(pPos);
        if (attackCooldown <= 0.0f) {
            setState("attack");
            combatLog.push_back("[CHICH NOC!] Small Bee lao toi chich noc doc!");
            CombatSystem::attack(*this, player, combatLog);
            attackCooldown = 0.9f;
            actionTimer = 0.32f; // Giữ trạng thái cho hoạt ảnh chích Attack-Sheet hoàn thành
            return;
        }
        actionTimer = 0.18f;
        return;
    }

    // 2.2. Nếu người chơi chạy quá xa -> bỏ truy đuổi, quay về vị trí ban đầu
    int cheb = CoreTemplates::calculateChebyshevDistance(*this, player);
    if (cheb > aggroRange + 8) {
        aiState = MonsterAIState::RETURNING;
        isAlerted = false;
        isAggro = false; // Bình tĩnh lại sau khi người chơi đã thoát xa
        actionTimer = 0.5f;
        return;
    }

    // 2.3. Đang truy đuổi (CHASE): bay bổ nhào trực diện áp sát đỉnh đầu / thân người chơi
    if (aiState == MonsterAIState::CHASE) {
        setState("run");
        setMoveLerpSpeed(13.0f);
        setFacing(pPos.x > pos.x);

        // Danh sách các điểm tiếp cận quanh người chơi (đỉnh đầu, thân trên hai bên)
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

        // Thử bước di chuyển bổ nhào trực tiếp
        Position stepDiag(pos.x + stepX, pos.y + stepY);
        Position stepVert(pos.x, pos.y + stepY);
        Position stepHoriz(pos.x + stepX, pos.y);

        if (stepDiag != pos && canFlyTo(stepDiag)) {
            setPosition(stepDiag);
        } else if (stepVert != pos && canFlyTo(stepVert)) {
            setPosition(stepVert);
        } else if (stepHoriz != pos && canFlyTo(stepHoriz)) {
            setPosition(stepHoriz);
        }

        actionTimer = 0.15f;
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

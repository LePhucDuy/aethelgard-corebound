#include "entities/QueenBee.h"
#include "entities/Player.h"
#include "entities/SmallBee.h"
#include "systems/MonsterFactory.h"
#include "systems/CombatSystem.h"
#include "map/Dungeon.h"
#include "core/Constants.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <algorithm>

QueenBee::QueenBee(const Position& pos)
    : FlyingMonster("Queen Bee (Hoang Hau Ong Chua)", pos, /*hp*/ 280, /*atk*/ 26, /*def*/ 7,
                    /*expReward*/ 220, /*goldReward*/ 120,
                    /*aggroRange*/ 14, /*patrolRange*/ 4),
      enraged(false),
      swarmSummoned(false),
      stingCooldown(1.5f),
      rushCooldown(4.0f),
      specialTimer(0.0f),
      evasionChance(25),
      isRushing(false),
      rushDirX(0),
      rushTargetX(pos.x) {
    // Hoạt họa Ong Chúa dùng bộ spritesheet ong chất lượng cao
    addAnimation("idle",   std::make_unique<Animation>("bee_fly",    4, 64, 64, 0.11f, true));
    addAnimation("run",    std::make_unique<Animation>("bee_fly",    4, 64, 64, 0.07f, true));
    addAnimation("attack", std::make_unique<Animation>("bee_attack", 4, 64, 64, 0.07f, false));
    addAnimation("hit",    std::make_unique<Animation>("bee_hit",    4, 64, 64, 0.07f, false));
    addAnimation("dead",   std::make_unique<Animation>("bee_hit",    4, 64, 64, 0.05f, false));
    setState("idle");
    setMoveLerpSpeed(7.0f);
}

void QueenBee::triggerEnrage() {
    if (!enraged) {
        enraged = true;
        attack += 6;   // Tăng từ 26 -> 32 ATK
        defense += 2;  // Tăng từ 7 -> 9 DEF
        setMoveLerpSpeed(12.0f);
        std::cout << "[BOSS NOI GIAN] " << name << " PHAT NO! Suc manh va toc do bay tang vuot troi!" << std::endl;
    }
}

void QueenBee::update(float deltaTime) {
    Monster::update(deltaTime);
    if (!currentAnim) return;

    if (stingCooldown > 0.0f) stingCooldown -= deltaTime;
    if (rushCooldown > 0.0f) rushCooldown -= deltaTime;
    if (specialTimer > 0.0f) specialTimer -= deltaTime;

    // Kiểm tra kích hoạt Cuồng Nộ khi máu giảm sâu (< 50%)
    if (alive && hp <= (maxHp / 2) && !enraged) {
        triggerEnrage();
    }
}

void QueenBee::takeDamage(int amount) {
    if (dying) return;

    // 1. Tỉ lệ né đòn phản xạ hoàng gia
    if ((std::rand() % 100) < evasionChance && !isRushing) {
        std::cout << "[NE DON HOANG GIA!] " << name << " chao lieng ne thoat don tan cong!" << std::endl;
        return;
    }

    // 2. Chịu sát thương
    Monster::takeDamage(amount);

    if (alive && hp <= (maxHp / 2) && !enraged) {
        triggerEnrage();
    }
}

void QueenBee::act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    if (dying || !alive) return;

    // Nếu đang phát hoạt ảnh bị đánh hoặc tấn công thì chờ trọn vẹn animation
    if (animState == "hit" && currentAnim && !currentAnim->hasFinished()) {
        return;
    }
    if (animState == "attack" && currentAnim && !currentAnim->hasFinished()) {
        return;
    }

    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;
    int cheb = std::max(std::abs(dx), std::abs(dy));

    auto canFlyTo = [&](const Position& c) {
        return dungeon.isValidPos(c)
            && dungeon.getTileType(c) == TileType::EMPTY
            && dungeon.getMonsterAt(c) == nullptr
            && c != pPos;
    };

    // =========================================================================
    // KỸ NĂNG 1: TRIỆU HỒI ĐÀN TIỂU ONG HỘ VỆ (SUMMON SWARM khi HP < 50%)
    // =========================================================================
    if (hp <= (maxHp * 5 / 10) && !swarmSummoned) {
        swarmSummoned = true;
        combatLog.push_back(">>> [TRIEU HOI HOANG GIA!] Queen Bee ru len mot hoi! Dan ong ho ve xuat hien tro chien! <<<");
        std::cout << "[BOSS KY NANG] Queen Bee trieu hoi ong ho ve!" << std::endl;

        // Triệu hồi 2 chú ong phụ tá ở hai bên
        Position s1(pos.x - 2, pos.y);
        Position s2(pos.x + 2, pos.y);
        if (canFlyTo(s1)) {
            dungeon.spawnMonster(MonsterType::SMALL_BEE, s1);
        }
        if (canFlyTo(s2)) {
            dungeon.spawnMonster(MonsterType::SMALL_BEE, s2);
        }
        setState("idle");
        actionTimer = 0.5f;
        return;
    }

    // =========================================================================
    // KỸ NĂNG 2: ĐÒN CHÍCH NỌC ĐỘC HOÀNG KIM (Cận chiến <= 1 ô)
    // =========================================================================
    if (cheb <= 1 && player.isAlive()) {
        faceTowards(pPos);
        if (stingCooldown <= 0.0f) {
            setState("attack");
            int dmgBonus = enraged ? 6 : 0;
            attack += dmgBonus;

            combatLog.push_back("[NOC DOC HOANG GIA!] Queen Bee chich mui kim doc khong lo vao ban!");
            CombatSystem::attack(*this, player, combatLog);

            attack -= dmgBonus;
            stingCooldown = enraged ? 1.0f : 1.6f;
            actionTimer = 0.35f;

            // Bay lùi lượn vòng tạo khoảng cách
            int retreatX = (dx > 0) ? -1 : (dx < 0 ? 1 : 0);
            Position retreatPos(pos.x + retreatX, pos.y - 1);
            if (canFlyTo(retreatPos)) {
                setPosition(retreatPos);
            }
            return;
        }
        actionTimer = 0.25f;
        return;
    }

    // =========================================================================
    // KỸ NĂNG 3: PHI THÂN LAO NỌC THẦN TỐC (QUEEN'S RUSH ở khoảng cách 3 - 7 ô)
    // =========================================================================
    if (cheb >= 3 && cheb <= 7 && rushCooldown <= 0.0f && player.isAlive()) {
        faceTowards(pPos);
        setState("run");
        setMoveLerpSpeed(18.0f); // Tốc độ lao cực đại

        combatLog.push_back("[PHI THAN LAO NOC!] Queen Bee lao vut toi nhu sam set!");
        rushCooldown = enraged ? 3.5f : 5.0f;

        // Tiến sát vào vị trí trên đầu người chơi
        Position dashTarget(pPos.x + (dx > 0 ? -1 : 1), pPos.y - 1);
        if (!canFlyTo(dashTarget)) {
            dashTarget = Position(pPos.x + (dx > 0 ? -1 : 1), pPos.y);
        }

        if (canFlyTo(dashTarget)) {
            setPosition(dashTarget);
        }
        actionTimer = 0.28f;
        return;
    }

    // =========================================================================
    // HÀNH VI TRUY ĐUỔI HOẶC TUẦN TRA TRÊN KHÔNG
    // =========================================================================
    if (cheb <= aggroRange) {
        // Đang giao chiến: truy đuổi áp sát
        setState("run");
        setMoveLerpSpeed(enraged ? 14.0f : 9.0f);
        setFacing(pPos.x > pos.x);

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
            actionTimer = 0.20f;
            return;
        }

        Position altX(pos.x + (tdx > 0 ? 1 : -1), pos.y);
        Position altY(pos.x, pos.y + (tdy > 0 ? 1 : -1));
        if (tdx != 0 && canFlyTo(altX)) setPosition(altX);
        else if (canFlyTo(altY)) setPosition(altY);
        actionTimer = 0.22f;
        return;
    } else {
        // Tuần tra lượn lờ thanh lịch trên bầu trời vực đá
        setState("idle");
        setMoveLerpSpeed(5.0f);

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
        actionTimer = 0.45f;
        return;
    }
}

void QueenBee::onDeath(Player& player) {
    std::cout << "[DAI BOSS HA GUC] Queen Bee - Hoang Hau Ong Chua da guc nga duoi luoi kiem cua ban!" << std::endl;
    player.addExp(expReward);
    if (!goldDropped) {
        player.addGold(goldReward);
    }
}

void QueenBee::render(float scale, Vector2 offset) const {
    if (!currentAnim) return;

    // Ong Chúa có kích thước đồ sộ hoàng tộc: scale * 1.5f (tổng 2.7x)
    float s = scale * 1.5f;
    float fWidth = (float)currentAnim->getFrameWidth() * s;
    float fHeight = (float)currentAnim->getFrameHeight() * s;

    Vector2 screenPos = {
        visualPos.x + ((float)Constants::TILE_SIZE - fWidth) / 2.0f + offset.x,
        visualPos.y - fHeight * 0.65f + offset.y
    };

    Vector2 center = {
        visualPos.x + (float)Constants::TILE_SIZE / 2.0f + offset.x,
        screenPos.y + fHeight * 0.5f
    };

    // 1. Hào quang Hoàng Tộc tỏa sáng rực rỡ xung quanh Boss
    if (enraged) {
        // Hào quang nộ bùng cháy: đỏ cam rực rỡ
        DrawCircleGradient(center, 90.0f, Color{ 255, 60, 30, 100 }, Color{ 255, 30, 0, 0 });
    } else {
        // Hào quang vàng hoàng gia thanh nhã
        DrawCircleGradient(center, 80.0f, Color{ 255, 215, 0, 80 }, Color{ 255, 180, 0, 0 });
    }

    // 2. Vẽ Sprite Ong Chúa
    Color tint = enraged ? Color{ 255, 230, 210, 255 } : WHITE;
    if (dying && currentAnim->getTotalFrames() > 1) {
        float progress = (float)currentAnim->getCurrentFrame() / (float)(currentAnim->getTotalFrames() - 1);
        if (progress > 1.0f) progress = 1.0f;
        tint.a = (unsigned char)(255.0f * (1.0f - progress));
    }
    currentAnim->draw(screenPos, s, tint);

    // 3. Vương miện ánh sáng hoàng tộc trên đầu Ong Chúa
    if (alive && !dying) {
        float time = (float)GetTime();
        float crownY = screenPos.y + 12.0f + std::sin(time * 3.5f) * 3.0f;
        float crownX = screenPos.x + fWidth / 2.0f;

        // Vẽ 3 đỉnh vương miện lấp lánh màu vàng kim
        Color crownColor = enraged ? Color{ 255, 80, 50, 240 } : Color{ 255, 235, 60, 240 };
        DrawCircle((int)crownX - 10, (int)crownY, 3.5f, crownColor);
        DrawCircle((int)crownX,      (int)crownY - 4.0f, 4.5f, crownColor);
        DrawCircle((int)crownX + 10, (int)crownY, 3.5f, crownColor);
        DrawCircle((int)crownX,      (int)crownY - 4.0f, 2.0f, WHITE);
    }
}

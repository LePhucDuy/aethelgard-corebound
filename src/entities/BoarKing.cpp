#include "entities/BoarKing.h"
#include "entities/Player.h"
#include "systems/CombatSystem.h"
#include "map/Dungeon.h"
#include "core/Constants.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <algorithm>

BoarKing::BoarKing(const Position& pos)
    : Monster("Boar King (Chua Heo Rung)", pos, 160, 20, 5, 200, 100,
              /*aggroRange*/ 15, /*patrolRange*/ 6, /*flying*/ false),
      bossState(BoarKingState::DORMANT),
      enraged(false),
      stateTimer(0.0f),
      windupTimer(0.0f),
      stunTimer(0.0f),
      stompCooldown(4.0f),
      chargeCooldown(5.0f),
      chargeDir(-1),
      chargeTargetX(146),
      requestedScreenShake(false),
      shakeIntensity(0.0f) {
    // Boss dùng bộ animation boar nhưng frame tốc độ cao, uy lực
    addAnimation("idle", std::make_unique<Animation>("boar_idle", 4, 48, 32, 0.12f, true));
    addAnimation("walk", std::make_unique<Animation>("boar_walk", 6, 48, 32, 0.09f, true));
    addAnimation("run",  std::make_unique<Animation>("boar_run",  6, 48, 32, 0.06f, true));
    addAnimation("dead", std::make_unique<Animation>("boar_hit",  4, 48, 32, 0.05f, false));
    setState("idle");
    setFacing(false); // Ban đầu quay mặt sang trái hướng về đấu trường
    setMoveLerpSpeed(6.0f);
}

void BoarKing::triggerEntrance() {
    if (bossState == BoarKingState::DORMANT) {
        bossState = BoarKingState::ENTRANCE_RUSH;
        setState("run");
        setFacing(false); // Quay sang trái lao vào đấu trường
        setMoveLerpSpeed(16.0f);
        // Đích đến giữa đấu trường (x = 146)
        pos = Position(146, 18);
        requestedScreenShake = true;
        shakeIntensity = 0.8f;
    }
}

void BoarKing::update(float deltaTime) {
    Entity::update(deltaTime);
    if (!currentAnim) return;

    if (stompCooldown > 0.0f) stompCooldown -= deltaTime;
    if (chargeCooldown > 0.0f) chargeCooldown -= deltaTime;

    // 1. Kiểm tra kết thúc Cinematic Entrance Rush
    if (bossState == BoarKingState::ENTRANCE_RUSH) {
        float targetX = 146.0f * (float)Constants::TILE_SIZE;
        if (std::abs(visualPos.x - targetX) < 4.0f) {
            visualPos.x = targetX;
            bossState = BoarKingState::PATROL;
            setState("idle");
            setMoveLerpSpeed(6.0f);
            actionTimer = 0.6f;
            requestedScreenShake = true;
            shakeIntensity = 1.0f;
        }
    }

    // 2. Trạng thái CHARGE_WINDUP: Lấy đà dậm móng cào đất
    if (bossState == BoarKingState::CHARGE_WINDUP) {
        windupTimer -= deltaTime;
        if (windupTimer <= 0.0f) {
            bossState = BoarKingState::CHARGING;
            setState("run");
            setMoveLerpSpeed(24.0f);
            actionTimer = 0.05f;
            requestedScreenShake = true;
            shakeIntensity = 0.6f;
        }
    }

    // 3. Trạng thái STUNNED: Bị choáng / kiệt sức
    if (bossState == BoarKingState::STUNNED) {
        stunTimer -= deltaTime;
        if (stunTimer <= 0.0f) {
            bossState = BoarKingState::CHASE;
            setState("idle");
            setMoveLerpSpeed(6.0f);
            actionTimer = 0.4f;
        }
    }
}

void BoarKing::act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    if (!alive || dying) return;
    if (bossState == BoarKingState::DORMANT || bossState == BoarKingState::ENTRANCE_RUSH) return;
    if (bossState == BoarKingState::STUNNED || bossState == BoarKingState::CHARGE_WINDUP) return;

    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;

    // Pha CUỒNG NỘ (HP <= 40%): Tăng sát thương, phòng thủ và hung bạo
    if (!enraged && hp <= maxHp * 4 / 10) {
        enraged = true;
        attack += 8;
        defense += 2;
        requestedScreenShake = true;
        shakeIntensity = 1.2f;
        combatLog.push_back(">>> [CUONG NO!] Boar King gao thet hung ton! Mat do ruc va sat thuong tang vot! <<<");
    }

    // =========================================================================
    // 1. ĐANG PHÓNG LÃO HÚC ĐIÊN CUỒNG (CHARGING)
    // =========================================================================
    if (bossState == BoarKingState::CHARGING) {
        int nextX = pos.x + chargeDir;

        // KIỂM TRA VA CHẠM VỚI NGƯỜI CHƠI
        if (pPos.x == nextX && pPos.y == pos.y) {
            // TÔNG TRÚNG!
            faceTowards(pPos);
            int chargeDmg = attack * 18 / 10;
            int actualDmg = std::max(1, chargeDmg - player.getDefense());
            player.takeDamage(chargeDmg);

            // Đẩy lùi (Knockback) người chơi 2 ô
            int knockX = std::clamp(pPos.x + chargeDir * 2, 131, 163);
            Position knockPos(knockX, pos.y);
            if (dungeon.isWalkable(knockPos)) {
                player.setPosition(knockPos);
            }

            requestedScreenShake = true;
            shakeIntensity = 1.2f;
            combatLog.push_back(TextFormat("[TONG TRUNG!] Boar King huc bay ban vang xa! (Gay %d sat thuong!)", actualDmg));

            pos.x = nextX;
            bossState = BoarKingState::CHASE;
            setState("idle");
            setMoveLerpSpeed(6.0f);
            actionTimer = 0.5f;
            return;
        }

        // KIỂM TRA CHẠM BIÊN HOẶC HẾT TẦM HÚC (Người chơi đã nhảy né qua đầu!)
        bool hitWall = (nextX <= 130 || nextX >= 164);
        bool reachedTarget = (nextX == chargeTargetX);

        if (hitWall || reachedTarget) {
            // HÚC TRƯỢT -> BỊ CHOÁNG 1.2 GIÂY!
            pos.x = std::clamp(nextX, 131, 163);
            bossState = BoarKingState::STUNNED;
            stunTimer = 1.2f;
            requestedScreenShake = true;
            shakeIntensity = 1.4f;
            setState("idle");
            setMoveLerpSpeed(6.0f);
            combatLog.push_back(">>> [HUC TRUOT!] Ban da nhay ne don thanh cong! Boar King mat da bi CHOANG 1.2s! PHAN CONG NGAY! <<<");
            actionTimer = 0.2f;
            return;
        }

        // Tiếp tục lao thẳng
        pos.x = nextX;
        actionTimer = 0.07f; // Lao cực nhanh
        return;
    }

    // =========================================================================
    // 2. KHOẢNG CÁCH CẬN CHIẾN (Kề sát 1 ô)
    // =========================================================================
    bool adjacent = (std::abs(dx) <= 1 && std::abs(dy) <= 1);
    if (adjacent && player.isAlive()) {
        faceTowards(pPos);

        // Kỹ năng: GIẬM ĐẤT SÓNG XUNG KÍCH (Earthquake Stomp)
        if (stompCooldown <= 0.0f) {
            setState("run");
            int stompDmg = attack * 12 / 10;
            int actualDmg = std::max(1, stompDmg - player.getDefense());
            player.takeDamage(stompDmg);

            // Hất lùi người chơi 2 ô ra xa
            int pushDir = (dx >= 0) ? 1 : -1;
            int pushX = std::clamp(pPos.x + pushDir * 2, 131, 163);
            Position pushPos(pushX, pos.y);
            if (dungeon.isWalkable(pushPos)) {
                player.setPosition(pushPos);
            }

            stompCooldown = enraged ? 3.5f : 5.5f;
            requestedScreenShake = true;
            shakeIntensity = 1.0f;
            combatLog.push_back(TextFormat("[GIAM DAT!] Boar King giam chan tao chan dong hat vang ban! (Gay %d sat thuong!)", actualDmg));
            actionTimer = 0.5f;
            return;
        }

        // Đòn đánh thường nanh heo
        if (attackCooldown <= 0.0f) {
            setState("run");
            CombatSystem::attack(*this, player, combatLog);
            attackCooldown = enraged ? 0.5f : 0.75f;
        }
        actionTimer = 0.35f;
        return;
    }

    // =========================================================================
    // 3. KHOẢNG CÁCH TẦM TRUNG / XA (3 - 9 ô trên sàn đất y = 18): KÍCH HOẠT LÃO HÚC
    // =========================================================================
    if (std::abs(dx) >= 3 && std::abs(dx) <= 9 && dy == 0 && chargeCooldown <= 0.0f) {
        bossState = BoarKingState::CHARGE_WINDUP;
        windupTimer = enraged ? 0.35f : 0.50f;
        chargeDir = (dx > 0) ? 1 : -1;
        chargeTargetX = (chargeDir > 0) ? std::min(163, pos.x + 8) : std::max(131, pos.x - 8);
        faceTowards(pPos);
        setState("idle");
        chargeCooldown = enraged ? 4.0f : 6.0f;
        combatLog.push_back("[LAY DA!] Boar King cao mong lay da chuan bi LAO HUC! Nhan [Space] nhay ne khi no phong toi!");
        actionTimer = 0.2f;
        return;
    }

    // =========================================================================
    // 4. ÁP SÁT TRUY ĐUỔI (CHASE)
    // =========================================================================
    setState("run");
    setMoveLerpSpeed(10.0f);
    faceTowards(pPos);
    int step = (dx > 0) ? 1 : -1;
    tryStepTo(dungeon, Position(pos.x + step, pos.y), player);
    actionTimer = 0.28f;
}

void BoarKing::onDeath(Player& player) {
    std::cout << "[BOSS HA GUC] Boar King da bi tieu diet!" << std::endl;
    player.addExp(expReward);
    player.addGold(goldReward);
}

void BoarKing::render(float scale, Vector2 offset) const {
    if (!currentAnim) return;

    // Tỉ lệ to lớn đồ sộ (scale * 2.2f: gấp hơn 2 lần quái thường)
    float s = scale * 2.2f;
    float fWidth = (float)currentAnim->getFrameWidth() * s;
    float fHeight = (float)currentAnim->getFrameHeight() * s;
    constexpr float FOOT_SINK = 6.0f;
    constexpr float TRIM_BOTTOM = 2.0f;

    Vector2 screenPos = {
        visualPos.x + ((float)Constants::TILE_SIZE - fWidth) / 2.0f + offset.x,
        visualPos.y + FOOT_SINK + TRIM_BOTTOM * s - fHeight + offset.y
    };

    // 1. Vẽ Vệt cảnh báo (Telegraph) khi Boss đang dậm chân lấy đà Lão Húc
    if (bossState == BoarKingState::CHARGE_WINDUP) {
        float startX = visualPos.x + (float)Constants::TILE_SIZE / 2.0f + offset.x;
        float endX = (float)(chargeTargetX * Constants::TILE_SIZE) + (float)Constants::TILE_SIZE / 2.0f + offset.x;
        float minX = std::min(startX, endX);
        float lineW = std::abs(endX - startX);
        float groundY = visualPos.y + (float)Constants::TILE_SIZE - 4.0f + offset.y;

        DrawRectangle((int)minX, (int)groundY, (int)lineW, 6, Color{ 255, 50, 50, 210 });
        DrawRectangleLines((int)minX, (int)groundY, (int)lineW, 6, Color{ 255, 230, 80, 240 });
    }

    // 2. Hào quang Boss rực rỡ (Aura)
    Vector2 center = {
        visualPos.x + (float)Constants::TILE_SIZE / 2.0f + offset.x,
        visualPos.y - fHeight * 0.4f + offset.y
    };

    if (enraged) {
        DrawCircleGradient(center, 75.0f, Color{ 255, 40, 20, 80 }, Color{ 255, 0, 0, 0 });
    } else {
        DrawCircleGradient(center, 65.0f, Color{ 255, 180, 40, 50 }, Color{ 255, 160, 0, 0 });
    }

    // 3. Vẽ Sprite Boss
    Color tint = WHITE;
    if (dying && currentAnim->getTotalFrames() > 1) {
        float progress = (float)currentAnim->getCurrentFrame() / (float)(currentAnim->getTotalFrames() - 1);
        if (progress > 1.0f) progress = 1.0f;
        tint.a = (unsigned char)(255.0f * (1.0f - progress));
    }
    currentAnim->draw(screenPos, s, tint);

    // 4. Hiệu ứng Choáng váng (Stunned)
    if (bossState == BoarKingState::STUNNED) {
        float time = (float)GetTime();
        float headY = screenPos.y + 10.0f;
        float headX = screenPos.x + fWidth / 2.0f;

        for (int i = 0; i < 3; ++i) {
            float angle = time * 6.0f + i * (2.0f * 3.14159f / 3.0f);
            float sx = headX + std::cos(angle) * 22.0f;
            float sy = headY + std::sin(angle) * 8.0f;
            DrawCircle((int)sx, (int)sy, 4.0f, GOLD);
            DrawCircle((int)sx, (int)sy, 2.0f, WHITE);
        }
    }
}
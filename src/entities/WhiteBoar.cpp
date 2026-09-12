#include "entities/WhiteBoar.h"
#include "entities/Player.h"
#include "systems/CombatSystem.h"
#include "map/Dungeon.h"
#include "core/Constants.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

WhiteBoar::WhiteBoar(const Position& pos)
    : Boar("White Boar (Bach Tru Tinh)", pos, /*hp*/ 80, /*attack*/ 16, /*defense*/ 4,
           /*expReward*/ 60, /*goldReward*/ 35, /*aggroRange*/ 8, /*patrolRange*/ 4) {
    // Hoạt họa đa trạng thái của Bạch Trư Tinh (White Boar)
    addAnimation("idle", std::make_unique<Animation>("boar_white_idle", 4, 48, 32, 0.14f, true));
    addAnimation("walk", std::make_unique<Animation>("boar_white_walk", 6, 48, 32, 0.11f, true));
    addAnimation("run",  std::make_unique<Animation>("boar_white_run",  6, 48, 32, 0.08f, true));
    addAnimation("hit",  std::make_unique<Animation>("boar_white_hit",  4, 48, 32, 0.07f, false));
    addAnimation("dead", std::make_unique<Animation>("boar_white_hit",  4, 48, 32, 0.06f, false));
    setState("idle");
    setMoveLerpSpeed(6.0f);
}

void WhiteBoar::act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    // Nếu đang phát hoạt ảnh bị đánh (hit) thì giữ nguyên, không đè hoạt ảnh di chuyển
    if (animState == "hit" && currentAnim && !currentAnim->hasFinished()) {
        return;
    }

    Position pPos = player.getPosition();
    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;
    bool adjacent = (std::abs(dx) <= 1 && std::abs(dy) <= 1);

    // 1. Cận chiến: Tấn công
    if (adjacent && player.isAlive()) {
        faceTowards(pPos);
        setState("run");
        if (attackCooldown <= 0.0f) {
            turnCount++;
            int baseAtk = getAttack();
            if (turnCount % 3 == 0) {
                // Đòn húc bạo kích hạng nặng của quái cấp trung
                combatLog.push_back("[BAO KICH!] Bach Tru Tinh lay da huc cuc manh vao ban!");
                attack = baseAtk * 14 / 10;
            }
            CombatSystem::attack(*this, player, combatLog);
            attack = baseAtk;
            attackCooldown = 0.75f;
        }
        actionTimer = 0.32f;
        return;
    }

    // 2. Tuần tra (Patrol)
    if (aiState == MonsterAIState::PATROL) {
        if (canSeePlayer(dungeon, player)) {
            aiState = MonsterAIState::CHASE;
            isAlerted = true;
            faceTowards(pPos);
            setState("run");
            combatLog.push_back("[CANH GIAC!] Bach Tru Tinh to lon phat hien va gao thet lao toi!");
            actionTimer = 0.15f;
            return;
        }

        setState("walk");
        setMoveLerpSpeed(5.5f);
        patrolStep(dungeon);
        actionTimer = 0.40f;
        return;
    }

    // 3. Truy đuổi (Chase)
    if (aiState == MonsterAIState::CHASE) {
        if (std::abs(dx) > aggroRange + 4 || std::abs(dy) > 3) {
            aiState = MonsterAIState::RETURNING;
            isAlerted = false;
            combatLog.push_back("Bach Tru Tinh mat dau ban va lui buoc quay ve.");
            actionTimer = 0.5f;
            return;
        }

        setState("run");
        setMoveLerpSpeed(16.0f);
        faceTowards(pPos);
        int step = (dx > 0) ? 1 : -1;
        tryStepTo(dungeon, Position(pos.x + step, pos.y), player);
        actionTimer = 0.18f;
        return;
    }

    // 4. Quay về (Returning)
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
            actionTimer = 0.5f;
            return;
        }

        setState("walk");
        setMoveLerpSpeed(5.5f);
        int step = (homePos.x > pos.x) ? 1 : -1;
        tryStepTo(dungeon, Position(pos.x + step, pos.y), player);
        actionTimer = 0.40f;
        return;
    }
}

void WhiteBoar::onDeath(Player& player) {
    std::cout << "[Ha guc] Bach Heo Tinh da bi tieu diet! Ban tang +" << expReward << " EXP va +" << goldReward << " Vang!" << std::endl;
    player.addExp(expReward);
    if (!goldDropped) {
        player.addGold(goldReward);
    }
}

void WhiteBoar::render(float scale, Vector2 offset) const {
    if (!currentAnim) return;

    // To hơn lợn rừng cơ bản ~30% (scale * 1.30f: khoảng 2.34f so với 1.8f của heo thường và ~4.0f của Boss)
    float s = scale * 1.30f;
    float fWidth = (float)currentAnim->getFrameWidth() * s;
    float fHeight = (float)currentAnim->getFrameHeight() * s;

    constexpr float FOOT_SINK = 6.0f;
    constexpr float TRIM_BOTTOM = 2.0f;
    Vector2 screenPos = {
        visualPos.x + ((float)Constants::TILE_SIZE - fWidth) / 2.0f + offset.x,
        visualPos.y - fHeight + (FOOT_SINK + TRIM_BOTTOM * s) + offset.y
    };

    Color tint = WHITE;
    if (dying && currentAnim->getTotalFrames() > 1) {
        float progress = (float)currentAnim->getCurrentFrame() / (float)(currentAnim->getTotalFrames() - 1);
        if (progress > 1.0f) progress = 1.0f;
        tint.a = (unsigned char)(255.0f * (1.0f - progress));
    }

    currentAnim->draw(screenPos, s, tint);
}


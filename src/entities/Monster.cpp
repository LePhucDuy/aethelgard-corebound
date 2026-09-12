#include "entities/Monster.h"
#include "entities/Player.h"
#include "map/Dungeon.h"
#include <cstdlib>

Monster::Monster(const std::string& name, const Position& pos, int hp, int attack, int defense,
                 int expReward, int goldReward,
                 int aggroRange, int patrolRange, bool flying)
    : Entity(name, pos, hp, attack, defense),
      expReward(expReward), goldReward(goldReward),
      homePos(pos), aggroRange(aggroRange), patrolRange(patrolRange),
      facingRight(true), flying(flying), turnCount(0), patrolDir(1),
      animState(""), dying(false), rewarded(false) {}

// ===== Hệ thống hoạt họa nhiều trạng thái =====

void Monster::addAnimation(const std::string& stateName, std::unique_ptr<Animation> anim) {
    anims[stateName] = std::move(anim);
    // Animation đầu tiên nạp vào trở thành mặc định
    if (!currentAnim) {
        setState(stateName);
    }
}

void Monster::setState(const std::string& stateName) {
    auto it = anims.find(stateName);
    if (it == anims.end()) return;           // Không có animation này -> bỏ qua
    if (animState == stateName) return;      // Đang dùng rồi -> không reset frame
    animState = stateName;
    currentAnim = it->second.get();
    currentAnim->reset();
}

void Monster::update(float deltaTime) {
    Entity::update(deltaTime);
    if (!currentAnim) return;

    // Tự động thoát khỏi đòn tấn công 1 lần (attack) khi animation chạy xong
    if (!dying && animState == "attack" && currentAnim->hasFinished()) {
        setState("idle");
    }
}

void Monster::render(float scale, Vector2 offset) const {
    if (!currentAnim) return;

    float fWidth = (float)currentAnim->getFrameWidth() * scale;
    float fHeight = (float)currentAnim->getFrameHeight() * scale;

    // pos = ô FLOOR (mặt cỏ) của quái bộ, ô EMPTY của ong bay.
    // Đáy FRAME neo tại mép TRÊN ô + lún 6px, cộng TRIM đáy NHỎ (~2px) cho phần
    // đệm trong suốt dưới chân ốc/heo. TRIM 10 cũ đẩy quái chìm ~14px*scale
    // xuống lòng đất (bug ảnh mới) nên giảm về 2px cho chân vừa chạm cỏ.
    // Ong bay giữ neo cũ (bay lơ lửng trên không là đúng).
    constexpr float FOOT_SINK = 6.0f;
    constexpr float TRIM_BOTTOM = 2.0f;
    Vector2 screenPos = {
        (float)(pos.x * Constants::TILE_SIZE) + ((float)Constants::TILE_SIZE - fWidth) / 2.0f + offset.x,
        (float)(pos.y * Constants::TILE_SIZE) - fHeight + (flying ? 0.0f : (FOOT_SINK + TRIM_BOTTOM * scale)) + offset.y
    };

    Color tint = WHITE;
    if (dying && currentAnim->getTotalFrames() > 1) {
        float progress = (float)currentAnim->getCurrentFrame() / (float)(currentAnim->getTotalFrames() - 1);
        if (progress > 1.0f) progress = 1.0f;
        tint.a = (unsigned char)(255.0f * (1.0f - progress));
    }

    currentAnim->draw(screenPos, scale, tint);
}

void Monster::takeDamage(int amount) {
    if (dying) return;
    Entity::takeDamage(amount);
    if (!alive) kill();
}

void Monster::kill() {
    alive = false;
    dying = true;
    // Phát animation biến mất: ưu tiên "dead", nếu không có thì giữ nguyên animation hiện tại
    if (anims.find("dead") != anims.end()) setState("dead");
}

bool Monster::isDeathAnimFinished() const {
    auto it = anims.find(animState);
    return it != anims.end() && it->second && it->second->hasFinished();
}

// ===== Hạ tầng AI dùng chung =====

void Monster::setFacing(bool right) {
    facingRight = right;
    if (currentAnim) currentAnim->setFacingRight(right);
}

void Monster::faceTowards(const Position& target) {
    if (target.x != pos.x) setFacing(target.x > pos.x);
}

bool Monster::isGrounded(Dungeon& dungeon, const Position& p) const {
    // Ô dưới chân phải là vật rắn (WALL/FLOOR gạch đỡ, STAIRS): tức là không thể đi xuyên
    // qua được. EMPTY là không khí -> không có sàn đỡ.
    if (!dungeon.isValidPos(Position(p.x, p.y + 1))) return true; // Đáy map: coi như có đất
    return !dungeon.isWalkable(Position(p.x, p.y + 1));
}

bool Monster::hasLineOfSight(Dungeon& dungeon, const Position& target) const {
    // Cùng hàng ngang: kiểm tra từng ô giữa 2 thực thể, không cho tường chắn ngang tầm nhìn
    if (target.y == pos.y) {
        int step = (target.x > pos.x) ? 1 : -1;
        for (int x = pos.x + step; x != target.x; x += step) {
            if (!dungeon.isWalkable(Position(x, pos.y))) return false;
        }
    }
    return true;
}

bool Monster::tryStepTo(Dungeon& dungeon, const Position& target, const Player& player) {
    if (!dungeon.isWalkable(target)) return false;                 // Ô không đi được
    if (target == player.getPosition()) return false;              // Không đè lên người chơi
    if (dungeon.getMonsterAt(target) != nullptr) return false;     // Không đè lên quái khác
    if (!flying && !isGrounded(dungeon, target)) return false;     // Quái bộ cần sàn đỡ dưới chân

    if (target.x != pos.x) setFacing(target.x > pos.x);
    setPosition(target);
    return true;
}

void Monster::patrolStep(Dungeon& dungeon) {
    Position next(pos.x + patrolDir, pos.y);

    bool outOfRange = std::abs(next.x - homePos.x) > patrolRange;
    bool blocked = !dungeon.isWalkable(next)
                   || dungeon.getMonsterAt(next) != nullptr
                   || (!flying && !isGrounded(dungeon, next));

    if (outOfRange || blocked) {
        // Đổi hướng tuần tra khi chạm biên vùng hoặc bị chặn
        patrolDir = -patrolDir;
        next = Position(pos.x + patrolDir, pos.y);
        bool canTurn = dungeon.isWalkable(next)
                       && dungeon.getMonsterAt(next) == nullptr
                       && (flying || isGrounded(dungeon, next));
        if (!canTurn) return; // Đứng yên lượt này
    }

    setFacing(patrolDir > 0);
    setPosition(next);
}

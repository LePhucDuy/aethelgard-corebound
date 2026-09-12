#include "entities/Monster.h"
#include "entities/Player.h"
#include "map/Dungeon.h"
#include <cstdlib>

// Khởi tạo các thành viên tĩnh (Static Members - Slide 36-39 Chương 3)
int Monster::activeMonsterCount = 0;
int Monster::totalMonstersDefeated = 0;

Monster::Monster(const std::string& name, const Position& pos, int hp, int attack, int defense,
                 int expReward, int goldReward,
                 int aggroRange, int patrolRange, bool flying)
    : Entity(name, pos, hp, attack, defense),
      expReward(expReward), goldReward(goldReward),
      homePos(pos), aggroRange(aggroRange), patrolRange(patrolRange),
      facingRight(true), flying(flying), turnCount(0), patrolDir(1),
      aiState(MonsterAIState::PATROL), actionTimer(0.0f), attackCooldown(0.0f),
      pauseTimer(0.0f), isAlerted(false),
      animState(""), dying(false), rewarded(false) {
    ++activeMonsterCount;
}

Monster::~Monster() {
    if (activeMonsterCount > 0) {
        --activeMonsterCount;
    }
}

// ===== Hệ thống hoạt họa nhiều trạng thái =====

void Monster::addAnimation(const std::string& stateName, std::unique_ptr<Animation> anim) {
    if (anim) {
        // Spritesheet của quái vật (Heo, Ốc, Ong) trong assets có hướng vẽ gốc quay về bên TRÁI.
        // Đặt baseFacingRight = false để khi quái quay phải (facingRight = true),
        // sprite được lật ngang (flip) sang phải chuẩn xác theo đúng hướng di chuyển.
        anim->setBaseFacingRight(false);
        anim->setFacingRight(facingRight);
    }
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
    currentAnim->setFacingRight(facingRight);
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
        visualPos.x + ((float)Constants::TILE_SIZE - fWidth) / 2.0f + offset.x,
        visualPos.y - fHeight + (flying ? 0.0f : (FOOT_SINK + TRIM_BOTTOM * scale)) + offset.y
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
    if (!alive) {
        kill();
    } else {
        // Bị đánh trúng -> lập tức báo động và chuyển sang truy đuổi
        isAlerted = true;
        aiState = MonsterAIState::CHASE;
        actionTimer = 0.1f;
    }
}

void Monster::kill() {
    if (!dying) {
        ++totalMonstersDefeated;
    }
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
    for (auto& pair : anims) {
        if (pair.second) pair.second->setFacingRight(right);
    }
    if (currentAnim) currentAnim->setFacingRight(right);
}

void Monster::faceTowards(const Position& target) {
    if (target.x != pos.x) setFacing(target.x > pos.x);
}

bool Monster::isGrounded(Dungeon& dungeon, const Position& p) const {
    // Ô dưới chân phải là vật rắn đỡ (WALL, FLOOR, STAIRS_DOWN).
    // Tuyệt đối không đứng trên không khí (EMPTY) hoặc nước ngập (WATER)!
    Position below(p.x, p.y + 1);
    if (!dungeon.isValidPos(below)) return false;
    TileType t = dungeon.getTileType(below);
    return (t == TileType::WALL || t == TileType::FLOOR || t == TileType::STAIRS_DOWN);
}

bool Monster::hasLineOfSight(Dungeon& dungeon, const Position& target) const {
    int dx = target.x - pos.x;
    int dy = target.y - pos.y;
    int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) return true;

    float xStep = (float)dx / (float)steps;
    float yStep = (float)dy / (float)steps;

    for (int i = 1; i < steps; ++i) {
        int checkX = (int)std::round(pos.x + i * xStep);
        int checkY = (int)std::round(pos.y + i * yStep);
        if (!dungeon.isWalkable(Position(checkX, checkY))) return false;
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
        setFacing(patrolDir > 0);
        pauseTimer = 0.5f; // Dừng lại 0.5s quan sát tự nhiên trước khi quay đầu
        setState("idle");  // Đứng yên trong lúc dừng
        return;
    }

    setFacing(patrolDir > 0);
    setPosition(next);
}

bool Monster::canSeePlayer(Dungeon& dungeon, const Player& player) const {
    if (!player.isAlive()) return false;
    Position pPos = player.getPosition();

    int dx = pPos.x - pos.x;
    int dy = pPos.y - pos.y;

    // 1. Khoảng cách ngang trong tầm phát hiện
    if (std::abs(dx) > aggroRange) return false;

    // 2. Độ cao / cùng tầng
    if (!flying && std::abs(dy) > 1) return false;
    if (flying && std::abs(dy) > aggroRange) return false;

    // 3. HƯỚNG NHÌN (QUAY LƯNG THÌ KHÔNG PHÁT HIỆN):
    // Quái quay phải (facingRight == true) -> chỉ nhìn thấy dx > 0
    // Quái quay trái (facingRight == false) -> chỉ nhìn thấy dx < 0
    if (facingRight && dx < 0) return false;
    if (!facingRight && dx > 0) return false;
    if (dx == 0 && !flying) return false;

    // 4. Đường nhìn thẳng không bị tường chắn
    if (!hasLineOfSight(dungeon, pPos)) return false;

    return true;
}

void Monster::updateAI(float deltaTime, Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    if (!alive || dying) return;

    if (actionTimer > 0.0f) actionTimer -= deltaTime;
    if (attackCooldown > 0.0f) attackCooldown -= deltaTime;
    if (pauseTimer > 0.0f) {
        pauseTimer -= deltaTime;
        return; // Đang dừng quan sát trước khi quay đầu
    }

    if (actionTimer <= 0.0f) {
        act(dungeon, player, combatLog);
    }
}

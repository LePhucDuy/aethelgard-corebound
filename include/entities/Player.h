#ifndef PLAYER_H
#define PLAYER_H

#include "entities/Entity.h"
#include "items/Inventory.h"
#include "systems/Skill.h"
#include <map>

/**
 * @brief Lớp Player kế thừa từ Entity (Inheritance: IS-A)
 * Đại diện cho nhân vật hiệp sĩ (Warrior/Knight) mà người chơi điều khiển.
 * 
 * ĐẶC ĐIỂM OOP:
 * 1. Kế thừa (Inheritance):
 *    - `Player : public Entity`: Kế thừa toàn bộ thuộc tính HP, ATK, DEF, Position.
 * 2. Hợp thành (Composition):
 *    - `std::unique_ptr<Inventory> inventory;`
 *    - Quản lý toàn bộ hệ thống hoạt họa phong phú: "idle", "run", "attack", "jump", "dead".
 * 3. Đa hình (Polymorphism):
 *    - Override phương thức ảo thuần túy `act(Dungeon& dungeon)`.
 */
class Player : public Entity {
private:
    // Khai báo Lớp bạn (Friend Class - Slide 60-61 Chương 3)
    friend class SaveLoadManager;
    friend class CombatSystem;

    int level;
    int exp;
    int expToNextLevel;
    int gold;
    int forgeLevel; // Cấp độ đe rèn cường hóa vũ khí (Blacksmith Forge)

    // Quan hệ hợp thành (Composition): Player sở hữu độc quyền Inventory
    std::unique_ptr<Inventory> inventory;

    // Quản lý các trạng thái hoạt họa (Idle, Run, Attack, Jump, Dead)
    std::map<std::string, std::unique_ptr<Animation>> anims;
    std::string currentState;
    float runTimer;
    float jumpTimer;       // Thời gian còn khóa hoạt họa nhảy (ưu tiên hơn run)
    float jumpVisualLift;  // Độ nâng hình ảnh (px world) cho cú nhảy xa
    float sinkVisualOffset;// Độ lún vào bùn đầm lầy (px)
    float fallTimer;       // Thời gian đếm ngược của cú rơi xuống hố
    float fallAirDuration; // Thời gian rơi tự do trong không trung (chuẩn gia tốc g)
    float fallTotalDuration; // Tổng thời gian rơi + tiếp đất giảm chấn
    float startFallY;      // Tọa độ Y bắt đầu rơi
    float targetFallY;     // Tọa độ Y đáy sàn tiếp đất
    bool facingRight;

    // Quản lý kỹ năng đa hình (Polymorphic Skills)
    std::vector<std::unique_ptr<Skill>> skills;

    // Cơ chế Đỡ đòn & Phản đòn hoàn hảo (Block & Perfect Parry) [K]
    float blockTimer;
    float parryWindowTimer;
    float blockCooldown;

    // Cơ chế Trúng Độc theo thời gian (Poison DoT System)
    float poisonTimer;
    float poisonTickTimer;
    int poisonDmgPerTick;

    void checkLevelUp();

public:
    Player(const std::string& name, const Position& pos, int hp = 100, int attack = 16, int defense = 5);
    ~Player() override = default;

    // Cài đặt phương thức thuần ảo từ lớp cha Entity (Polymorphism)
    void act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) override;

    // Cập nhật hoạt họa và render
    void update(float deltaTime) override;
    void render(float scale = 2.0f, Vector2 offset = {0.0f, 0.0f}) const override;

    // Quản lý kỹ năng đa hình
    bool useSkill(size_t index, GameEngine* engine);
    Skill* getSkill(size_t index) const;
    size_t getSkillCount() const { return skills.size(); }

    // Di chuyển và các hành động hoạt họa
    void setPosition(const Position& newPos, bool snapVisual = false) override;
    bool moveBy(int dx, int dy, Dungeon& dungeon);
    void triggerAttack();
    void triggerJump(int dx = 0, int dy = -1);
    void triggerFall(float airDuration = 0.52f, float landDuration = 0.22f);
    void setFacingRight(bool right);

    bool isFacingRight() const { return facingRight; }
    bool isAttacking() const { return currentState == "attack"; }
    bool isJumping() const { return currentState == "jump"; }
    bool isFalling() const { return fallTimer > 0.0f; }

    // Cơ chế Đỡ đòn & Phản đòn hoàn hảo [K]
    void triggerBlock();
    bool isBlocking() const { return blockTimer > 0.0f; }
    bool isParrying() const { return parryWindowTimer > 0.0f; }
    float getBlockTimer() const { return blockTimer; }
    float getBlockCooldown() const { return blockCooldown; }

    // Cơ chế Trúng Độc theo thời gian (Poison DoT)
    void applyPoison(float duration, int dmgPerTick);
    void curePoison();
    bool isPoisoned() const { return poisonTimer > 0.0f; }
    float getPoisonTimer() const { return poisonTimer; }
    int getPoisonDmgPerTick() const { return poisonDmgPerTick; }

    // Xử lý kinh nghiệm, cấp độ và tiền vàng
    void addExp(int amount);
    void addGold(int amount);
    bool spendGold(int amount);
    void addAttack(int amount) { attack += amount; }
    void addDefense(int amount) { defense += amount; }

    // Cơ chế tiêu thụ vàng: Đe rèn cường hóa vũ khí (Blacksmith Forge)
    int getForgeLevel() const { return forgeLevel; }
    int getNextUpgradeCost() const { return 30 + forgeLevel * 35; }
    int getNextUpgradeBonus() const { return 3 + forgeLevel * 2; }
    bool upgradeForge();

    // Quản lý túi đồ
    Inventory& getInventory() { return *inventory; }
    const Inventory& getInventory() const { return *inventory; }

    // Quản lý animation
    void addAnimation(const std::string& stateName, std::unique_ptr<Animation> anim);
    void setState(const std::string& stateName);
    const std::string& getState() const { return currentState; }
    const Animation* getCurrentAnimation() const;

    // Getters
    int getLevel() const { return level; }
    int getExp() const { return exp; }
    int getExpToNextLevel() const { return expToNextLevel; }
    int getGold() const { return gold; }

    // Method Chaining với con trỏ this (Slide 24-25 Chương 3)
    Player& setLevel(int val) { level = val; return *this; }
    Player& setExp(int val) { exp = val; return *this; }
    Player& setExpToNextLevel(int val) { expToNextLevel = val; return *this; }
    Player& setGold(int val) { gold = val; return *this; }
    Player& setForgeLevel(int val) { forgeLevel = val; return *this; }
    Player& setHp(int val) { hp = val; return *this; }
    Player& setMaxHp(int val) { maxHp = val; return *this; }
    Player& setAttack(int val) { attack = val; return *this; }
    Player& setDefense(int val) { defense = val; return *this; }

    // Đặt lại chỉ số ban đầu khi chơi lại
    void resetStats(const Position& startPos);
    void resetJumpVisual() { jumpTimer = 0.0f; jumpVisualLift = 0.0f; }
    void setSinkVisualOffset(float offset) { sinkVisualOffset = offset; }
    float getSinkVisualOffset() const { return sinkVisualOffset; }
};

#endif // PLAYER_H

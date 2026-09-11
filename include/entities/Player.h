#ifndef PLAYER_H
#define PLAYER_H

#include "entities/Entity.h"
#include "items/Inventory.h"
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
    int level;
    int exp;
    int expToNextLevel;
    int gold;

    // Quan hệ hợp thành (Composition): Player sở hữu độc quyền Inventory
    std::unique_ptr<Inventory> inventory;

    // Quản lý các trạng thái hoạt họa (Idle, Run, Attack, Jump, Dead)
    std::map<std::string, std::unique_ptr<Animation>> anims;
    std::string currentState;
    float runTimer;
    bool facingRight;

    void checkLevelUp();

public:
    Player(const std::string& name, const Position& pos, int hp = 100, int attack = 16, int defense = 5);
    ~Player() override = default;

    // Cài đặt phương thức thuần ảo từ lớp cha Entity (Polymorphism)
    void act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) override;

    // Cập nhật hoạt họa và render
    void update(float deltaTime) override;
    void render(float scale = 2.0f, Vector2 offset = {0.0f, 0.0f}) const override;

    // Di chuyển và các hành động hoạt họa
    bool moveBy(int dx, int dy, Dungeon& dungeon);
    void triggerAttack();
    void triggerJump(int dx = 0, int dy = -1);

    bool isFacingRight() const { return facingRight; }
    bool isAttacking() const { return currentState == "attack"; }
    bool isJumping() const { return currentState == "jump"; }

    // Xử lý kinh nghiệm, cấp độ và tiền vàng
    void addExp(int amount);
    void addGold(int amount);
    void addAttack(int amount) { attack += amount; }
    void addDefense(int amount) { defense += amount; }

    // Quản lý túi đồ
    Inventory& getInventory() { return *inventory; }
    const Inventory& getInventory() const { return *inventory; }

    // Quản lý animation
    void addAnimation(const std::string& stateName, std::unique_ptr<Animation> anim);
    void setState(const std::string& stateName);
    const std::string& getState() const { return currentState; }

    // Getters & Setters
    int getLevel() const { return level; }
    int getExp() const { return exp; }
    int getExpToNextLevel() const { return expToNextLevel; }
    int getGold() const { return gold; }

    void setLevel(int val) { level = val; }
    void setExp(int val) { exp = val; }
    void setExpToNextLevel(int val) { expToNextLevel = val; }
    void setGold(int val) { gold = val; }
    void setHp(int val) { hp = val; }
    void setMaxHp(int val) { maxHp = val; }
    void setAttack(int val) { attack = val; }
    void setDefense(int val) { defense = val; }

    // Đặt lại chỉ số ban đầu khi chơi lại
    void resetStats(const Position& startPos);
};

#endif // PLAYER_H

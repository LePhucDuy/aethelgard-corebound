#ifndef SKILL_H
#define SKILL_H

#include <string>
#include <iostream>

// Forward declarations
class Player;
class GameEngine;

/**
 * @brief Lớp trừu tượng Skill (Abstract Base Class - Chương 6 Đa hình)
 * Đại diện cho các kỹ năng/chiêu thức của nhân vật.
 * 
 * TIÊU CHÍ OOP CHƯƠNG 6 (Đa hình):
 * 1. Phương thức thuần ảo: `virtual bool execute(Player* user, GameEngine* engine) = 0;`
 * 2. Hàm hủy ảo: `virtual ~Skill() = default;`
 * 3. Liên kết động (Late Binding) lúc runtime: khi người chơi nhấn phím [L-Shift] hoặc [Q],
 *    chương trình gọi hàm qua con trỏ lớp cơ sở `Skill*`.
 */
class Skill {
protected:
    std::string name;
    std::string description;
    float cooldown;
    float currentCooldown;

public:
    Skill(const std::string& name, const std::string& description, float cooldown)
        : name(name), description(description), cooldown(cooldown), currentCooldown(0.0f) {}

    virtual ~Skill() = default;

    // Phương thức thuần ảo thực thi kỹ năng (Late Binding)
    virtual bool execute(Player* user, GameEngine* engine) = 0;

    virtual void update(float dt) {
        if (currentCooldown > 0.0f) {
            currentCooldown -= dt;
            if (currentCooldown < 0.0f) currentCooldown = 0.0f;
        }
    }

    bool canExecute() const {
        return currentCooldown <= 0.0f;
    }

    void triggerCooldown() {
        currentCooldown = cooldown;
    }

    const std::string& getName() const { return name; }
    const std::string& getDescription() const { return description; }
    float getCooldown() const { return cooldown; }
    float getCurrentCooldown() const { return currentCooldown; }
};

/**
 * @brief Kỹ năng Chém Kiếm cơ bản (SlashSkill)
 */
class SlashSkill : public Skill {
public:
    SlashSkill();
    bool execute(Player* user, GameEngine* engine) override;
};

/**
 * @brief Kỹ năng Lướt Né Đòn (DashSkill - phím L-Shift)
 * Lướt nhanh 3 ô về phía trước, có thời gian bất tử (invulnerability frames).
 */
class DashSkill : public Skill {
public:
    DashSkill();
    bool execute(Player* user, GameEngine* engine) override;
};

/**
 * @brief Kỹ năng Hồi Máu Khẩn Cấp (HealSkill - phím Q)
 * Hồi phục tức thì 30 HP, thời gian hồi chiêu 8 giây.
 */
class HealSkill : public Skill {
public:
    HealSkill();
    bool execute(Player* user, GameEngine* engine) override;
};

#endif // SKILL_H


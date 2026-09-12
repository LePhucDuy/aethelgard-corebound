#ifndef CHEST_H
#define CHEST_H

#include "core/IRenderable.h"
#include "core/Position.h"
#include <string>

class Player;
class GameEngine;

/**
 * @brief Lớp Chest đại diện cho Rương Kho Báu Hoàng Kim trong Hầm Ngục.
 * 
 * TIÊU CHÍ OOP CHƯƠNG 5 & 6:
 * - Kế thừa giao diện (Interface Inheritance): Kế thừa từ `IRenderable` (và gián tiếp kế thừa ảo từ `IGameObject`).
 * - Đóng gói (Encapsulation): Che giấu pos, opened, unlockCost, rewardName ở phạm vi private.
 * - Đa hình (Polymorphism): Ghi đè phương thức thuần ảo `render(...)` từ `IRenderable`.
 */
class Chest : public IRenderable {
private:
    Position pos;
    bool opened;
    int unlockCost;
    std::string rewardName;
    int expReward;

public:
    Chest(const Position& pos, int cost = 35, const std::string& reward = "Than Duoc Aethelgard", int exp = 50);
    ~Chest() override = default;

    // Ghi đè phương thức thuần ảo từ giao diện IRenderable (Chương 5 & 6)
    void render(float scale = 2.0f, Vector2 offset = {0.0f, 0.0f}) const override;

    bool isOpened() const { return opened; }
    const Position& getPosition() const { return pos; }
    int getUnlockCost() const { return unlockCost; }
    const std::string& getRewardName() const { return rewardName; }

    bool isNear(const Position& playerPos) const {
        return pos.manhattanDistanceTo(playerPos) <= 2;
    }

    bool tryOpen(Player& player, GameEngine* engine);
};

#endif // CHEST_H


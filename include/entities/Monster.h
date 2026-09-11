#ifndef MONSTER_H
#define MONSTER_H

#include <vector>
#include "entities/Entity.h"

// Forward declaration
class Player;
class Dungeon;

/**
 * @brief Lớp trừu tượng Monster (Abstract Base Class 2)
 * Kế thừa từ Entity (Inheritance: IS-A), làm lớp cha cho toàn bộ quái vật trong game.
 *
 * AI THEO LƯỢT (Turn-based AI):
 * - Mỗi lượt của quái vật, Engine gọi `act(dungeon, player, combatLog)` — lớp con
 *   tự quyết định: tuần tra (patrol), đuổi theo (chase) hay tấn công (melee).
 * - Hạ tầng AI dùng chung được đặt tại lớp cha: bước đi an toàn (tryStepTo),
 *   kiểm tra sàn đỡ dưới chân (isGrounded — quái bộ không đứng lơ lửng),
 *   tầm nhìn thẳng (hasLineOfSight — không đánh/nhìn xuyên tường).
 */
class Monster : public Entity {
protected:
    int expReward;
    int goldReward;

    // Trạng thái AI
    Position homePos;   // Điểm sinh — tâm của vùng tuần tra
    int aggroRange;     // Bán kính phát hiện người chơi (ô)
    int patrolRange;    // Bán kính đi tuỳ ý quanh homePos (ô)
    bool facingRight;   // Hướng quay mặt (lật sprite)
    bool flying;        // Bay: chỉ đứng trên ô không khí (EMPTY), không cần sàn đỡ
    int turnCount;      // Số lượt đã qua (dùng cho kỹ năng theo chu kỳ)
    int patrolDir;      // Hướng tuần tra hiện tại (+1 / -1)

    // ===== Helper AI dùng chung cho mọi loài =====
    void setFacing(bool right);
    void faceTowards(const Position& target);

    // Bước tới ô đích nếu an toàn: walkable, không đè quái khác, không đè player,
    // và (với quái bộ) ô đích phải có sàn đỡ phía dưới
    bool tryStepTo(Dungeon& dungeon, const Position& target, const Player& player);

    // Ô dưới chân pos có là vật chắn (WALL/không walkable) không
    bool isGrounded(Dungeon& dungeon, const Position& pos) const;

    // Có nhìn thấy target không (cùng hàng ngang: không bị tường chắn giữa 2 ô)
    bool hasLineOfSight(Dungeon& dungeon, const Position& target) const;

    // Tuần tra: đi qua lại quanh homePos, đổi hướng khi chạm biên hoặc bị chắn
    void patrolStep(Dungeon& dungeon);

public:
    Monster(const std::string& name, const Position& pos, int hp, int attack, int defense,
            int expReward, int goldReward,
            int aggroRange = 5, int patrolRange = 3, bool flying = false);
    virtual ~Monster() override = default;

    // AI theo lượt — mỗi loài quái tự triển khai hành vi riêng (Polymorphism)
    void act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) override = 0;
    virtual void onDeath(Player& player) = 0;

    int getExpReward() const { return expReward; }
    int getGoldReward() const { return goldReward; }
    const Position& getHomePos() const { return homePos; }
    bool isFacingRight() const { return facingRight; }
    bool isFlying() const { return flying; }
};

#endif // MONSTER_H

#ifndef AI_STRATEGY_H
#define AI_STRATEGY_H

#include <string>
#include <vector>
#include <memory>
#include "core/Position.h"

// Forward declarations
class Monster;
class Dungeon;
class Player;

/**
 * @brief Giao diện trừu tượng Chiến Lược AI (Strategy Pattern - GoF Behavioral Pattern)
 * 
 * ĐẶC ĐIỂM OOP:
 * 1. Đa hình động (Runtime Polymorphism):
 *    - `IAIStrategy` là Pure Abstract Base Class với phương thức thuần ảo `execute(...)`.
 * 2. Ưu tiên Hợp thành hơn Kế thừa (Composition over Inheritance):
 *    - Quái vật sở hữu một con trỏ `std::unique_ptr<IAIStrategy> aiStrategy`.
 *    - Có thể thay đổi chiến lược hành vi linh hoạt ngay trong thời gian chạy (Runtime Strategy Swapping).
 * 3. Nguyên lý Đóng/Mở (Open/Closed Principle):
 *    - Khi bổ sung loài quái mới với AI mới (ví dụ AI Đánh Lén, AI Bắn Cung), chỉ cần tạo
 *      lớp chiến lược mới mà không cần chỉnh sửa lại mã nguồn của lớp `Monster`.
 */
class IAIStrategy {
public:
    virtual ~IAIStrategy() = default;

    // Thực thi thuật toán AI của chiến lược này
    virtual void execute(Monster& monster, Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) = 0;

    // Tên định danh của chiến lược
    virtual std::string getStrategyName() const = 0;
};

/**
 * @brief Chiến lược tuần tra mặt đất truyền thống (Ground Patrol & Melee Strike)
 * Dành cho quái bộ: Snail, Boar, WhiteBoar
 */
class GroundPatrolStrategy : public IAIStrategy {
public:
    GroundPatrolStrategy() = default;
    ~GroundPatrolStrategy() override = default;

    void execute(Monster& monster, Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) override;
    std::string getStrategyName() const override { return "GroundPatrolStrategy"; }
};

/**
 * @brief Chiến lược bay lượn bầy đàn và bổ nhào chọc nọc độc (Aerial Swarm & Dive Strike)
 * Dành cho quái bay: SmallBee, QueenBee
 */
class SwarmAerialStrategy : public IAIStrategy {
public:
    SwarmAerialStrategy() = default;
    ~SwarmAerialStrategy() override = default;

    void execute(Monster& monster, Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) override;
    std::string getStrategyName() const override { return "SwarmAerialStrategy"; }
};

/**
 * @brief Chiến lược Cuồng Nộ Bão Tố (Boss Berserk & Relentless Rush)
 * Kích hoạt khi đại boss (QueenBee, BoarKing) rơi vào trạng thái cuồng nộ
 */
class BossRageStrategy : public IAIStrategy {
private:
    float rushTimer;

public:
    BossRageStrategy() : rushTimer(0.0f) {}
    ~BossRageStrategy() override = default;

    void execute(Monster& monster, Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) override;
    std::string getStrategyName() const override { return "BossRageStrategy"; }
};

#endif // AI_STRATEGY_H


#ifndef QUEEN_BEE_H
#define QUEEN_BEE_H

#include "entities/FlyingMonster.h"
#include <vector>

// Forward declaration
class Player;

/**
 * @brief Lớp QueenBee — ĐẠI BOSS ONG CHÚA canh giữ Vực Đá & Thác Nước (Khu C).
 * 
 * Kiến trúc OOP C++17 (Chương 5 & Chương 6 UTH):
 * - Kế thừa phân cấp từ FlyingMonster -> Monster -> Entity -> IGameObject.
 * - Runtime Polymorphism: Override các phương thức thuần ảo act, takeDamage, onDeath, render, update.
 * - Encapsulation: Bảo vệ các chỉ số đặc quyền của Boss và trạng thái chiến đấu nộ (Enrage).
 */
class QueenBee : public FlyingMonster {
private:
    bool enraged;               // Trạng thái nộ hoàng gia (HP < 50%)
    bool swarmSummoned;         // Đã triệu hồi đàn ong hộ vệ hay chưa
    float stingCooldown;        // Cooldown chiêu chọc nọc độc hoàng kim
    float rushCooldown;         // Cooldown chiêu phi thân lao nọc
    float specialTimer;         // Bộ đếm hoạt ảnh chiêu thức
    int evasionChance;          // Tỉ lệ né đòn hoàng gia (25%)
    bool isRushing;             // Đang trong trạng thái lao nọc thần tốc
    int rushDirX;               // Hướng lao ngang
    int rushTargetX;            // Vị trí đích đến của cú lao
    bool callSwarmRequested;    // Cờ báo hiệu bị người chơi tấn công -> lập tức triệu hồi đàn ong
    float summonCooldown;       // Thời gian hồi chiêu triệu hồi đàn ong cứu viện

public:
    explicit QueenBee(const Position& pos);
    ~QueenBee() override = default;

    // Polymorphic overrides (Chương 6)
    void update(float deltaTime) override;
    void act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) override;
    void takeDamage(int amount) override;
    void onDeath(Player& player) override;
    void render(float scale = 2.0f, Vector2 offset = {0.0f, 0.0f}) const override;

    // Kỹ năng Boss đặc biệt
    bool isEnraged() const { return enraged; }
    bool isSwarmSummoned() const { return swarmSummoned; }
    void triggerEnrage();
};

#endif // QUEEN_BEE_H


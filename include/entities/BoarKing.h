#ifndef BOAR_KING_H
#define BOAR_KING_H

#include "entities/Boar.h"
#include <vector>

// Forward declaration
class Player;

enum class BoarKingState {
    DORMANT,         // Chờ ở rìa ngoài bên phải trước khi kích hoạt
    ENTRANCE_RUSH,   // Phi nước đại từ phải sang trái vào giữa đấu trường
    PATROL,          // Tuần tra bình thường
    CHASE,           // Áp sát người chơi
    CHARGE_WINDUP,   // Dậm móng lấy đà húc (telegraph)
    CHARGING,        // Đang phóng như tên bắn theo đường thẳng
    STUNNED,         // Bị choáng / kiệt sức 1s do húc hụt khi người chơi nhảy né
    STOMP            // Giậm mạnh 2 chân trước gây sóng xung kích
};

/**
 * @brief Lớp BoarKing — BOSS TỐI THƯỢNG canh giữ Đấu Trường Khu F.
 * Kế thừa đa mức (Multi-level Inheritance: IGameObject -> Entity -> Monster -> GroundMonster -> Boar -> BoarKing).
 */
class BoarKing : public Boar {
private:
    BoarKingState bossState;
    bool enraged;
    float stateTimer;
    float windupTimer;
    float stunTimer;
    float stompCooldown;
    float chargeCooldown;
    int chargeDir;          // Hướng húc: +1 (phải), -1 (trái)
    int chargeTargetX;      // Tọa độ X đích đến của đợt húc
    bool requestedScreenShake;
    float shakeIntensity;

public:
    explicit BoarKing(const Position& pos);
    ~BoarKing() override = default;

    void update(float deltaTime) override;
    void act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) override;
    void onDeath(Player& player) override;
    void render(float scale = 2.0f, Vector2 offset = {0.0f, 0.0f}) const override;

    // Cinematic Entrance & Kỹ năng
    void triggerEntrance();
    bool isEntranceDone() const { return bossState != BoarKingState::DORMANT && bossState != BoarKingState::ENTRANCE_RUSH; }
    bool isDormant() const { return bossState == BoarKingState::DORMANT; }
    bool isCharging() const { return bossState == BoarKingState::CHARGING; }
    bool isStunned() const { return bossState == BoarKingState::STUNNED; }
    BoarKingState getBossState() const { return bossState; }

    bool consumeScreenShake(float& outIntensity) {
        if (requestedScreenShake) {
            requestedScreenShake = false;
            outIntensity = shakeIntensity;
            return true;
        }
        return false;
    }
};

#endif // BOAR_KING_H
#ifndef SMALL_BEE_H
#define SMALL_BEE_H

#include "entities/FlyingMonster.h"

/**
 * @brief Lớp SmallBee (Ong sát thủ) kế thừa từ FlyingMonster (Kế thừa phân cấp)
 * Quái cơ động bay: tốc độ cao, né đòn tốt, gây sát thương độc chích.
 */
class SmallBee : public FlyingMonster {
private:
    int evasionChance; // Tỉ lệ phần trăm né đòn (%)
    bool isAggro;      // Trạng thái thù địch: mặc định false (hòa bình), chỉ cắn khi bị đánh trúng

public:
    explicit SmallBee(const Position& pos);
    ~SmallBee() override = default;

    // Getter/Setter trạng thái thù địch
    bool isAggressive() const { return isAggro; }
    void setAggro(bool aggro) { isAggro = aggro; }

    // Override phương thức thuần ảo từ Monster & Entity (Polymorphism)
    void act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) override;
    void onDeath(Player& player) override;

    // Override takeDamage để thể hiện khả năng né đòn và kích hoạt phản kích khi trúng đòn
    void takeDamage(int amount) override;
};

#endif // SMALL_BEE_H

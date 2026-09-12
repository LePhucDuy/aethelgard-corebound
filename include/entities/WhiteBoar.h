#ifndef WHITE_BOAR_H
#define WHITE_BOAR_H

#include "entities/Boar.h"

/**
 * @brief Lớp WhiteBoar (Bạch Trư Tinh / Lợn Rừng Cấp Trung) kế thừa từ Boar.
 * Minh chứng Kế thừa phân cấp (Hierarchical Inheritance: Boar -> WhiteBoar, Boar -> BoarKing).
 * 
 * Đặc điểm quái cấp trung (Mid-tier / Elite Monster):
 * - To hơn lợn rừng cơ bản ~30% (scale 1.30f).
 * - Bộ lông màu trắng bạc, bờm gai tím sẫm, mắt đỏ dữ dằn.
 * - Chỉ số chiến đấu vượt trội: HP 80, ATK 16, DEF 4, tầm tuần tra & phát hiện xa hơn.
 * - Đòn húc bạo kích hạng nặng (Heavy Rush) gây sát thương cực lớn mỗi chu kỳ tấn công.
 */
class WhiteBoar : public Boar {
public:
    explicit WhiteBoar(const Position& pos);
    ~WhiteBoar() override = default;

    // Ghi đè hành vi đa hình (Polymorphism)
    void act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) override;
    void onDeath(Player& player) override;
    void render(float scale = 2.0f, Vector2 offset = {0.0f, 0.0f}) const override;
};

#endif // WHITE_BOAR_H


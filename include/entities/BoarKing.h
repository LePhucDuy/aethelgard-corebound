#ifndef BOAR_KING_H
#define BOAR_KING_H

#include "entities/Monster.h"
#include <vector>

// Forward declaration
class Player;

/**
 * @brief Lớp BoarKing — BOSS canh giữ Cổng Cửa trên Đỉnh Đền Thờ.
 *
 * KỊCH BẢN BOSS:
 * - Tuần tra quanh bệ cầu thang (STAIRS_DOWN), aggro 6 ô cùng tầng.
 * - Mỗi lượt thứ 3: [LÃO HÚC] — lao thẳng tối đa 3 ô về phía người chơi,
 *   nếu kết thúc kề cạnh thì húc x1.5 sát thương + đẩy lùi người chơi 1 ô.
 * - Khi HP xuống dưới 30%: [CỰC GIẢN] — ATK +6, aggro tăng lên 8 ô (một lần duy nhất).
 * - Được vẽ to hơn (x1.35) và ánh vàng để phân biệt với heo thường.
 * - Chưa hạ boss thì bệ cầu thang bị PHONG ẤN (không thể sang màn mới).
 */
class BoarKing : public Monster {
private:
    bool enraged;

public:
    explicit BoarKing(const Position& pos);
    ~BoarKing() override = default;

    void act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) override;
    void onDeath(Player& player) override;
    void render(float scale = 2.0f, Vector2 offset = {0.0f, 0.0f}) const override;
};

#endif // BOAR_KING_H
#ifndef COMBAT_SYSTEM_H
#define COMBAT_SYSTEM_H

#include <vector>
#include <string>
#include "entities/Entity.h"

/**
 * @brief Lớp CombatSystem xử lý giao tranh chiến đấu giữa các Entity.
 * 
 * ĐẶC ĐIỂM OOP:
 * 1. Quan hệ kết hợp (Aggregation):
 *    - `CombatSystem` nhận tham chiếu `Entity& attacker` và `Entity& defender`.
 *    - Nó không sở hữu hay quản lý vòng đời của kẻ tấn công hay kẻ phòng thủ (tồn tại độc lập).
 * 
 * 2. Tính đa hình (Polymorphism):
 *    - Cho dù là `Player` đánh `Monster`, hay `Monster` đánh `Player`, hay giữa 2 `Monster`,
 *      hàm `attack()` đều xử lý nhất quán thông qua lớp cơ sở `Entity&`.
 */
class GameEngine;

class CombatSystem {
public:
    // Xử lý đòn tấn công giữa 2 thực thể và ghi lại nhật ký chiến đấu (có tham số ngầm định GameEngine* - Chương 2)
    static bool attack(Entity& attacker, Entity& defender, std::vector<std::string>& combatLog, GameEngine* engine = nullptr);
};

#endif // COMBAT_SYSTEM_H

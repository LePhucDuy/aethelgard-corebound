#ifndef POTION_H
#define POTION_H

#include "items/Item.h"

/**
 * @brief Lớp Potion kế thừa Item
 * Đại diện cho bình thuốc hồi phục máu khi sử dụng.
 */
class Potion : public Item {
private:
    int healAmount;

public:
    Potion(const std::string& name, const std::string& description, int healAmount, const Position& pos = {0, 0});

    // Override phương thức thuần ảo từ lớp cha Item (Polymorphism)
    bool use(Player* target) override;

    int getHealAmount() const { return healAmount; }
};

#endif // POTION_H

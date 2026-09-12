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
    int stackCount;

public:
    Potion(const std::string& name, const std::string& description, int healAmount, const Position& pos = {0, 0}, const std::string& textureId = "", int count = 1);

    // Override phương thức thuần ảo từ lớp cha Item (Polymorphism)
    bool use(Player* target) override;

    // Nạp chồng toán tử 1 ngôi tiền tố (Slide 9-11 Chương 4)
    // Tăng số lượng bình thuốc khi nhặt thêm
    Potion& operator++() {
        ++stackCount;
        return *this;
    }

    // Giảm số lượng bình thuốc khi tiêu thụ
    Potion& operator--() {
        if (stackCount > 0) --stackCount;
        return *this;
    }

    int getHealAmount() const { return healAmount; }
    int getStackCount() const { return stackCount; }
    void setStackCount(int count) { stackCount = count; }
};

#endif // POTION_H

#ifndef ARMOR_H
#define ARMOR_H

#include "items/Item.h"

/**
 * @brief Lớp Armor kế thừa từ Item (Inheritance: IS-A - Slide 5-8 Chương 5)
 * Đại diện cho trang bị phòng thủ (Khiên Hộ Vệ, Giáp Hộ Mệnh) giúp tăng chỉ số Defense.
 * 
 * ĐẶC ĐIỂM OOP:
 * - Kế thừa (Inheritance): Tái sử dụng thuộc tính name, description, pos, onGround, textureId từ Item.
 * - Đóng gói (Encapsulation): bonusDefense được bảo vệ ở phạm vi private.
 * - Đa hình (Polymorphism): Ghi đè phương thức thuần ảo use(Player* target).
 */
class Armor : public Item {
private:
    int bonusDefense;

public:
    Armor(const std::string& name, const std::string& description, int bonusDefense, 
          const Position& pos = {0, 0}, const std::string& textureId = "item_armor_shield");

    // Ghi đè phương thức thuần ảo từ lớp cha Item (Polymorphism)
    bool use(Player* target) override;

    int getBonusDefense() const { return bonusDefense; }
};

#endif // ARMOR_H


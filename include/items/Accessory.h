#ifndef ACCESSORY_H
#define ACCESSORY_H

#include "items/Item.h"

/**
 * @brief Lớp Accessory kế thừa từ Item (Inheritance: IS-A)
 * Đại diện cho trang sức ma thuật (Nhẫn Cổ Ngữ Ruby, Bùa Hộ Mệnh)
 * gia tăng kết hợp các chỉ số ATK, DEF và Max HP.
 * 
 * ĐẶC ĐIỂM OOP:
 * - Kế thừa (Inheritance): Item -> Accessory
 * - Đóng gói (Encapsulation): Che giấu bonusAttack, bonusDefense, bonusMaxHp
 * - Đa hình (Polymorphism): Ghi đè phương thức thuần ảo use(Player* target)
 */
class Accessory : public Item {
private:
    int bonusAttack;
    int bonusDefense;
    int bonusMaxHp;

public:
    Accessory(const std::string& name, const std::string& description,
              int bonusAttack, int bonusDefense, int bonusMaxHp,
              const Position& pos = {0, 0}, const std::string& textureId = "item_ring_power");

    // Ghi đè phương thức thuần ảo từ lớp cha Item (Polymorphism)
    bool use(Player* target) override;

    int getBonusAttack() const { return bonusAttack; }
    int getBonusDefense() const { return bonusDefense; }
    int getBonusMaxHp() const { return bonusMaxHp; }
};

#endif // ACCESSORY_H


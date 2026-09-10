#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <iostream>
#include "core/Position.h"

// Forward declaration
class Player;

/**
 * @brief Lớp trừu tượng Item (Abstract Base Class 3)
 * Đại diện cho mọi vật phẩm có thể rơi trong hầm ngục và nhặt vào túi đồ.
 * 
 * ĐẶC ĐIỂM OOP:
 * 1. Phương thức thuần ảo: `virtual bool use(Player* target) = 0;`
 *    - Mỗi loại vật phẩm (Vũ khí, Thuốc hồi máu, Giáp) có cách sử dụng và hiệu ứng hoàn toàn khác nhau.
 * 2. Đa hình (Polymorphism):
 *    - Túi đồ `Inventory` chứa danh sách con trỏ `std::unique_ptr<Item>`. Khi gọi `item->use(player)`,
 *      chương trình tự động gọi đúng hàm của lớp con (Weapon, Potion) mà không cần kiểm tra kiểu thủ công.
 */
class Item {
protected:
    std::string name;
    std::string description;
    Position pos;
    bool onGround; // true nếu đang rơi trên sàn hầm ngục, false nếu đã trong túi

public:
    Item(const std::string& name, const std::string& description, const Position& pos = {0, 0})
        : name(name), description(description), pos(pos), onGround(true) {}

    virtual ~Item() = default;

    // Phương thức thuần ảo: Tác dụng khi vật phẩm được người chơi sử dụng
    virtual bool use(Player* target) = 0;

    // Getters & Setters
    const std::string& getName() const { return name; }
    const std::string& getDescription() const { return description; }
    const Position& getPosition() const { return pos; }
    void setPosition(const Position& newPos) { pos = newPos; }
    bool isOnGround() const { return onGround; }
    void setOnGround(bool state) { onGround = state; }

    // In thông tin vật phẩm
    friend std::ostream& operator<<(std::ostream& os, const Item& item) {
        os << "[" << item.name << "] " << item.description;
        return os;
    }
};

#endif // ITEM_H

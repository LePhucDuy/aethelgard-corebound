#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include <memory>
#include <iostream>
#include "items/Item.h"
#include "core/Constants.h"

// Forward declaration
class Player;

/**
 * @brief Lớp Inventory quản lý túi đồ của nhân vật.
 * 
 * ĐẶC ĐIỂM OOP:
 * 1. Cấu trúc dữ liệu STL:
 *    - Quản lý danh sách các vật phẩm bằng `std::vector<std::unique_ptr<Item>>`.
 *    - Sử dụng `std::unique_ptr` đảm bảo quyền sở hữu độc quyền (Ownership) và tự động thu hồi
 *      bộ nhớ khi vật phẩm bị dùng hoặc xóa khỏi túi (nguyên lý RAII).
 * 
 * 2. Nạp chồng toán tử (Operator Overloading):
 *    - `operator[]`: Cho phép truy xuất nhanh vật phẩm theo chỉ số slot `inventory[0]`, `inventory[1]`.
 *      Đây là ví dụ kinh điển, tự nhiên và có ý nghĩa thực tế cao trong OOP.
 * 
 * 3. Đóng gói (Encapsulation):
 *    - Toàn bộ danh sách vật phẩm được giấu kín (private), chỉ tương tác qua `addItem()`, `useItem()`, `dropItem()`.
 */
class Inventory {
private:
    std::vector<std::unique_ptr<Item>> items;
    size_t capacity;

public:
    explicit Inventory(size_t capacity = Constants::MAX_INVENTORY_SLOTS);
    ~Inventory() = default;

    // Thêm vật phẩm vào túi
    bool addItem(std::unique_ptr<Item> item);

    // Sử dụng vật phẩm ở vị trí index
    bool useItem(size_t index, Player* player);

    // Lấy và rút vật phẩm ra khỏi túi đồ
    std::unique_ptr<Item> takeItem(size_t index);

    // Xóa vật phẩm ở vị trí index
    bool removeItem(size_t index);

    // Nạp chồng toán tử operator[] để truy cập slot vật phẩm
    Item* operator[](size_t index);
    const Item* operator[](size_t index) const;

    // Getters
    size_t getSize() const { return items.size(); }
    size_t getCapacity() const { return capacity; }
    bool isFull() const { return items.size() >= capacity; }
    bool isEmpty() const { return items.empty(); }

    // Hiển thị danh sách vật phẩm
    void display() const;
};

#endif // INVENTORY_H

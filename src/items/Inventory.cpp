#include "items/Inventory.h"
#include <iostream>

Inventory::Inventory(size_t capacity) : capacity(capacity) {}

bool Inventory::addItem(std::unique_ptr<Item> item) {
    if (!item) return false;
    if (isFull()) {
        std::cout << "[Túi đồ] Túi đồ đã đầy! Không thể nhặt thêm: " << item->getName() << std::endl;
        return false;
    }

    std::cout << "[Túi đồ] Đã thêm vào túi: " << item->getName() << std::endl;
    items.push_back(std::move(item));
    return true;
}

bool Inventory::useItem(size_t index, Player* player) {
    if (index >= items.size() || !items[index]) {
        std::cout << "[Túi đồ] Vị trí ô vật phẩm không hợp lệ!" << std::endl;
        return false;
    }

    // Gọi đa hình (Polymorphism): use() của Weapon hoặc Potion
    bool success = items[index]->use(player);
    if (success) {
        // Sau khi dùng thành công (ví dụ bình thuốc hồi máu), xóa vật phẩm khỏi túi
        items.erase(items.begin() + index);
        return true;
    }
    return false;
}

std::unique_ptr<Item> Inventory::takeItem(size_t index) {
    if (index >= items.size()) return nullptr;

    std::unique_ptr<Item> taken = std::move(items[index]);
    items.erase(items.begin() + index);
    return taken;
}

bool Inventory::removeItem(size_t index) {
    if (index >= items.size()) return false;
    items.erase(items.begin() + index);
    return true;
}

Item* Inventory::operator[](size_t index) {
    if (index >= items.size()) return nullptr;
    return items[index].get();
}

const Item* Inventory::operator[](size_t index) const {
    if (index >= items.size()) return nullptr;
    return items[index].get();
}

void Inventory::display() const {
    std::cout << "--- TÚI ĐỒ (" << items.size() << "/" << capacity << ") ---" << std::endl;
    if (items.empty()) {
        std::cout << "(Trống)" << std::endl;
        return;
    }

    for (size_t i = 0; i < items.size(); ++i) {
        std::cout << " [" << i + 1 << "] " << *items[i] << std::endl;
    }
}

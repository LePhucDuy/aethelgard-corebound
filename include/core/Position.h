#ifndef POSITION_H
#define POSITION_H

#include <iostream>
#include <cmath>

/**
 * @brief Struct Position biểu diễn tọa độ ô lưới (Grid Coordinate) trong hầm ngục.
 * 
 * Nạp chồng toán tử (Operator Overloading):
 * - operator== : Kiểm tra 2 thực thể có đứng cùng 1 ô không (phát hiện va chạm, nhặt đồ).
 * - operator!= : Kiểm tra khác vị trí.
 * - operator+  : Cộng dồn vector di chuyển (offset).
 * - operator<< : In tọa độ ra stream (phục vụ ghi log, debug và xuất console).
 */
struct Position {
    int x;
    int y;

    Position() : x(0), y(0) {}
    Position(int x, int y) : x(x), y(y) {}

    // Toán tử nạp chồng so sánh vị trí
    bool operator==(const Position& other) const {
        return (x == other.x && y == other.y);
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }

    // Toán tử cộng vị trí với vector độ dời
    Position operator+(const Position& offset) const {
        return Position(x + offset.x, y + offset.y);
    }

    Position& operator+=(const Position& offset) {
        x += offset.x;
        y += offset.y;
        return *this;
    }

    // Tính khoảng cách Manhattan (khoảng cách lưới chuẩn của game Roguelike theo lượt)
    int manhattanDistanceTo(const Position& other) const {
        return std::abs(x - other.x) + std::abs(y - other.y);
    }

    // Nạp chồng toán tử xuất stream
    friend std::ostream& operator<<(std::ostream& os, const Position& pos) {
        os << "(" << pos.x << ", " << pos.y << ")";
        return os;
    }
};

#endif // POSITION_H

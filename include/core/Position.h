#ifndef POSITION_H
#define POSITION_H

#include <iostream>
#include <cmath>
#include "raylib.h"

/**
 * @brief Struct Position biểu diễn tọa độ ô lưới (Grid Coordinate) trong hầm ngục.
 * 
 * TIÊU CHÍ OOP CHƯƠNG 3 (Lớp & Đối tượng) & CHƯƠNG 4 (Quá tải toán tử):
 * 1. Constructor: Khởi tạo mặc định, có tham số, và Constructor sao chép (Copy Constructor).
 * 2. Con trỏ this: Dùng trong toán tử gán sao chép (`return *this;`).
 * 3. Nạp chồng toán tử toàn diện:
 *    - `operator==`, `operator!=`: So sánh tọa độ trùng khớp / khác nhau.
 *    - `operator<`: So sánh thứ tự từ điển, cho phép sắp xếp và lưu trong `std::map<Position, ...>`.
 *    - `operator+`, `operator-`: Phép cộng / trừ vector độ dời.
 *    - `operator+=`, `operator-=`: Phép gán kết hợp.
 *    - `operator<<`: Nạp chồng toán tử xuất luồng (Stream Insertion) qua hàm bạn (friend).
 *    - `operator>>`: Nạp chồng toán tử nhập luồng (Stream Extraction) qua hàm bạn (friend).
 */
struct Position {
    int x;
    int y;

    // 1. Constructor mặc định
    Position() : x(0), y(0) {}

    // 2. Constructor có tham số
    Position(int x, int y) : x(x), y(y) {}

    // 3. Constructor sao chép (Copy Constructor - Slide 28-30 Chương 3)
    Position(const Position& other) : x(other.x), y(other.y) {}

    // 4. Toán tử gán sao chép (Copy Assignment Operator - Slide 23 Chương 4)
    Position& operator=(const Position& other) {
        if (this != &other) { // Kiểm tra tự gán thông qua con trỏ this
            x = other.x;
            y = other.y;
        }
        return *this; // Trả về tham chiếu đối tượng hiện tại
    }

    // 5. Nạp chồng toán tử so sánh (Slide 6-8 Chương 4)
    bool operator==(const Position& other) const {
        return (x == other.x && y == other.y);
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }

    bool operator<(const Position& other) const {
        return (x < other.x) || (x == other.x && y < other.y);
    }

    // 6. Nạp chồng toán tử số học
    Position operator+(const Position& offset) const {
        return Position(x + offset.x, y + offset.y);
    }

    Position operator-(const Position& offset) const {
        return Position(x - offset.x, y - offset.y);
    }

    Position& operator+=(const Position& offset) {
        x += offset.x;
        y += offset.y;
        return *this;
    }

    Position& operator-=(const Position& offset) {
        x -= offset.x;
        y -= offset.y;
        return *this;
    }

    // Tính khoảng cách Manhattan (khoảng cách lưới chuẩn của game Roguelike theo lượt)
    int manhattanDistanceTo(const Position& other) const {
        return std::abs(x - other.x) + std::abs(y - other.y);
    }

    // 7. Nạp chồng toán tử xuất stream bằng hàm bạn (friend function - Slide 25 Chương 4)
    friend std::ostream& operator<<(std::ostream& os, const Position& pos) {
        os << "(" << pos.x << ", " << pos.y << ")";
        return os;
    }

    // 8. Nạp chồng toán tử nhập stream bằng hàm bạn (friend function - Slide 25 Chương 4)
    friend std::istream& operator>>(std::istream& is, Position& pos) {
        char ch = 0;
        // Bỏ qua khoảng trắng đầu dòng
        while (is.good() && std::isspace(is.peek())) {
            is.get();
        }
        // Hỗ trợ đọc cả dạng "(x, y)" hoặc "x y"
        if (is.peek() == '(') {
            is >> ch >> pos.x >> ch >> pos.y >> ch;
        } else {
            is >> pos.x >> pos.y;
        }
        return is;
    }

    // 9. Nạp chồng toán tử chuyển đổi kiểu (User-Defined Conversion Operator - Slide 24 Chương 4)
    // Tự động chuyển đổi tọa độ ô tile sang tọa độ Vector2 của Raylib (pixel thế giới)
    operator Vector2() const {
        return Vector2{ static_cast<float>(x) * 32.0f, static_cast<float>(y) * 32.0f };
    }

    // 10. Toán tử 1 ngôi tiền tố & hậu tố (Slide 9-11 Chương 4)
    // Tăng/giảm tọa độ X một bước lưới
    Position& operator++() {
        ++x;
        return *this;
    }

    Position operator++(int) {
        Position temp = *this;
        ++x;
        return temp;
    }

    Position& operator--() {
        --x;
        return *this;
    }

    Position operator--(int) {
        Position temp = *this;
        --x;
        return temp;
    }
};

/**
 * @brief Functor so sánh khoảng cách giữa hai tọa độ tới một tâm gốc (Chương 4 - Slide 17)
 * Nạp chồng toán tử gọi hàm operator() để làm tiêu chuẩn so sánh tìm kiếm mục tiêu gần nhất.
 */
struct DistanceComparator {
    Position origin;
    explicit DistanceComparator(const Position& orig) : origin(orig) {}

    bool operator()(const Position& a, const Position& b) const {
        return origin.manhattanDistanceTo(a) < origin.manhattanDistanceTo(b);
    }
};

#endif // POSITION_H

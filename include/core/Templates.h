#ifndef TEMPLATES_H
#define TEMPLATES_H

#include <algorithm>
#include <cmath>

/**
 * @file Templates.h
 * @brief Tập hợp các Khuôn mẫu hàm (Function Templates) dùng chung trong dự án.
 * 
 * TIÊU CHÍ OOP CHƯƠNG 7 (Slide 4-13: Function Template):
 * 1. Định nghĩa khuôn mẫu hàm độc lập với kiểu dữ liệu, cho phép tái sử dụng
 *    cho số nguyên (int), số thực (float, double) hoặc các đối tượng tùy biến.
 * 2. Trình biên dịch sẽ tự động sinh mã chuyên biệt (Template Instantiation)
 *    khi hàm được gọi với kiểu dữ liệu tương ứng.
 */

namespace CoreTemplates {

    /**
     * @brief Khuôn mẫu hàm giới hạn giá trị trong đoạn [minVal, maxVal]
     * Minh họa Slide 4-8 Chương 7.
     */
    template <typename T>
    T clampValue(T val, T minVal, T maxVal) {
        if (val < minVal) return minVal;
        if (val > maxVal) return maxVal;
        return val;
    }

    /**
     * @brief Khuôn mẫu hàm hoán vị hai giá trị sử dụng tham chiếu
     * Minh họa Slide 9-11 Chương 7.
     */
    template <typename T>
    void swapValues(T& a, T& b) {
        T temp = a;
        a = b;
        b = temp;
    }

    /**
     * @brief Khuôn mẫu hàm tìm giá trị lớn hơn giữa 2 phần tử
     */
    template <typename T>
    T getMaxValue(T a, T b) {
        return (a > b) ? a : b;
    }

    /**
     * @brief Khuôn mẫu hàm với nhiều tham số kiểu dữ liệu khác nhau (Slide 11 Chương 7)
     * Tính khoảng cách Chebyshev giữa 2 đối tượng bất kỳ có phương thức getPosition()
     */
    template <typename T1, typename T2>
    int calculateChebyshevDistance(const T1& obj1, const T2& obj2) {
        int dx = std::abs(obj1.getPosition().x - obj2.getPosition().x);
        int dy = std::abs(obj1.getPosition().y - obj2.getPosition().y);
        return (dx > dy) ? dx : dy;
    }

} // namespace CoreTemplates

#endif // TEMPLATES_H


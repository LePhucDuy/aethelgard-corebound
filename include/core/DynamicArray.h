#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <iostream>
#include <stdexcept>

/**
 * @brief Khuôn mẫu lớp DynamicArray (Class Template)
 * Cấu trúc mảng động tự co giãn, tự quản lý bộ nhớ động bằng con trỏ thô.
 * 
 * TIÊU CHÍ OOP TOÀN DIỆN:
 * 1. Chương 2: Quản lý bộ nhớ động cấp phát với `new[]` và thu hồi bằng `delete[]`.
 * 2. Chương 3: 
 *    - Constructor khởi tạo kích thước mặc định.
 *    - Destructor giải phóng vùng nhớ con trỏ để chống rò rỉ RAM (Memory Leak).
 *    - Copy Constructor thực hiện sao chép sâu (Deep Copy) toàn bộ mảng dữ liệu.
 *    - Con trỏ `this`: kiểm tra tự gán trong toán tử gán.
 * 3. Chương 4:
 *    - Nạp chồng toán tử gán sao chép (`operator=`).
 *    - Nạp chồng toán tử chỉ số mảng (`operator[]`) cho cả đối tượng thường và hằng (const).
 *    - Nạp chồng toán tử xuất luồng (`operator<<`) dạng hàm bạn (friend).
 * 4. Chương 7 (Slide 14-25):
 *    - Class Template với tham số kiểu `template <typename T>`.
 *    - Khai báo bạn bè của khuôn mẫu lớp (Slide 21-22).
 */
template <typename T>
class DynamicArray {
private:
    T* data;           // Con trỏ quản lý mảng động trên vùng nhớ Heap
    size_t capacity;   // Sức chứa tối đa hiện tại
    size_t count;      // Số lượng phần tử thực tế đang lưu

    // Tự động mở rộng gấp đôi kích thước vùng nhớ khi mảng đầy
    void resize(size_t newCapacity) {
        T* newData = new T[newCapacity];
        for (size_t i = 0; i < count; ++i) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

public:
    // 1. Constructor mặc định (Slide 19-23 Chương 3)
    explicit DynamicArray(size_t initialCapacity = 8)
        : data(new T[initialCapacity]), capacity(initialCapacity), count(0) {}

    // 2. Destructor - Giải phóng con trỏ vùng nhớ động (Slide 31-34 Chương 3)
    ~DynamicArray() {
        delete[] data;
        data = nullptr;
    }

    // 3. Constructor sao chép - Sao chép sâu (Deep Copy) (Slide 28-30 Chương 3)
    DynamicArray(const DynamicArray& other)
        : data(new T[other.capacity]), capacity(other.capacity), count(other.count) {
        for (size_t i = 0; i < count; ++i) {
            data[i] = other.data[i];
        }
    }

    // 4. Toán tử gán sao chép (Copy Assignment Operator - Slide 23 Chương 4)
    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) { // Kiểm tra chống tự gán bằng con trỏ this
            delete[] data;    // Giải phóng bộ nhớ cũ
            capacity = other.capacity;
            count = other.count;
            data = new T[capacity];
            for (size_t i = 0; i < count; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    // 5. Thêm phần tử vào cuối mảng
    void push_back(const T& item) {
        if (count >= capacity) {
            resize(capacity * 2);
        }
        data[count++] = item;
    }

    // 6. Nạp chồng toán tử truy xuất chỉ số mảng (Slide 16 Chương 4)
    T& operator[](size_t index) {
        if (index >= count) {
            throw std::out_of_range("Chi so vuot qua gioi han cua DynamicArray!");
        }
        return data[index];
    }

    const T& operator[](size_t index) const {
        if (index >= count) {
            throw std::out_of_range("Chi so vuot qua gioi han cua DynamicArray!");
        }
        return data[index];
    }

    // 7. Getters nghiệp vụ
    size_t size() const { return count; }
    size_t getCapacity() const { return capacity; }
    bool empty() const { return count == 0; }

    void clear() {
        count = 0;
    }

    // 8. Khai báo hàm bạn của khuôn mẫu lớp (Slide 21-22 Chương 7)
    template <typename U>
    friend std::ostream& operator<<(std::ostream& os, const DynamicArray<U>& arr);
};

// Cài đặt toán tử xuất luồng cho khuôn mẫu lớp
template <typename U>
std::ostream& operator<<(std::ostream& os, const DynamicArray<U>& arr) {
    os << "[";
    for (size_t i = 0; i < arr.count; ++i) {
        os << arr.data[i];
        if (i + 1 < arr.count) os << ", ";
    }
    os << "]";
    return os;
}

#endif // DYNAMIC_ARRAY_H


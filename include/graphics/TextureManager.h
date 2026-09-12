#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <string>
#include <map>
#include <iostream>
#include <raylib.h>

/**
 * @brief Lớp TextureManager áp dụng mẫu thiết kế Singleton kết hợp STL std::map
 * để quản lý, cache và giải phóng các tài nguyên hình ảnh (Texture2D) trên GPU VRAM.
 * 
 * Ưu điểm OOP:
 * - Tránh nạp trùng lặp ảnh nhiều lần gây tốn bộ nhớ.
 * - Quản lý tập trung tài nguyên, đảm bảo toàn bộ Texture được Unload đúng cách khi tắt game.
 */
class TextureManager {
private:
    std::map<std::string, Texture2D> textures; // STL map lưu trữ theo id

    TextureManager() = default;
    ~TextureManager();

    // Vô hiệu hóa copy constructor và copy assignment operator (chuẩn Singleton)
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

public:
    static TextureManager& getInstance();

    // Nạp một texture và gán định danh (ID)
    bool load(const std::string& id, const std::string& filePath);

    // Lấy texture theo ID đã nạp
    const Texture2D& get(const std::string& id) const;

    // Kiểm tra texture đã tồn tại chưa
    bool has(const std::string& id) const;

    // Giải phóng toàn bộ texture khỏi VRAM
    void unloadAll();
};

#endif // TEXTURE_MANAGER_H

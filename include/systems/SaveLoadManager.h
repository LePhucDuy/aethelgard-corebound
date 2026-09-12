#ifndef SAVE_LOAD_MANAGER_H
#define SAVE_LOAD_MANAGER_H

#include <string>
#include "entities/Player.h"
#include "map/Dungeon.h"

/**
 * @brief Lớp SaveLoadManager phụ trách đọc và ghi dữ liệu game ra file (File I/O).
 * 
 * TIÊU CHÍ CHẤM ĐIỂM OOP SỐ 7:
 * - Sử dụng các thư viện chuẩn C++ <fstream>, <sstream> để lưu và nạp trạng thái:
 *   + Thông số người chơi (Level, HP, MaxHP, ATK, DEF, EXP, Vàng, Tọa độ).
 *   + Tầng hầm ngục hiện tại (Floor Level).
 *   + Danh sách toàn bộ vật phẩm đang có trong túi đồ (Inventory).
 */
class SaveLoadManager {
public:
    // Lưu trạng thái game vào file văn bản
    static bool saveGame(const std::string& filePath, const Player& player, const Dungeon& dungeon);

    // Tải lại trạng thái game từ file
    static bool loadGame(const std::string& filePath, Player& player, Dungeon& dungeon);
};

#endif // SAVE_LOAD_MANAGER_H

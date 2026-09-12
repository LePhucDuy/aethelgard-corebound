#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <vector>
#include <string>
#include <raylib.h>
#include "entities/Player.h"
#include "map/Dungeon.h"

/**
 * @brief Định danh trạng thái tổng thể của trò chơi.
 */
enum class GameState {
    RUNNING,
    GAME_OVER,
    VICTORY
};

/**
 * @brief Lớp GameEngine đóng vai trò điều phối chính của toàn bộ trò chơi.
 * 
 * ĐẶC ĐIỂM OOP:
 * - Áp dụng mẫu kiến trúc điều phối tổng thể (Facade / Coordinator).
 * - Kết nối chặt chẽ giữa Input, Map, Entities, Combat, HUD và File I/O.
 * - Quản lý vòng lặp lượt đi (Turn-based logic):
 *   + Người chơi hành động (Đi lại / Tấn công / Dùng item) -> Kích hoạt lượt của Quái vật.
 */
class GameEngine {
private:
    Player player;
    Dungeon dungeon;
    GameState state;
    std::vector<std::string> combatLog;
    Camera2D camera;
    Font fontMain;
    float moveTimer;
    float attackTimer;
    float userZoomOffset;

    // Kịch bản màn chơi
    int currentZone;               // Khu vực hiện tại (-1: chưa vào khu nào)
    std::string bannerText;        // Banner thông báo khu mới
    float bannerTimer;             // Thời gian còn hiển thị banner
    int monstersDefeated;          // Số quái đã hạ (thống kê chiến thắng)

    // Trạng thái giao diện
    bool showInventory;            // true: đang mở bảng túi đồ (phím B/I/Tab hoặc click chuột)
    bool showCombatLog;            // true: đang mở khung nhật ký chiến đấu (phím L)

    // Ghi đè vị trí xuất phát (tuỳ chọn, phục vụ debug/test từng khu: --spawn X Y)
    int spawnOverrideX;
    int spawnOverrideY;

    void handleInput();
    void processMonsterTurn();
    void renderHUD() const;
    void drawText(const char* text, float posX, float posY, float fontSize, Color color) const;

public:
    GameEngine(int spawnX = -1, int spawnY = -1, bool startWithInventory = false);
    ~GameEngine();

    // Khởi tạo các tài nguyên (Textures, Animations, Floor 1)
    void init();

    // Cập nhật logic theo khung thời gian delta
    void update(float deltaTime);

    // Vẽ toàn bộ thế giới game và giao diện người dùng
    void render() const;

    // Vòng lặp chính của game (hỗ trợ chụp ảnh tự động khi truyền đường dẫn)
    void run(const std::string& autoScreenshot = "");
};

#endif // GAME_ENGINE_H

#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <vector>
#include <string>
#include "core/Position.h"
#include "core/DynamicArray.h"
#include "core/Templates.h"
#include "systems/EventSystem.h"
#include "entities/Player.h"
#include "graphics/DamagePopup.h"
#include "graphics/GoldParticle.h"
#include "items/Armor.h"
#include "items/Accessory.h"
#include "items/Chest.h"
#include "map/Dungeon.h"
#include <vector>
#include <string>
#include <memory>
#include <raylib.h>

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
    DynamicArray<std::string> templateCombatLog; // Ứng dụng Class Template tự xây dựng (Chương 7)
    mutable DynamicArray<DamagePopup> activeDamagePopups; // Mảng động quản lý số sát thương nổi thời gian thực (Chương 7)
    mutable DynamicArray<GoldParticle> activeGoldParticles; // Mảng động quản lý các hạt vàng rơi khi quái chết (Chương 7)
    Camera2D camera;
    Font fontMain;
    float moveTimer;
    float attackTimer;
    float edgeSlipTimer;
    float userZoomOffset;

    // Trạng thái lún đầm lầy (Swamp sinking)
    bool isSinking;
    bool submergedInSwamp;
    float sinkTimer;
    float sinkDuration;
    float sinkDepth;

    // Hoạt cảnh tử trận mượt mà (Smooth death sequence)
    bool isDying;
    float deathTimer;
    float deathDuration;

    // Kịch bản màn chơi
    int currentZone;               // Khu vực hiện tại (-1: chưa vào khu nào)
    std::string bannerText;        // Banner thông báo khu mới
    float bannerTimer;             // Thời gian còn hiển thị banner
    int monstersDefeated;          // Số quái đã hạ (thống kê chiến thắng)

    // Boss Boar King Cinematic & Cảnh báo nguy hiểm
    bool bossCinematicTriggered;   // true khi người chơi bước vào đấu trường Boss (x >= 130)
    float bossWarningTimer;        // Thời gian đếm ngược hiển thị cảnh báo trùm
    float screenShake;             // Cường độ rung màn hình (Screen Shake)

    // Trạng thái giao diện & Tiêu thụ vàng
    bool showInventory;            // true: đang mở bảng túi đồ (phím B/I/Tab hoặc click chuột)
    bool showShop;                 // true: đang mở cửa hàng hầm ngục (phím P hoặc click chuột)
    bool showForge;                // true: đang mở đe rèn cường hóa (phím U hoặc click chuột)
    bool showCombatLog;            // true: đang mở khung nhật ký chiến đấu (phím L)

    // Thông báo phản hồi tức thì trên các cửa sổ giao diện (Shop, Forge)
    std::string shopNotification;
    Color shopNotificationColor;
    float shopNotificationTimer;

    std::string forgeNotification;
    Color forgeNotificationColor;
    float forgeNotificationTimer;

    // Vệt kiếm chém hình vòng cung (Slash Arc Trail) & Khựng đòn (Hit Stop)
    struct SlashArc {
        Vector2 center;
        float radius;
        float startAngle;
        float endAngle;
        float maxLifetime;
        float timer;
        bool facingRight;
        Color baseColor;
    };
    std::vector<SlashArc> activeSlashArcs;
    float hitStopTimer;
    mutable float playerGhostHp;

    // Hệ thống Observer Pattern (GoF Behavioral Pattern)
    std::unique_ptr<AchievementObserver> achievementObserver;
    std::unique_ptr<CombatLogObserver> combatLogObserver;
    std::string achievementBanner;
    float achievementBannerTimer;

    // Ghi đè vị trí xuất phát (tuỳ chọn, phục vụ debug/test từng khu: --spawn X Y)
    int spawnOverrideX;
    int spawnOverrideY;
    bool startLethalOverride;

    void handleInput();
    void renderHUD() const;
    void drawText(const char* text, float posX, float posY, float fontSize, Color color) const;

    // Quản lý hiệu ứng vàng rơi và các cơ chế tiêu thụ vàng
    void updateGoldParticles(float deltaTime);
    void renderGoldParticles(Vector2 offset) const;
    void renderShop() const;
    void renderForge() const;
    void buyShopItem(int slot);
    void triggerForgeUpgrade();
    void interactWithChest();
    void tryPickupItemAtPlayerPos();

public:
    GameEngine(int spawnX = -1, int spawnY = -1, bool startWithInventory = false, bool startLethal = false, bool startWithShop = false, bool startWithForge = false);
    ~GameEngine();

    // Khởi tạo các tài nguyên (Textures, Animations, Floor 1)
    void init();

    // Cập nhật logic theo khung thời gian delta
    void update(float deltaTime);

    // Vẽ toàn bộ thế giới game và giao diện người dùng (hỗ trợ chụp ảnh màn hình đồng bộ trước SwapBuffers)
    void render(const std::string& screenshotPath = "") const;

    // Vòng lặp chính của game (hỗ trợ chụp ảnh tự động khi truyền đường dẫn)
    void run(const std::string& autoScreenshot = "");

    // Quản lý số sát thương nổi thời gian thực (Class Template DynamicArray - Chương 7)
    void addDamagePopup(const std::string& text, float worldX, float worldY, Color color = YELLOW, float duration = 0.8f);

    // Hiệu ứng bung tỏa hạt vàng rơi ra từ thân quái vật (Gold Burst & Magnet Attract)
    void spawnGoldBurst(float worldX, float worldY, float groundY, int totalGold, int count = 6);

    // Hiệu ứng vệt kiếm chém hình vòng cung & Khựng đòn (Game Feel)
    void triggerHitStop(float duration = 0.06f);
    void addSlashArc(Vector2 center, bool facingRight);

    Dungeon& getDungeon() { return dungeon; }
    const Dungeon& getDungeon() const { return dungeon; }
    Player& getPlayer() { return player; }
    const Player& getPlayer() const { return player; }
    std::vector<std::string>& getCombatLog() { return combatLog; }

    // Hàm tự kiểm thử học thuật cho toàn bộ 7 chương OOP của trường UTH
    static void runOOPAcademicTests();
};

#endif // GAME_ENGINE_H

#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>

// Forward declaration
class Entity;

/**
 * @brief Định danh các loại sự kiện trong game (Event Types)
 */
enum class GameEventType {
    MONSTER_KILLED,       // Quái vật bị tiêu diệt
    BOSS_DEFEATED,        // Đại boss bị tiêu diệt
    PLAYER_DAMAGED,       // Người chơi nhận sát thương
    GOLD_GAINED,          // Thu thập tiền vàng
    FORGE_UPGRADED,       // Cường hóa vũ khí tại đe rèn
    CHEST_OPENED,         // Mở rương kho báu hoàng kim
    ACHIEVEMENT_UNLOCKED  // Mở khóa thành tựu danh giá
};

/**
 * @brief Dữ liệu truyền tải khi phát tán sự kiện (Event Data Payload)
 */
struct GameEvent {
    GameEventType type;
    int value;
    std::string message;
    const Entity* source;
    const Entity* target;

    GameEvent(GameEventType t, int val = 0, const std::string& msg = "",
              const Entity* src = nullptr, const Entity* tgt = nullptr)
        : type(t), value(val), message(msg), source(src), target(tgt) {}
};

/**
 * @brief Giao diện trừu tượng Observer (Interface IGameObserver)
 * ĐẶC ĐIỂM OOP:
 * - Áp dụng mẫu thiết kế Observer Pattern (GoF Behavioral Pattern).
 * - Lớp cơ sở trừu tượng thuần ảo định nghĩa hợp đồng thông báo `onNotify`.
 * - Đảm bảo nguyên lý Dependency Inversion: Bộ điều phối không phụ thuộc vào lớp cụ thể.
 */
class IGameObserver {
public:
    virtual ~IGameObserver() = default;
    virtual void onNotify(const GameEvent& event) = 0;
    virtual std::string getObserverName() const = 0;
};

/**
 * @brief Bộ điều phối sự kiện tập trung (Subject / Event Dispatcher)
 * ĐẶC ĐIỂM OOP:
 * - Singleton Pattern kết hợp Observer Pattern.
 * - Quản lý danh sách các Observer đăng ký theo từng loại sự kiện.
 * - Phát tán sự kiện (Publish-Subscribe) giúp giải mã ghép nối (Decoupling) 100%.
 */
class EventDispatcher {
private:
    std::map<GameEventType, std::vector<IGameObserver*>> listeners;
    EventDispatcher() = default;

public:
    // Hủy copy và gán để đảm bảo tính duy nhất của Singleton (Chương 3)
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;

    static EventDispatcher& getInstance() {
        static EventDispatcher instance;
        return instance;
    }

    void subscribe(GameEventType type, IGameObserver* observer);
    void unsubscribe(GameEventType type, IGameObserver* observer);
    void notify(const GameEvent& event);
    void clear();
};

/**
 * @brief Observer chuyên trách theo dõi và trao Thành Tựu Danh Giá (Achievement System)
 * Kế thừa từ IGameObserver (Inheritance: IS-A)
 */
class AchievementObserver : public IGameObserver {
private:
    std::vector<std::string> unlockedAchievements;
    std::vector<std::string> pendingBanners; // Hàng đợi thông báo hiển thị lên màn hình
    int totalKills;
    int totalGoldEarned;

public:
    AchievementObserver();
    ~AchievementObserver() override = default;

    void onNotify(const GameEvent& event) override;
    std::string getObserverName() const override { return "AchievementObserver"; }

    bool hasAchievement(const std::string& achName) const;
    void unlockAchievement(const std::string& achName, const std::string& desc);

    const std::vector<std::string>& getUnlockedAchievements() const { return unlockedAchievements; }
    bool hasPendingBanner() const { return !pendingBanners.empty(); }
    std::string popLatestBanner();
};

/**
 * @brief Observer phụ trách ghi nhận lịch sử chiến đấu tự động vào Combat Log
 */
class CombatLogObserver : public IGameObserver {
private:
    std::vector<std::string>* combatLogRef;

public:
    explicit CombatLogObserver(std::vector<std::string>* logRef = nullptr) : combatLogRef(logRef) {}
    ~CombatLogObserver() override = default;

    void setCombatLogRef(std::vector<std::string>* logRef) { combatLogRef = logRef; }
    void onNotify(const GameEvent& event) override;
    std::string getObserverName() const override { return "CombatLogObserver"; }
};

#endif // EVENT_SYSTEM_H


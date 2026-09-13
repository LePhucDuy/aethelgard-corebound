#include "systems/EventSystem.h"
#include "entities/Entity.h"
#include <iostream>

// =============================================================================
// EVENT DISPATCHER (SUBJECT TRONG OBSERVER PATTERN)
// =============================================================================

void EventDispatcher::subscribe(GameEventType type, IGameObserver* observer) {
    if (!observer) return;
    auto& list = listeners[type];
    if (std::find(list.begin(), list.end(), observer) == list.end()) {
        list.push_back(observer);
    }
}

void EventDispatcher::unsubscribe(GameEventType type, IGameObserver* observer) {
    if (!observer) return;
    auto it = listeners.find(type);
    if (it != listeners.end()) {
        auto& list = it->second;
        list.erase(std::remove(list.begin(), list.end(), observer), list.end());
    }
}

void EventDispatcher::notify(const GameEvent& event) {
    auto it = listeners.find(event.type);
    if (it != listeners.end()) {
        // Sao chép danh sách con trỏ để an toàn nếu observer tự hủy đăng ký khi đang xử lý
        auto targets = it->second;
        for (IGameObserver* obs : targets) {
            if (obs) {
                obs->onNotify(event);
            }
        }
    }
}

void EventDispatcher::clear() {
    listeners.clear();
}

// =============================================================================
// ACHIEVEMENT OBSERVER (HỆ THỐNG THEO DÕI THÀNH TỰU TỰ ĐỘNG)
// =============================================================================

AchievementObserver::AchievementObserver()
    : totalKills(0), totalGoldEarned(0) {
}

bool AchievementObserver::hasAchievement(const std::string& achName) const {
    return std::find(unlockedAchievements.begin(), unlockedAchievements.end(), achName) != unlockedAchievements.end();
}

void AchievementObserver::unlockAchievement(const std::string& achName, const std::string& desc) {
    if (hasAchievement(achName)) return;

    unlockedAchievements.push_back(achName);
    std::string banner = "[THANH TUU MO KHOA] " + achName + " - " + desc;
    pendingBanners.push_back(banner);

    std::cout << "\n=======================================================\n";
    std::cout << "  >>> " << banner << " <<<\n";
    std::cout << "=======================================================\n" << std::endl;

    // Phát lại sự kiện ACHIEVEMENT_UNLOCKED cho các hệ thống UI/Âm thanh
    GameEvent achEvent(GameEventType::ACHIEVEMENT_UNLOCKED, (int)unlockedAchievements.size(), banner);
    EventDispatcher::getInstance().notify(achEvent);
}

std::string AchievementObserver::popLatestBanner() {
    if (pendingBanners.empty()) return "";
    std::string b = pendingBanners.front();
    pendingBanners.erase(pendingBanners.begin());
    return b;
}

void AchievementObserver::onNotify(const GameEvent& event) {
    switch (event.type) {
        case GameEventType::MONSTER_KILLED: {
            totalKills++;
            if (totalKills == 1) {
                unlockAchievement("First Blood", "Ha guc quai vat dau tien trong cuoc hanh trinh!");
            }
            if (totalKills == 10) {
                unlockAchievement("Monster Slayer", "Tieu diet 10 quai vat doc chiem vung dat!");
            }
            break;
        }

        case GameEventType::BOSS_DEFEATED: {
            std::string bossName = event.message;
            if (event.target) bossName += " " + event.target->getName();

            if (bossName.find("Queen") != std::string::npos || bossName.find("Ong Chua") != std::string::npos) {
                unlockAchievement("Queen Vanquisher", "Chien thang Dai Boss Hoang Hau Ong Chua tren khong!");
            }
            if (bossName.find("King") != std::string::npos || bossName.find("Boar King") != std::string::npos || bossName.find("Heo") != std::string::npos) {
                unlockAchievement("King Slayer", "Ha guc Chua Heo Rung Boar King, mo khoa Cong Den Tho!");
            }
            break;
        }

        case GameEventType::GOLD_GAINED: {
            totalGoldEarned += event.value;
            if (totalGoldEarned >= 100) {
                unlockAchievement("Gold Hoarder", "Thu thap tich luy duoc 100 dong vang hoang kim!");
            }
            break;
        }

        case GameEventType::FORGE_UPGRADED: {
            int level = event.value;
            if (level >= 1) {
                unlockAchievement("Apprentice Blacksmith", "Thuc hien cuong hoa vu khi thanh cong tai De Ren!");
            }
            if (level >= 3) {
                unlockAchievement("Master Blacksmith", "Nang cap vu khi len Cap +3, suc manh vuot troi!");
            }
            break;
        }

        case GameEventType::CHEST_OPENED: {
            unlockAchievement("Treasure Hunter", "Tim thay va mo khoa Ruong Kho Bau Hoang Kim co!");
            break;
        }

        default:
            break;
    }
}

// =============================================================================
// COMBAT LOG OBSERVER (BỘ TỰ ĐỘNG GHI NHẬT KÝ CHIẾN TRẬN)
// =============================================================================

void CombatLogObserver::onNotify(const GameEvent& event) {
    if (!combatLogRef) return;

    if (event.type == GameEventType::ACHIEVEMENT_UNLOCKED) {
        combatLogRef->push_back(">>> " + event.message + " <<<");
    } else if (!event.message.empty()) {
        combatLogRef->push_back(event.message);
    }

    if (combatLogRef->size() > 6) {
        combatLogRef->erase(combatLogRef->begin());
    }
}


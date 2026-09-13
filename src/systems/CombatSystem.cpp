#include "systems/CombatSystem.h"
#include "systems/EventSystem.h"
#include "engine/GameEngine.h"
#include "entities/Monster.h"
#include "core/Constants.h"
#include <cstdlib>
#include <iostream>

bool CombatSystem::attack(Entity& attacker, Entity& defender, std::vector<std::string>& combatLog, GameEngine* engine) {
    if (!attacker.isAlive() || !defender.isAlive()) return false;

    // Tính toán sát thương
    int baseAtk = attacker.getAttack();
    bool isCrit = (std::rand() % 100) < 15; // 15% chí mạng
    if (isCrit) {
        baseAtk = static_cast<int>(baseAtk * 1.5f);
    }

    int oldHp = defender.getHp();
    defender.takeDamage(baseAtk);
    int damageTaken = oldHp - defender.getHp();

    // Hiển thị số sát thương nổi thời gian thực qua DynamicArray trong GameEngine
    if (engine && damageTaken > 0) {
        Vector2 defPos = defender.getPosition(); // Sử dụng toán tử chuyển đổi kiểu operator Vector2() (Chương 4)
        bool isPlayerAttack = (attacker.getName().find("Hiep Si") != std::string::npos);
        Color popColor = isPlayerAttack ? (isCrit ? RED : YELLOW) : MAROON;
        std::string popText = "-" + std::to_string(damageTaken) + (isCrit ? " CRIT!" : "");
        engine->addDamagePopup(popText, defPos.x + 8.0f, defPos.y - 14.0f, popColor, 0.9f);
    }

    // Tạo thông điệp nhật ký chiến đấu rõ ràng giữa đòn tấn công và đòn phản công
    std::string logMsg = "";
    if (attacker.getName().find("Hiep Si") != std::string::npos) {
        logMsg = "Hiep si vung kiem chem trung " + defender.getName();
        if (isCrit) logMsg += " [CHI MANG!]";
        logMsg += " gay " + std::to_string(damageTaken) + " sat thuong!";
    } else {
        logMsg = "[" + attacker.getName() + "] phan cong lai! Ban bi tru -" + std::to_string(damageTaken) + " HP.";
    }

    if (!defender.isAlive()) {
        logMsg += " -> " + defender.getName() + " da bi tieu diet!";
        
        Monster* m = dynamic_cast<Monster*>(&defender);

        // Phát tán sự kiện qua Mẫu thiết kế Observer Pattern (EventDispatcher)
        bool isBoss = (defender.getName().find("Queen") != std::string::npos ||
                       defender.getName().find("Boar King") != std::string::npos ||
                       defender.getName().find("Ong Chua") != std::string::npos ||
                       defender.getName().find("Chua Heo") != std::string::npos);

        if (isBoss) {
            GameEvent bossEvent(GameEventType::BOSS_DEFEATED, m ? m->getExpReward() : 100, defender.getName(), &attacker, &defender);
            EventDispatcher::getInstance().notify(bossEvent);
        } else {
            GameEvent killEvent(GameEventType::MONSTER_KILLED, m ? m->getExpReward() : 20, defender.getName(), &attacker, &defender);
            EventDispatcher::getInstance().notify(killEvent);
        }

        // Kích hoạt hiệu ứng văng hạt vàng rơi ra thế giới (Gold Burst Effect)
        if (m && engine && !m->isGoldDropped()) {
            m->setGoldDropped(true);
            Vector2 mPos = m->getVisualPosition();
            float groundY = (float)(m->getPosition().y * Constants::TILE_SIZE) + 8.0f;
            int gReward = m->getGoldReward();
            int coinCount = (gReward >= 50) ? 14 : ((gReward >= 20) ? 8 : 5);
            engine->spawnGoldBurst(mPos.x + 16.0f, mPos.y + 4.0f, groundY, gReward, coinCount);
        }
    } else if (damageTaken > 0 && defender.getName().find("Hiep Si") != std::string::npos) {
        GameEvent dmgEvent(GameEventType::PLAYER_DAMAGED, damageTaken, "Player damaged", &attacker, &defender);
        EventDispatcher::getInstance().notify(dmgEvent);
    }

    combatLog.push_back(logMsg);
    // Giữ nhật ký tối đa 5 dòng gần nhất để không tràn màn hình
    if (combatLog.size() > 5) {
        combatLog.erase(combatLog.begin());
    }

    std::cout << "[Combat Log] " << logMsg << std::endl;
    return true;
}

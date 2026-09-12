#include "systems/CombatSystem.h"
#include "engine/GameEngine.h"
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
    }

    combatLog.push_back(logMsg);
    // Giữ nhật ký tối đa 5 dòng gần nhất để không tràn màn hình
    if (combatLog.size() > 5) {
        combatLog.erase(combatLog.begin());
    }

    std::cout << "[Combat Log] " << logMsg << std::endl;
    return true;
}

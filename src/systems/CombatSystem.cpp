#include "systems/CombatSystem.h"
#include "systems/EventSystem.h"
#include "systems/AudioSystem.h"
#include "engine/GameEngine.h"
#include "entities/Monster.h"
#include "entities/Player.h"
#include "core/Constants.h"
#include <cstdlib>
#include <iostream>

bool CombatSystem::attack(Entity& attacker, Entity& defender, std::vector<std::string>& combatLog, GameEngine* engine) {
    if (!attacker.isAlive() || !defender.isAlive()) return false;

    // Kiểm tra tương tác Đỡ đòn & Phản đòn khi người chơi bị tấn công
    Player* pDefender = dynamic_cast<Player*>(&defender);
    bool isBlocked = false;

    if (pDefender) {
        if (pDefender->isParrying()) {
            // =================================================================
            // 1. PERFECT PARRY (Cửa sổ 0.18s đầu tiên)
            // =================================================================
            // - Sát thương nhận vào = 0 (Hoàn toàn miễn nhiễm)
            // - Phản phệ 150% sát thương của quái vật dội ngược trở lại
            // - Phát âm thanh keng kim loại parry_clash và tia sét hoàng kim
            int reflectDmg = std::max(6, static_cast<int>(attacker.getAttack() * 1.5f));
            int oldAttackerHp = attacker.getHp();
            attacker.takeDamage(reflectDmg);
            int actualReflect = oldAttackerHp - attacker.getHp();

            // Kích hoạt âm thanh keng kim loại
            AudioSystem::getInstance().play("parry_clash", 1.0f, 1.0f);

            // Hiển thị chữ nổi PARRY! vàng rực
            if (engine) {
                Vector2 pPos = pDefender->getPosition();
                engine->addDamagePopup("PARRY!", pPos.x + 8.0f, pPos.y - 18.0f, GOLD, 1.2f);
                Vector2 atkPos = attacker.getPosition();
                engine->addDamagePopup("-" + std::to_string(actualReflect) + " REFLECT!", atkPos.x + 8.0f, atkPos.y - 14.0f, ORANGE, 1.0f);
            }

            std::string logMsg = ">>> [PERFECT PARRY!] Hiep si phan don hoan hao! Doi phuong bi phan -" + std::to_string(actualReflect) + " HP!";
            if (!attacker.isAlive()) {
                logMsg += " -> " + attacker.getName() + " da bi phan che tieu diet!";
                Monster* mAttacker = dynamic_cast<Monster*>(&attacker);
                if (mAttacker) {
                    GameEvent killEvent(GameEventType::MONSTER_KILLED, mAttacker->getExpReward(), attacker.getName(), &defender, &attacker);
                    EventDispatcher::getInstance().notify(killEvent);
                    if (engine && !mAttacker->isGoldDropped()) {
                        mAttacker->setGoldDropped(true);
                        Vector2 mPos = mAttacker->getVisualPosition();
                        float groundY = (float)(mAttacker->getPosition().y * Constants::TILE_SIZE) + 8.0f;
                        int gReward = mAttacker->getGoldReward();
                        int coinCount = (gReward >= 50) ? 14 : ((gReward >= 20) ? 8 : 5);
                        engine->spawnGoldBurst(mPos.x + 16.0f, mPos.y + 4.0f, groundY, gReward, coinCount);
                    }
                }
            }

            combatLog.push_back(logMsg);
            if (combatLog.size() > 5) combatLog.erase(combatLog.begin());
            std::cout << "[Combat Log] " << logMsg << std::endl;
            return true;
        }
        else if (pDefender->isBlocking()) {
            // =================================================================
            // 2. BLOCK THƯỜNG (Sau 0.18s đến 0.45s)
            // =================================================================
            // - Giảm 75% sát thương
            // - Chặn đứng hoàn toàn hiệu ứng lây độc từ nọc ong
            isBlocked = true;
            AudioSystem::getInstance().play("shield_block", 0.95f, 1.0f);
        }
    }

    // Tính toán sát thương cơ bản
    int baseAtk = attacker.getAttack();
    bool isCrit = (std::rand() % 100) < 15; // 15% chí mạng
    if (isCrit) {
        baseAtk = static_cast<int>(baseAtk * 1.5f);
    }

    if (isBlocked) {
        baseAtk = std::max(1, static_cast<int>(baseAtk * 0.25f)); // Giảm 75% sát thương
    }

    int oldHp = defender.getHp();
    defender.takeDamage(baseAtk);
    int damageTaken = oldHp - defender.getHp();

    // Hiển thị số sát thương nổi thời gian thực qua DynamicArray trong GameEngine
    if (engine && damageTaken > 0) {
        Vector2 defPos = defender.getPosition();
        bool isPlayerAttack = (attacker.getName().find("Hiep Si") != std::string::npos);
        Color popColor = isPlayerAttack ? (isCrit ? RED : YELLOW) : (isBlocked ? SKYBLUE : MAROON);
        std::string popText = "-" + std::to_string(damageTaken);
        if (isCrit) popText += " CRIT!";
        if (isBlocked) popText += " BLOCKED";
        engine->addDamagePopup(popText, defPos.x + 8.0f, defPos.y - 14.0f, popColor, 0.9f);
    }

    // Phát âm thanh tương ứng
    if (attacker.getName().find("Hiep Si") != std::string::npos) {
        AudioSystem::getInstance().play("sword_slash", 0.85f, 1.0f);
        AudioSystem::getInstance().play("hit_impact", 0.90f, 1.0f);
    }

    // Cơ chế lây độc từ loài Ong (Poison DoT System)
    // Nếu quái vật là ong và người chơi không đỡ đòn
    if (pDefender && !isBlocked) {
        bool isQueenBee = (attacker.getName().find("Queen") != std::string::npos ||
                           attacker.getName().find("Ong Chua") != std::string::npos);
        bool isSmallBee = (attacker.getName().find("Bee") != std::string::npos ||
                           attacker.getName().find("Ong") != std::string::npos);

        if (isQueenBee) {
            // Hoàng hậu ong chúa: 100% tỉ lệ gây Độc Hoàng Gia (6s, 5 dmg/s)
            pDefender->applyPoison(6.0f, 5);
            AudioSystem::getInstance().play("poison_tick", 1.0f, 0.9f);
            if (engine) {
                Vector2 defPos = defender.getPosition();
                engine->addDamagePopup("DOC HOANG GIA!", defPos.x + 8.0f, defPos.y - 28.0f, Color{ 220, 80, 255, 255 }, 1.3f);
            }
        } else if (isSmallBee) {
            // Ong thợ nhỏ: 60% tỉ lệ gây Độc Thường (4s, 3 dmg/s)
            if ((std::rand() % 100) < 60) {
                pDefender->applyPoison(4.0f, 3);
                AudioSystem::getInstance().play("poison_tick", 0.9f, 1.1f);
                if (engine) {
                    Vector2 defPos = defender.getPosition();
                    engine->addDamagePopup("TRUNG DOC!", defPos.x + 8.0f, defPos.y - 28.0f, Color{ 190, 80, 240, 255 }, 1.1f);
                }
            }
        }
    }

    // Tạo thông điệp nhật ký chiến đấu rõ ràng giữa đòn tấn công và đòn phản công
    std::string logMsg = "";
    if (attacker.getName().find("Hiep Si") != std::string::npos) {
        logMsg = "Hiep si vung kiem chem trung " + defender.getName();
        if (isCrit) logMsg += " [CHI MANG!]";
        logMsg += " gay " + std::to_string(damageTaken) + " sat thuong!";
    } else {
        if (isBlocked) {
            logMsg = "[" + attacker.getName() + "] tan cong nhung bi Khien do chan! Chi mat -" + std::to_string(damageTaken) + " HP.";
        } else {
            logMsg = "[" + attacker.getName() + "] phan cong lai! Ban bi tru -" + std::to_string(damageTaken) + " HP.";
            if (pDefender && pDefender->isPoisoned()) {
                logMsg += " [Dinh Doc Ong!]";
            }
        }
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
            AudioSystem::getInstance().play("coin_pickup", 0.8f, 1.0f);
        }
    } else if (damageTaken > 0 && defender.getName().find("Hiep Si") != std::string::npos) {
        GameEvent dmgEvent(GameEventType::PLAYER_DAMAGED, damageTaken, "Player damaged", &attacker, &defender);
        EventDispatcher::getInstance().notify(dmgEvent);
    }

    combatLog.push_back(logMsg);
    if (combatLog.size() > 5) {
        combatLog.erase(combatLog.begin());
    }

    std::cout << "[Combat Log] " << logMsg << std::endl;
    return true;
}

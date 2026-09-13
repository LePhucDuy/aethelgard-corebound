#include "systems/Skill.h"
#include "entities/Player.h"
#include "engine/GameEngine.h"
#include "map/Dungeon.h"
#include <iostream>

SlashSkill::SlashSkill()
    : Skill("Vung Kiem", "Chem can chien bang thanh kiem sac ben [J]", 0.35f) {}

bool SlashSkill::execute(Player* user, GameEngine* engine) {
    if (!user || !canExecute()) return false;
    user->triggerAttack();
    triggerCooldown();
    return true;
}

DashSkill::DashSkill()
    : Skill("Luot Ne Don", "Luot nhanh 3 o ve phia truoc tranh don tan cong [L-Shift]", 1.2f) {}

bool DashSkill::execute(Player* user, GameEngine* engine) {
    if (!user || !canExecute()) return false;
    
    int dirX = user->isFacingRight() ? 1 : -1;
    if (engine) {
        Dungeon& dungeon = engine->getDungeon();
        for (int step = 1; step <= 3; ++step) {
            Position nextPos(user->getPosition().x + dirX, user->getPosition().y);
            if (dungeon.isWalkable(nextPos)) {
                user->moveBy(dirX, 0, dungeon);
            } else {
                break;
            }
        }
    }

    triggerCooldown();
    return true;
}

HealSkill::HealSkill()
    : Skill("Hoi Phuc Khan Cap", "Kich hoat phep thuat phuc hoi 30 HP tuc thi [Q]", 8.0f) {}

bool HealSkill::execute(Player* user, GameEngine* engine) {
    if (!user || !canExecute()) return false;

    bool wasPoisoned = user->isPoisoned();
    if (user->getHp() >= user->getMaxHp() && !wasPoisoned) {
        if (engine) {
            engine->getCombatLog().push_back("Mau cua ban da day va khong trung doc!");
        }
        return false;
    }

    int oldHp = user->getHp();
    user->heal(30);
    int healed = user->getHp() - oldHp;

    if (wasPoisoned) {
        user->curePoison();
    }

    if (engine) {
        if (healed > 0) {
            engine->addDamagePopup("+" + std::to_string(healed) + " HP", 
                                   static_cast<float>(user->getPosition().x * 32), 
                                   static_cast<float>(user->getPosition().y * 32 - 10), 
                                   GREEN, 1.0f);
        }
        if (wasPoisoned) {
            engine->addDamagePopup("THANH TAY DOC!", 
                                   static_cast<float>(user->getPosition().x * 32), 
                                   static_cast<float>(user->getPosition().y * 32 - 24), 
                                   Color{ 120, 255, 180, 255 }, 1.2f);
            engine->getCombatLog().push_back("[THANH TAY] Phep Thanh Quang da hoa giai toan bo doc to!");
        }
        engine->getCombatLog().push_back("Ban da su dung [Hoi Phuc Khan Cap] +30 HP!");
    }

    triggerCooldown();
    return true;
}


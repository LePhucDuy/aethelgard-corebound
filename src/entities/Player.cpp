#include "entities/Player.h"
#include "core/Constants.h"
#include <iostream>

Player::Player(const std::string& name, const Position& pos, int hp, int attack, int defense)
    : Entity(name, pos, hp, attack, defense),
      level(1), exp(0), expToNextLevel(50), gold(0),
      inventory(std::make_unique<Inventory>(Constants::MAX_INVENTORY_SLOTS)),
      currentState("idle"), runTimer(0.0f), facingRight(true) {}

void Player::checkLevelUp() {
    while (exp >= expToNextLevel) {
        exp -= expToNextLevel;
        level++;
        expToNextLevel = static_cast<int>(expToNextLevel * 1.5f);

        // Gia tăng chỉ số khi thăng cấp
        maxHp += 20;
        hp = maxHp; // Hồi đầy máu khi lên cấp
        attack += 5;
        defense += 2;

        std::cout << "\n==============================================" << std::endl;
        std::cout << " [CHUC MUNG!] " << name << " da len Cap " << level << "!" << std::endl;
        std::cout << " Chi so tang: MaxHP +20 (" << maxHp << "), ATK +5 (" << attack 
                  << "), DEF +2 (" << defense << ")" << std::endl;
        std::cout << "==============================================\n" << std::endl;
    }
}

void Player::addExp(int amount) {
    if (amount <= 0) return;
    exp += amount;
    std::cout << "[EXP] +" << amount << " kinh nghiem (" << exp << "/" << expToNextLevel << ")" << std::endl;
    checkLevelUp();
}

void Player::addGold(int amount) {
    if (amount <= 0) return;
    gold += amount;
    std::cout << "[Vang] +" << amount << " vang (Tong: " << gold << ")" << std::endl;
}

void Player::addAnimation(const std::string& stateName, std::unique_ptr<Animation> anim) {
    anims[stateName] = std::move(anim);
}

const Animation* Player::getCurrentAnimation() const {
    auto it = anims.find(currentState);
    if (it != anims.end() && it->second) return it->second.get();
    return currentAnim;
}

void Player::setState(const std::string& stateName) {
    if (anims.find(stateName) != anims.end()) {
        currentState = stateName;
        anims[stateName]->reset();
    }
}

void Player::triggerAttack() {
    if (anims.find("attack") != anims.end()) {
        currentState = "attack";
        anims["attack"]->reset();
    }
}

void Player::triggerJump(int dx, int dy) {
    (void)dy;
    if (dx < 0) {
        facingRight = false;
        for (auto& pair : anims) {
            if (pair.second) pair.second->setFacingRight(false);
        }
    } else if (dx > 0) {
        facingRight = true;
        for (auto& pair : anims) {
            if (pair.second) pair.second->setFacingRight(true);
        }
    }

    if (anims.find("jump") != anims.end()) {
        currentState = "jump";
        anims["jump"]->reset();
    }
}

void Player::update(float deltaTime) {
    if (!alive) {
        if (anims.find("dead") != anims.end()) {
            currentState = "dead";
            anims["dead"]->update(deltaTime);
        }
        return;
    }

    // Xử lý chuyển đổi trạng thái hoạt họa
    if (currentState == "attack") {
        auto it = anims.find("attack");
        if (it != anims.end() && it->second) {
            it->second->update(deltaTime);
            if (it->second->hasFinished()) {
                currentState = "idle";
            }
        }
    } else if (currentState == "jump") {
        auto it = anims.find("jump");
        if (it != anims.end() && it->second) {
            it->second->update(deltaTime);
            if (it->second->hasFinished()) {
                currentState = "idle";
            }
        }
    } else if (currentState == "run") {
        auto it = anims.find("run");
        if (it != anims.end() && it->second) {
            it->second->update(deltaTime);
        }
        runTimer -= deltaTime;
        if (runTimer <= 0.0f) {
            currentState = "idle";
        }
    } else {
        // Trạng thái đứng yên (Idle)
        auto it = anims.find("idle");
        if (it != anims.end() && it->second) {
            it->second->update(deltaTime);
        }
    }
}

void Player::render(float scale, Vector2 offset) const {
    // Lấy animation theo trạng thái hiện tại (idle/run/attack/jump/dead)
    auto it = anims.find(currentState);
    const Animation* anim = (it != anims.end()) ? it->second.get() : currentAnim;
    if (!anim) return;

    // CHUẨN HÓA ANCHOR — mọi state neo chung theo chiều cao tham chiếu REF_H = 80
    // (cao nhất họ Warrior): đáy LOGIC = pos.y*TILE + FOOT_SINK(6) + trim*scale.
    // Jump/dead 64px KHÔNG tự neo theo fHeight riêng nữa (đó là lý do nhảy bị lún
    // đúng 16px*scale) mà neo theo refHeight 80px + trim 6px của chính nó, nên
    // bàn chân mọi state trùng khít nhau, hết lún khi nhảy.
    constexpr float REF_FRAME_H = 80.0f;
    constexpr float FOOT_SINK = 6.0f;
    float fWidth = (float)anim->getFrameWidth() * scale;
    float refHeight = REF_FRAME_H * scale;
    float trimBottom = 12.0f; // idle/run/attack 80px
    if (currentState == "jump" || currentState == "dead") trimBottom = 6.0f;

    Vector2 screenPos = {
        (float)(pos.x * Constants::TILE_SIZE) + ((float)Constants::TILE_SIZE - fWidth) / 2.0f + offset.x,
        (float)(pos.y * Constants::TILE_SIZE) + FOOT_SINK - refHeight + trimBottom * scale + offset.y
    };

    anim->draw(screenPos, scale);
}

bool Player::moveBy(int dx, int dy, Dungeon& dungeon) {
    (void)dungeon;
    pos.x += dx;
    pos.y += dy;

    // Cập nhật hướng quay mặt của sprite
    if (dx < 0) {
        facingRight = false;
        for (auto& pair : anims) {
            if (pair.second) pair.second->setFacingRight(false);
        }
    } else if (dx > 0) {
        facingRight = true;
        for (auto& pair : anims) {
            if (pair.second) pair.second->setFacingRight(true);
        }
    }

    // Kích hoạt hoạt họa chạy
    currentState = "run";
    runTimer = 0.28f;
    return true;
}

void Player::act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) {
    // Player được điều khiển trực tiếp qua handleInput() của GameEngine,
    // nên phương thức act() theo lượt của Entity là không dùng (giữ rỗng).
    (void)dungeon; (void)player; (void)combatLog;
}

void Player::resetStats(const Position& startPos) {
    pos = startPos;
    hp = 100;
    maxHp = 100;
    attack = 16;
    defense = 5;
    level = 1;
    exp = 0;
    expToNextLevel = 50;
    gold = 0;
    alive = true;
    currentState = "idle";
    runTimer = 0.0f;
    facingRight = true;
}

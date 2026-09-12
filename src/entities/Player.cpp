#include "entities/Player.h"
#include "core/Constants.h"
#include <iostream>
#include <cmath>

Player::Player(const std::string& name, const Position& pos, int hp, int attack, int defense)
    : Entity(name, pos, hp, attack, defense),
      level(1), exp(0), expToNextLevel(50), gold(0),
      inventory(std::make_unique<Inventory>(Constants::MAX_INVENTORY_SLOTS)),
      currentState("idle"), runTimer(0.0f), jumpTimer(0.0f), jumpVisualLift(0.0f),
      sinkVisualOffset(0.0f),
      facingRight(true) {}

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

void Player::setFacingRight(bool right) {
    facingRight = right;
    for (auto& pair : anims) {
        if (pair.second) pair.second->setFacingRight(right);
    }
}

void Player::triggerJump(int dx, int dy) {
    (void)dy;
    if (dx < 0) {
        setFacingRight(false);
    } else if (dx > 0) {
        setFacingRight(true);
    }

    if (anims.find("jump") != anims.end()) {
        currentState = "jump";
        anims["jump"]->reset();
        // Khóa hoạt họa nhảy ~0.45s để không bị run đè ngay khi giữ phím ngang.
        // jumpVisualLift là độ nâng CỰC ĐẠI của parabol; render() nội suy theo
        // jumpTimer để bay lên rồi đáp mượt (giữa cú nhảy nâng cao nhất).
        jumpTimer = 0.45f;
        jumpVisualLift = 34.0f; // ~1 ô: đủ thấy bay lên rồi đáp xuống cỏ
    }
}

void Player::update(float deltaTime) {
    Entity::update(deltaTime);

    if (!alive) {
        if (anims.find("dead") != anims.end()) {
            currentState = "dead";
            anims["dead"]->update(deltaTime);
        }
        return;
    }

    // Xử lý chuyển đổi trạng thái hoạt họa — jump được ƯU TIÊN: trong jumpTimer
    // thì run/moveBy không được đè state (fix giữ phím ngang + Space vẫn hiện run).
    // Jump-All 15 frame loop liên tục trong suốt cú nhảy nên luôn thấy bay.
    if (jumpTimer > 0.0f) {
        jumpTimer -= deltaTime;
        auto it = anims.find("jump");
        if (it != anims.end() && it->second) {
            it->second->update(deltaTime);
        }
        currentState = "jump";
        if (jumpTimer <= 0.0f) {
            jumpVisualLift = 0.0f;
            currentState = "idle";
        }
        return;
    }
    jumpVisualLift = 0.0f;
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

    // Parabol chuẩn: progress 0->1, lift = max * sin(pi*progress):
    // đầu cú nhảy nâng 0 -> giữa nâng cực đại 34px -> cuối về 0 (đáp đúng cỏ).
    float jumpLift = 0.0f;
    if (jumpTimer > 0.0f) {
        float progress = 1.0f - (jumpTimer / 0.45f);
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
        jumpLift = jumpVisualLift * std::sin(progress * 3.14159265f);
    }
    Vector2 screenPos = {
        visualPos.x + ((float)Constants::TILE_SIZE - fWidth) / 2.0f + offset.x,
        visualPos.y + FOOT_SINK - refHeight + trimBottom * scale + offset.y
            - jumpLift + sinkVisualOffset
    };

    anim->draw(screenPos, scale);
}

bool Player::moveBy(int dx, int dy, Dungeon& dungeon) {
    (void)dungeon;
    pos.x += dx;
    pos.y += dy;

    // Cập nhật hướng quay mặt của sprite
    if (dx < 0) {
        setFacingRight(false);
    } else if (dx > 0) {
        setFacingRight(true);
    }

    // Kích hoạt hoạt họa chạy — đồng bộ với nhịp input 0.07s để run loop mượt,
    // không bị ngắt quãng về idle giữa các bước khi giữ phím (fix chạy khựng).
    // Đang trong cú nhảy (jumpTimer) thì GIỮ nguyên state jump — không cho run đè
    // (fix giữ phím ngang + Space vẫn hiện hoạt họa run).
    if (jumpTimer > 0.0f) return true;

    currentState = "run";
    runTimer = 0.12f;
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
    jumpTimer = 0.0f;
    jumpVisualLift = 0.0f;
    sinkVisualOffset = 0.0f;
    facingRight = true;
}

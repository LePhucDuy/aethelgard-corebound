#include "entities/Player.h"
#include "systems/EventSystem.h"
#include "core/Constants.h"
#include "core/Templates.h"
#include <iostream>
#include <cmath>
#include <algorithm>

Player::Player(const std::string& name, const Position& pos, int hp, int attack, int defense)
    : Entity(name, pos, hp, attack, defense),
      level(1), exp(0), expToNextLevel(50), gold(0), forgeLevel(0),
      inventory(std::make_unique<Inventory>(Constants::MAX_INVENTORY_SLOTS)),
      currentState("idle"), runTimer(0.0f), jumpTimer(0.0f), jumpVisualLift(0.0f),
      sinkVisualOffset(0.0f),
      fallTimer(0.0f), fallAirDuration(0.0f), fallTotalDuration(0.0f),
      startFallY(0.0f), targetFallY(0.0f),
      facingRight(true),
      blockTimer(0.0f), parryWindowTimer(0.0f), blockCooldown(0.0f),
      poisonTimer(0.0f), poisonTickTimer(0.0f), poisonDmgPerTick(0) {
    // Khởi tạo các kỹ năng đa hình
    skills.push_back(std::make_unique<SlashSkill>());
    skills.push_back(std::make_unique<DashSkill>());
    skills.push_back(std::make_unique<HealSkill>());
}

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
    EventDispatcher::getInstance().notify(GameEvent(GameEventType::GOLD_GAINED, amount, "+" + std::to_string(amount) + " vang", this));
}

bool Player::spendGold(int amount) {
    if (amount <= 0) return true;
    if (gold < amount) return false;
    gold -= amount;
    std::cout << "[Vang] Da tieu ton -" << amount << " vang (Con lai: " << gold << ")" << std::endl;
    return true;
}

bool Player::upgradeForge() {
    int cost = getNextUpgradeCost();
    if (!spendGold(cost)) {
        std::cout << "[De ren] Khong du " << cost << " vang de cuong hoa vu khi!" << std::endl;
        return false;
    }

    int bonus = getNextUpgradeBonus();
    forgeLevel++;
    attack += bonus;
    std::cout << "[De ren] Cuong hoa thanh cong! Vu khi len Cap +" << forgeLevel 
              << ", tang +" << bonus << " ATK! (Tong ATK: " << attack << ")" << std::endl;
    EventDispatcher::getInstance().notify(GameEvent(GameEventType::FORGE_UPGRADED, forgeLevel, "Upgrade Level " + std::to_string(forgeLevel), this));
    return true;
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
    if (fallTimer > 0.0f) return; // Không chém khi đang rơi tự do
    if (anims.find("attack") != anims.end()) {
        currentState = "attack";
        anims["attack"]->reset();
    }
}

void Player::triggerBlock() {
    if (fallTimer > 0.0f || !alive) return;
    if (blockCooldown > 0.0f) return;

    blockTimer = 0.45f;
    parryWindowTimer = 0.18f;
    blockCooldown = 0.70f;
}

void Player::applyPoison(float duration, int dmgPerTick) {
    poisonTimer = CoreTemplates::getMaxValue(poisonTimer, duration);
    poisonDmgPerTick = CoreTemplates::getMaxValue(poisonDmgPerTick, dmgPerTick);
    poisonTickTimer = 0.0f;
}

void Player::curePoison() {
    poisonTimer = 0.0f;
    poisonTickTimer = 0.0f;
    poisonDmgPerTick = 0;
}

void Player::setFacingRight(bool right) {
    facingRight = right;
    for (auto& pair : anims) {
        if (pair.second) pair.second->setFacingRight(right);
    }
}

void Player::triggerJump(int dx, int dy) {
    if (fallTimer > 0.0f) return; // Đang rơi xuống hố không được nhảy tiếp
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

void Player::setPosition(const Position& newPos, bool snapVisual) {
    int dy = newPos.y - pos.y;
    Entity::setPosition(newPos, snapVisual);

    // Bất kỳ khi nào người chơi rơi từ trên cao xuống sàn dưới (dy >= 2 ô):
    // Tự động kích hoạt hiệu ứng rơi tự do Jump-End-Sheet với tốc độ chậm chuẩn vật lý
    if (!snapVisual && dy >= 2 && alive) {
        float airDuration = CoreTemplates::clampValue(0.22f * std::sqrt((float)dy), 0.35f, 0.65f);
        float landDuration = 0.22f;
        triggerFall(airDuration, landDuration);
    }
}

void Player::triggerFall(float airDuration, float landDuration) {
    fallAirDuration = airDuration;
    fallTotalDuration = airDuration + landDuration;
    fallTimer = fallTotalDuration;
    startFallY = visualPos.y;
    targetFallY = (float)(pos.y * Constants::TILE_SIZE);
    currentState = "fall";
    jumpTimer = 0.0f;
    jumpVisualLift = 0.0f;
    runTimer = 0.0f;

    auto it = anims.find("fall");
    if (it != anims.end() && it->second) {
        it->second->reset();
        it->second->setCurrentFrame(0);
    }
}

void Player::update(float deltaTime) {
    Entity::update(deltaTime);

    // Cập nhật hồi chiêu các kỹ năng
    for (auto& s : skills) {
        if (s) s->update(deltaTime);
    }

    // Cập nhật thời gian Đỡ đòn & Phản đòn
    if (blockTimer > 0.0f) {
        blockTimer -= deltaTime;
        if (blockTimer < 0.0f) blockTimer = 0.0f;
    }
    if (parryWindowTimer > 0.0f) {
        parryWindowTimer -= deltaTime;
        if (parryWindowTimer < 0.0f) parryWindowTimer = 0.0f;
    }
    if (blockCooldown > 0.0f) {
        blockCooldown -= deltaTime;
        if (blockCooldown < 0.0f) blockCooldown = 0.0f;
    }

    // Cập nhật Trúng Độc DoT (rút máu độc lập mỗi giây)
    if (alive && poisonTimer > 0.0f) {
        poisonTimer -= deltaTime;
        poisonTickTimer += deltaTime;
        if (poisonTickTimer >= 1.0f) {
            poisonTickTimer -= 1.0f;
            hp -= poisonDmgPerTick;
            if (hp <= 0) {
                hp = 0;
                alive = false;
            }
        }
        if (poisonTimer <= 0.0f) {
            curePoison();
        }
    }

    if (!alive) {
        if (anims.find("dead") != anims.end()) {
            if (currentState != "dead") {
                currentState = "dead";
                anims["dead"]->reset();
            }
            anims["dead"]->update(deltaTime);
        }
        return;
    }

    // Xử lý hoạt ảnh rơi tự do xuống hố (Jump-End-Sheet) với tốc độ vật lý chân thực ngoài đời:
    // Giai đoạn 1: Rơi trong không trung (airDuration ~0.52s): gia tốc trọng trường s = 0.5 * g * t^2,
    // Frame 0 duỗi thẳng chân lao xuống nhanh dần.
    // Giai đoạn 2: Tiếp đất giảm chấn (landDuration ~0.22s):
    // Frame 1 chùng gối giảm chấn -> Frame 2 gập sâu gối thu kiếm kiên định -> về idle.
    if (fallTimer > 0.0f) {
        fallTimer -= deltaTime;
        currentState = "fall";

        float elapsed = fallTotalDuration - fallTimer;
        auto it = anims.find("fall");

        if (elapsed < fallAirDuration) {
            float tNorm = elapsed / fallAirDuration;
            float progress = tNorm * tNorm; // Parabol gia tốc trọng trường rơi nhanh dần
            visualPos.y = startFallY + (targetFallY - startFallY) * progress;

            if (it != anims.end() && it->second) {
                it->second->setCurrentFrame(0); // Frame 0: Lơ lửng trên không, duỗi thẳng chân
            }
        } else {
            visualPos.y = targetFallY; // Đã chạm sàn tầng dưới

            float landElapsed = elapsed - fallAirDuration;
            float landTotal = fallTotalDuration - fallAirDuration;
            float landNorm = (landTotal > 0.0f) ? (landElapsed / landTotal) : 1.0f;

            if (it != anims.end() && it->second) {
                if (landNorm < 0.45f) {
                    it->second->setCurrentFrame(1); // Frame 1: Chạm đất, chùng gối giảm chấn
                } else {
                    it->second->setCurrentFrame(2); // Frame 2: Gập sâu gối, thu kiếm thủ thế kiên định
                }
            }
        }

        // Tự động căn chỉnh nhẹ nhàng trục X vào tâm ô đích
        float targetX = (float)(pos.x * Constants::TILE_SIZE);
        float tx = 1.0f - std::exp(-12.0f * deltaTime);
        visualPos.x += (targetX - visualPos.x) * tx;
        if (std::abs(visualPos.x - targetX) < 0.25f) visualPos.x = targetX;

        if (fallTimer <= 0.0f) {
            fallTimer = 0.0f;
            visualPos.y = targetFallY;
            currentState = "idle";
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
    constexpr float REF_FRAME_H = 80.0f;
    constexpr float FOOT_SINK = 6.0f;
    float fWidth = (float)anim->getFrameWidth() * scale;
    float refHeight = REF_FRAME_H * scale;
    float trimBottom = 12.0f; // idle/run/attack 80px
    if (currentState == "jump") {
        trimBottom = 6.0f;
    } else if (currentState == "dead") {
        trimBottom = 27.0f; // Dead sheet 64px: khớp đáy bàn chân F0 và xác nằm F7 sát mặt đất
    } else if (currentState == "fall") {
        trimBottom = 12.0f; // Jump-End sheet 64px: khớp chuẩn xác 100% từng pixel với Idle
    }

    // Parabol chuẩn: progress 0->1, lift = max * sin(pi*progress):
    // đầu cú nhảy nâng 0 -> giữa nâng cực đại 34px -> cuối về 0 (đáp đúng cỏ).
    float jumpLift = 0.0f;
    if (jumpTimer > 0.0f) {
        float progress = 1.0f - (jumpTimer / 0.45f);
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
        jumpLift = jumpVisualLift * std::sin(progress * 3.14159265f);
    }

    // Căn chỉnh trục X khi chết hoặc tiếp đất để bàn chân không bị dịch chuyển đột ngột giữa Idle và Fall/Dead
    float stateShiftX = 0.0f;
    if (currentState == "dead") {
        stateShiftX = facingRight ? (16.5f * scale) : (-16.5f * scale);
    } else if (currentState == "fall") {
        stateShiftX = facingRight ? (9.0f * scale) : (-9.0f * scale);
    }

    Vector2 screenPos = {
        visualPos.x + ((float)Constants::TILE_SIZE - fWidth) / 2.0f + stateShiftX + offset.x,
        visualPos.y + FOOT_SINK - refHeight + trimBottom * scale + offset.y
            - jumpLift + sinkVisualOffset
    };

    anim->draw(screenPos, scale);

    // Vẽ hào quang khiên chắn khi đang trong thế Đỡ đòn hoặc Phản đòn [K]
    if (isBlocking() && alive) {
        float cx = visualPos.x + Constants::TILE_SIZE / 2.0f + (facingRight ? 18.0f : -18.0f) + offset.x;
        float cy = visualPos.y + Constants::TILE_SIZE / 2.0f - 4.0f + offset.y - jumpLift + sinkVisualOffset;

        if (isParrying()) {
            // Hiệu ứng Perfect Parry: Hào quang hoàng kim rực sáng, tia sét kim loại
            float pulse = 1.0f + 0.15f * std::sin((float)GetTime() * 30.0f);
            DrawCircleGradient(Vector2{ cx, cy }, 22.0f * pulse, Color{ 255, 230, 100, 170 }, Color{ 255, 180, 20, 0 });
            DrawCircleLines((int)cx, (int)cy, 18.0f * pulse, GOLD);
            DrawCircleLines((int)cx, (int)cy, 13.0f, Color{ 255, 255, 255, 230 });
            // Tia phản đòn chữ thập sáng chói
            DrawLine((int)(cx - 16), (int)cy, (int)(cx + 16), (int)cy, Color{ 255, 240, 140, 255 });
            DrawLine((int)cx, (int)(cy - 16), (int)cx, (int)(cy + 16), Color{ 255, 240, 140, 255 });
        } else {
            // Hiệu ứng Block thông thường: Khiên năng lượng lam ngọc tinh thể vững chãi
            DrawCircleGradient(Vector2{ cx, cy }, 18.0f, Color{ 80, 180, 255, 120 }, Color{ 20, 90, 200, 0 });
            DrawCircleLines((int)cx, (int)cy, 16.0f, Color{ 130, 215, 255, 240 });
            Vector2 p1 = { cx, cy - 10 };
            Vector2 p2 = { cx + 8, cy - 3 };
            Vector2 p3 = { cx, cy + 10 };
            Vector2 p4 = { cx - 8, cy - 3 };
            DrawLineEx(p1, p2, 2.0f, Color{ 170, 235, 255, 240 });
            DrawLineEx(p2, p3, 2.0f, Color{ 170, 235, 255, 240 });
            DrawLineEx(p3, p4, 2.0f, Color{ 170, 235, 255, 240 });
            DrawLineEx(p4, p1, 2.0f, Color{ 170, 235, 255, 240 });
        }
    }
}

bool Player::moveBy(int dx, int dy, Dungeon& dungeon) {
    if (fallTimer > 0.0f) return false; // Không di chuyển ngang khi đang rơi xuống hố
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
    forgeLevel = 0;
    alive = true;
    currentState = "idle";
    runTimer = 0.0f;
    jumpTimer = 0.0f;
    jumpVisualLift = 0.0f;
    sinkVisualOffset = 0.0f;
    fallTimer = 0.0f;
    fallAirDuration = 0.0f;
    fallTotalDuration = 0.0f;
    startFallY = 0.0f;
    targetFallY = 0.0f;
    facingRight = true;
    blockTimer = 0.0f;
    parryWindowTimer = 0.0f;
    blockCooldown = 0.0f;
    poisonTimer = 0.0f;
    poisonTickTimer = 0.0f;
    poisonDmgPerTick = 0;
    for (auto& pair : anims) {
        if (pair.second) pair.second->reset();
    }
}

bool Player::useSkill(size_t index, GameEngine* engine) {
    if (index >= skills.size() || !skills[index]) return false;
    return skills[index]->execute(this, engine);
}

Skill* Player::getSkill(size_t index) const {
    if (index >= skills.size()) return nullptr;
    return skills[index].get();
}


#include "items/Chest.h"
#include "entities/Player.h"
#include "engine/GameEngine.h"
#include "graphics/TextureManager.h"
#include "items/Potion.h"
#include "items/Weapon.h"
#include "systems/EventSystem.h"
#include "core/Constants.h"
#include <iostream>
#include <cmath>

Chest::Chest(const Position& pos, int cost, const std::string& reward, int exp)
    : IRenderable(), pos(pos), opened(false), unlockCost(cost), rewardName(reward), expReward(exp) {}

void Chest::render(float scale, Vector2 offset) const {
    TextureManager& tm = TextureManager::getInstance();
    const std::string texId = opened ? "item_chest_gold_open" : "item_chest_gold_closed";

    float worldX = (float)(pos.x * Constants::TILE_SIZE) + offset.x;
    float worldY = (float)(pos.y * Constants::TILE_SIZE) + offset.y;
    constexpr float CHEST_SIZE = 36.0f;

    float drawX = worldX + ((float)Constants::TILE_SIZE - CHEST_SIZE) / 2.0f;
    float drawY = worldY + (float)Constants::TILE_SIZE - CHEST_SIZE;

    // Hiệu ứng ánh sáng hào quang vàng xung quanh rương
    if (!opened) {
        float glowAlpha = 0.45f + 0.25f * std::sin((float)GetTime() * 4.0f);
        DrawCircleGradient(Vector2{ drawX + CHEST_SIZE / 2.0f, drawY + CHEST_SIZE / 2.0f }, 
                           24.0f, ColorAlpha(GOLD, glowAlpha), ColorAlpha(YELLOW, 0.0f));
    } else {
        float openGlow = 0.5f + 0.3f * std::sin((float)GetTime() * 6.0f);
        DrawCircleGradient(Vector2{ drawX + CHEST_SIZE / 2.0f, drawY + CHEST_SIZE / 2.0f - 4.0f }, 
                           30.0f, ColorAlpha(YELLOW, openGlow), ColorAlpha(ORANGE, 0.0f));
    }

    if (tm.has(texId)) {
        const Texture2D& tex = tm.get(texId);
        Rectangle srcRec = { 0, 0, (float)tex.width, (float)tex.height };
        Rectangle destRec = { drawX, drawY, CHEST_SIZE, CHEST_SIZE };
        DrawTexturePro(tex, srcRec, destRec, Vector2{ 0, 0 }, 0.0f, WHITE);
    } else {
        // Fallback vẽ khối rương bằng Raylib
        Color cColor = opened ? Color{ 160, 120, 40, 255 } : Color{ 120, 80, 20, 255 };
        DrawRectangle((int)drawX, (int)drawY, (int)CHEST_SIZE, (int)CHEST_SIZE, cColor);
        DrawRectangleLines((int)drawX, (int)drawY, (int)CHEST_SIZE, (int)CHEST_SIZE, GOLD);
    }
}

bool Chest::tryOpen(Player& player, GameEngine* engine) {
    if (opened) return false;

    if (player.getInventory().isFull()) {
        std::cout << "[Ruong Bau] Tui do da day! Khong the mo Ruong Hoang Kim." << std::endl;
        return false;
    }

    if (!player.spendGold(unlockCost)) {
        std::cout << "[Ruong Bau] Khong du " << unlockCost << " vang de mo Ruong Hoang Kim!" << std::endl;
        return false;
    }

    opened = true;
    player.addExp(expReward);

    // Trao thưởng vật phẩm quý giá
    if (rewardName.find("Kiem") != std::string::npos || rewardName.find("Vu Khi") != std::string::npos) {
        player.getInventory().addItem(std::make_unique<Weapon>(rewardName, "Vu khi truyen thuyet mo tu Ruong Hoang Kim", 15, Position(0, 0), "item_sword_mystic"));
    } else {
        player.getInventory().addItem(std::make_unique<Potion>(rewardName, "Duoc pham quy hiem mo tu Ruong Hoang Kim", 100, Position(0, 0), "item_potion_elixir"));
    }

    std::cout << "[Ruong Bau] Da mo khoa Ruong Hoang Kim! Nhan +" << expReward 
              << " EXP va vat pham: " << rewardName << "!" << std::endl;
    EventDispatcher::getInstance().notify(GameEvent(GameEventType::CHEST_OPENED, expReward, rewardName, &player));
    return true;
}


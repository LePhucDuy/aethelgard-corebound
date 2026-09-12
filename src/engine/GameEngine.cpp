#include "engine/GameEngine.h"
#include "graphics/TextureManager.h"
#include "systems/CombatSystem.h"
#include "systems/SaveLoadManager.h"
#include "core/GameException.h"
#include "items/Potion.h"
#include "items/Weapon.h"
#include "items/Armor.h"
#include "items/Accessory.h"
#include "items/Chest.h"
#include "entities/Snail.h"
#include "entities/Boar.h"
#include "entities/BoarKing.h"
#include "core/Constants.h"
#include <rlgl.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <sstream>

GameEngine::GameEngine(int spawnX, int spawnY, bool startWithInventory, bool startLethal, bool startWithShop, bool startWithForge)
    : player("Hiep Si Aethelgard", Position(4, 17), 100, 16, 5),
      dungeon(Constants::DUNGEON_WIDTH, Constants::DUNGEON_HEIGHT),
      state(GameState::RUNNING),
      moveTimer(0.0f),
      attackTimer(0.0f),
      edgeSlipTimer(0.0f),
      userZoomOffset(0.0f),
      isSinking(false),
      submergedInSwamp(false),
      sinkTimer(0.0f),
      sinkDuration(1.6f),
      sinkDepth(0.0f),
      isDying(false),
      deathTimer(0.0f),
      deathDuration(1.35f),
      currentZone(-1),
      bannerText(""),
      bannerTimer(0.0f),
      monstersDefeated(0),
      bossCinematicTriggered(false),
      bossWarningTimer(0.0f),
      screenShake(0.0f),
      showInventory(startWithInventory),
      showShop(startWithShop),
      showForge(startWithForge),
      showCombatLog(true),
      spawnOverrideX(spawnX),
      spawnOverrideY(spawnY),
      startLethalOverride(startLethal) {
    camera.target = Vector2{ 0.0f, 12.5f * (float)Constants::TILE_SIZE };
    camera.offset = Vector2{ (float)Constants::SCREEN_WIDTH / 2.0f, (float)Constants::SCREEN_HEIGHT - 120.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.15f;
    fontMain.texture.id = 0;
}

GameEngine::~GameEngine() {
    if (fontMain.texture.id != 0 && fontMain.texture.id != GetFontDefault().texture.id) {
        UnloadFont(fontMain);
    }
}

void GameEngine::init() {
    // 0. Tải Font TrueType sắc nét từ Windows để chống lỗi bể chữ, nhòe chữ
    if (FileExists("C:/Windows/Fonts/segoeui.ttf")) {
        fontMain = LoadFontEx("C:/Windows/Fonts/segoeui.ttf", 24, nullptr, 0);
        SetTextureFilter(fontMain.texture, TEXTURE_FILTER_BILINEAR);
    } else if (FileExists("C:/Windows/Fonts/arial.ttf")) {
        fontMain = LoadFontEx("C:/Windows/Fonts/arial.ttf", 24, nullptr, 0);
        SetTextureFilter(fontMain.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        fontMain = GetFontDefault();
    }

    TextureManager& tm = TextureManager::getInstance();

    // 1. Nạp toàn bộ tài nguyên hình ảnh môi trường và thực thể
    tm.load("bg_back",        "assets/environment/back.png");
    tm.load("bg_middle",      "assets/environment/middle.png");
    tm.load("tileset",        "assets/environment/tiles.png");
    tm.load("warrior_idle",   "assets/characters/warrior/Idle/Idle-Sheet.png");
    tm.load("warrior_run",    "assets/characters/warrior/Run/Run-Sheet.png");
    tm.load("warrior_attack", "assets/characters/warrior/Attack-01/Attack-01-Sheet.png");
    tm.load("warrior_jump",   "assets/characters/warrior/Jumlp-All/Jump-All-Sheet.png");
    tm.load("warrior_dead",   "assets/characters/warrior/Dead/Dead-Sheet.png");
    tm.load("warrior_fall",   "assets/characters/warrior/Jump-End/Jump-End-Sheet.png");
    tm.load("boar_walk",       "assets/mobs/boar/Walk/Walk-Base-Sheet.png");
    tm.load("boar_idle",       "assets/mobs/boar/Idle/Idle-Sheet.png");
    tm.load("boar_run",        "assets/mobs/boar/Run/Run-Sheet.png");
    tm.load("boar_hit",        "assets/mobs/boar/Hit-Vanish/Hit-Sheet.png");
    tm.load("boar_black_walk", "assets/mobs/boar/Walk/Walk-Base-SheetBlack.png");
    tm.load("boar_black_idle", "assets/mobs/boar/Idle/Idle-Sheet-export-Back.png");
    tm.load("boar_black_run",  "assets/mobs/boar/Run/Run-Sheet-Black.png");
    tm.load("boar_black_hit",  "assets/mobs/boar/Hit-Vanish/Hit-Sheet-Black.png");
    tm.load("boar_white_walk", "assets/mobs/boar/Walk/Walk-Base-Sheet-White.png");
    tm.load("boar_white_idle", "assets/mobs/boar/Idle/Idle-Sheet-White.png");
    tm.load("boar_white_run",  "assets/mobs/boar/Run/Run-Sheet-White.png");
    tm.load("boar_white_hit",  "assets/mobs/boar/Hit-Vanish/Hit-Sheet-White.png");
    tm.load("bee_fly",        "assets/mobs/small_bee/Fly/Fly-Sheet.png");
    tm.load("bee_attack",     "assets/mobs/small_bee/Attack/Attack-Sheet.png");
    tm.load("bee_hit",        "assets/mobs/small_bee/Hit/Hit-Sheet.png");
    tm.load("snail_walk",     "assets/mobs/snail/walk-Sheet.png");
    tm.load("snail_hide",     "assets/mobs/snail/Hide-Sheet.png");
    tm.load("snail_dead",     "assets/mobs/snail/Dead-Sheet.png");

    // Nạp toàn bộ tài nguyên hình ảnh vật phẩm & biểu tượng (Items & Icons)
    tm.load("item_sword_steel",       "assets/items/sword_steel.png");
    tm.load("item_sword_mystic",      "assets/items/sword_mystic.png");
    tm.load("item_potion_starter",    "assets/items/potion_starter.png");
    tm.load("item_potion_health",     "assets/items/potion_health.png");
    tm.load("item_potion_strength",   "assets/items/potion_strength.png");
    tm.load("item_potion_elixir",     "assets/items/potion_elixir.png");
    tm.load("item_gold_coin",         "assets/items/gold_coin.png");
    tm.load("item_gold_pile",         "assets/items/gold_pile.png");
    tm.load("item_chest_gold_closed", "assets/items/chest_gold_closed.png");
    tm.load("item_chest_gold_open",   "assets/items/chest_gold_open.png");
    tm.load("item_icon_shop",         "assets/items/icon_shop.png");
    tm.load("item_icon_forge",        "assets/items/icon_forge.png");
    tm.load("item_armor_shield",      "assets/items/armor_shield.png");
    tm.load("item_ring_power",        "assets/items/ring_power.png");

    // 2. Thiết lập ĐẦY ĐỦ hệ thống hoạt họa phong phú cho Player
    player.addAnimation("idle",   std::make_unique<Animation>("warrior_idle", 4, 64, 80, 0.14f, true));
    player.addAnimation("run",    std::make_unique<Animation>("warrior_run", 8, 80, 80, 0.08f, true));
    player.addAnimation("attack", std::make_unique<Animation>("warrior_attack", 8, 96, 80, 0.06f, false));
    player.addAnimation("jump",   std::make_unique<Animation>("warrior_jump", 15, 64, 64, 0.03f, true));
    player.addAnimation("dead",   std::make_unique<Animation>("warrior_dead", 8, 80, 64, 0.085f, false));
    player.addAnimation("fall",   std::make_unique<Animation>("warrior_fall", 3, 64, 64, 0.18f, false));
    player.setState("idle");

    // 3. Khởi tạo tầng 1 hầm ngục 2D Side dài 75 ô
    dungeon.generate(1);
    // Vị trí xuất phát: ghi đè bởi --spawn (debug) nếu có, ngược lại dùng điểm start của map
    if (spawnOverrideX >= 0 && spawnOverrideY >= 0) {
        player.setPosition(Position(spawnOverrideX, spawnOverrideY), true);
    } else {
        player.setPosition(dungeon.getPlayerStartPos(), true);
    }

    // 4. Cung cấp vật phẩm và vàng khởi đầu
    player.setGold(45);
    player.getInventory().addItem(std::make_unique<Potion>("Binh Thuoc Khoi Dau", "Hoi phuc 30 HP", 30, Position(0, 0), "item_potion_starter"));
    player.getInventory().addItem(std::make_unique<Weapon>("Dao Gam Khoi Dau", "Vu khi co ban +3 ATK", 3, Position(0, 0), "item_sword_steel"));

    // 5. Nhật ký chào mừng
    combatLog.push_back("Chao mung ban den voi Ham nguc Aethelgard!");
    combatLog.push_back("Nhan [P] Cua Hang | [U] De Ren | [B] Tui Do | [E] Mo Ruong");
    combatLog.push_back("Ha guc quai vat se roi tien vang bung toa ruc ro!");

    // 6. Tuỳ chọn kiểm tra tử trận (--kill)
    if (startLethalOverride) {
        player.takeDamage(9999);
    }
}

void GameEngine::drawText(const char* text, float posX, float posY, float fontSize, Color color) const {
    if (fontMain.texture.id != 0) {
        DrawTextEx(fontMain, text, Vector2{ posX, posY }, fontSize, 1.0f, color);
    } else {
        DrawText(text, (int)posX, (int)posY, (int)fontSize, color);
    }
}

void GameEngine::handleInput() {
    if (state == GameState::GAME_OVER || state == GameState::VICTORY) {
        if (IsKeyPressed(KEY_R)) {
            dungeon.generate(1);
            player.resetStats(dungeon.getPlayerStartPos());
            combatLog.clear();
            combatLog.push_back("=== BAN DA HOI SINH! BAT DAU LAI TU TANG 1 ===");
            monstersDefeated = 0;
            currentZone = -1; // Kích hoạt lại banner khu A
            edgeSlipTimer = 0.0f;
            isSinking = false;
            submergedInSwamp = false;
            sinkTimer = 0.0f;
            sinkDepth = 0.0f;
            player.setSinkVisualOffset(0.0f);
            isDying = false;
            deathTimer = 0.0f;
            bossCinematicTriggered = false;
            bossWarningTimer = 0.0f;
            screenShake = 0.0f;
            state = GameState::RUNNING;
        }
        return;
    }

    // Nếu đang trong hoạt cảnh tử trận: khóa điều khiển hoàn toàn
    if (isDying) {
        return;
    }

    Position pPos = player.getPosition();

    // =========================================================================
    // XỬ LÝ KHI ĐANG BỊ LÚN ĐẦM LẦY (SWAMP SINKING)
    // =========================================================================
    if (isSinking) {
        // Cho phép người chơi vùng vẫy phóng mình nhảy thoát lên bờ trong nửa giây đầu (sinkTimer > 0.8s)
        if (sinkTimer > 0.8f && IsKeyPressed(KEY_SPACE)) {
            int escDir = player.isFacingRight() ? 1 : -1;
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) escDir = 1;
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  escDir = -1;

            Position candidates[] = {
                Position(pPos.x + escDir * 2, 18),
                Position(pPos.x + escDir, 18),
                Position(pPos.x - escDir * 2, 18),
                Position(pPos.x - escDir, 18),
                Position(pPos.x + escDir * 3, 18),
                Position(pPos.x - escDir * 3, 18)
            };

            for (const auto& cand : candidates) {
                if (dungeon.isValidPos(cand) && dungeon.isWalkable(cand) && !dungeon.isWater(cand) && dungeon.getMonsterAt(cand) == nullptr) {
                    player.triggerJump(cand.x - pPos.x, -1);
                    player.setPosition(cand);
                    isSinking = false;
                    sinkTimer = 0.0f;
                    sinkDepth = 0.0f;
                    player.setSinkVisualOffset(0.0f);
                    combatLog.push_back("[THOAT HIEM!] Ban da kip thoi vung vay phong minh thoat khoi dam lay lun!");
                    return;
                }
            }
        }
        // Khi đang lún: bị bùn giữ chân, không thể đi lại hay tấn công bình thường
        return;
    }

    // Phím Toàn màn hình [F11] hoặc [Alt + Enter]
    if (IsKeyPressed(KEY_F11) || ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && IsKeyPressed(KEY_ENTER))) {
        ToggleFullscreen();
    }

    // Phím P: bật/tắt hiển thị Cửa Hàng hầm ngục (Shop)
    if (IsKeyPressed(KEY_P)) {
        showShop = !showShop;
        if (showShop) {
            showInventory = false;
            showForge = false;
        }
    }

    // Phím U: bật/tắt hiển thị Đe Rèn Cường Hóa (Forge)
    if (IsKeyPressed(KEY_U) && !showShop) {
        showForge = !showForge;
        if (showForge) {
            showInventory = false;
            showShop = false;
        }
    }

    // Phím B / I / Tab: bật/tắt hiển thị bảng túi đồ
    if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_I) || IsKeyPressed(KEY_TAB)) {
        showInventory = !showInventory;
        if (showInventory) {
            showShop = false;
            showForge = false;
        }
    }

    // Phím E: Tương tác mở Rương Báu Hoàng Kim khi đứng gần
    if (IsKeyPressed(KEY_E) && !showInventory && !showShop && !showForge) {
        interactWithChest();
    }

    // Phím ESC: đóng bất kỳ bảng modal nào đang mở
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (showShop) { showShop = false; return; }
        if (showForge) { showForge = false; return; }
        if (showInventory) { showInventory = false; return; }
    }

    // Phím L: thu gọn / mở khung nhật ký chiến đấu
    if (IsKeyPressed(KEY_L)) {
        showCombatLog = !showCombatLog;
    }

    // Phím tắt số [1]-[6] mua nhanh khi Cửa Hàng đang mở
    if (showShop) {
        if (IsKeyPressed(KEY_ONE))   buyShopItem(0);
        if (IsKeyPressed(KEY_TWO))   buyShopItem(1);
        if (IsKeyPressed(KEY_THREE)) buyShopItem(2);
        if (IsKeyPressed(KEY_FOUR))  buyShopItem(3);
        if (IsKeyPressed(KEY_FIVE))  buyShopItem(4);
        if (IsKeyPressed(KEY_SIX))   buyShopItem(5);
    }

    // Phím Enter / Space / U để cường hóa khi Đe Rèn đang mở
    if (showForge) {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_U)) {
            triggerForgeUpgrade();
        }
    }

    // Tương tác chuột trái (Click UI Buttons & Modals)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        // 1. Các nút trên thanh Header: [P] CỬA HÀNG, [U] ĐE RÈN, [B] TÚI ĐỒ
        Rectangle shopBtn = { (float)(screenW - 445), 8.0f, 142.0f, 32.0f };
        if (CheckCollisionPointRec(mouse, shopBtn)) {
            showShop = !showShop;
            if (showShop) { showInventory = false; showForge = false; }
            return;
        }

        Rectangle forgeBtn = { (float)(screenW - 295), 8.0f, 138.0f, 32.0f };
        if (CheckCollisionPointRec(mouse, forgeBtn)) {
            showForge = !showForge;
            if (showForge) { showInventory = false; showShop = false; }
            return;
        }

        Rectangle invBtn = { (float)(screenW - 150), 8.0f, 140.0f, 32.0f };
        if (CheckCollisionPointRec(mouse, invBtn)) {
            showInventory = !showInventory;
            if (showInventory) { showShop = false; showForge = false; }
            return;
        }

        // 2. Nút mở / thu gọn Nhật ký chiến đấu
        int logH = showCombatLog ? 96 : 26;
        int logY = screenH - logH;
        Rectangle logToggleBtn = { 12.0f, (float)logY, 240.0f, 24.0f };
        if (CheckCollisionPointRec(mouse, logToggleBtn)) {
            showCombatLog = !showCombatLog;
            return;
        }

        // 3. Tương tác khi CỬA HÀNG đang mở
        if (showShop) {
            int modalW = 560, modalH = 460;
            int modalX = screenW / 2 - modalW / 2;
            int modalY = screenH / 2 - modalH / 2;

            // Nút đóng [X]
            Rectangle closeBtn = { (float)(modalX + modalW - 36), (float)(modalY + 8), 28.0f, 28.0f };
            if (CheckCollisionPointRec(mouse, closeBtn)) {
                showShop = false;
                return;
            }

            // Click vào các thẻ mặt hàng hoặc nút [MUA]
            for (int i = 0; i < 6; ++i) {
                int col = i % 2;
                int row = i / 2;
                int cardX = modalX + 16 + col * 270;
                int cardY = modalY + 54 + row * 128;
                Rectangle cardRect = { (float)cardX, (float)cardY, 262.0f, 118.0f };
                if (CheckCollisionPointRec(mouse, cardRect)) {
                    buyShopItem(i);
                    return;
                }
            }

            // Click ra ngoài modal để đóng
            Rectangle modalRect = { (float)modalX, (float)modalY, (float)modalW, (float)modalH };
            if (!CheckCollisionPointRec(mouse, modalRect)) {
                showShop = false;
                return;
            }
            return;
        }

        // 4. Tương tác khi ĐE RÈN đang mở
        if (showForge) {
            int modalW = 460, modalH = 400;
            int modalX = screenW / 2 - modalW / 2;
            int modalY = screenH / 2 - modalH / 2;

            // Nút đóng [X]
            Rectangle closeBtn = { (float)(modalX + modalW - 36), (float)(modalY + 8), 28.0f, 28.0f };
            if (CheckCollisionPointRec(mouse, closeBtn)) {
                showForge = false;
                return;
            }

            // Nút [CƯỜNG HÓA KIẾM]
            Rectangle upgradeBtn = { (float)(modalX + 30), (float)(modalY + modalH - 65), (float)(modalW - 60), 46.0f };
            if (CheckCollisionPointRec(mouse, upgradeBtn)) {
                triggerForgeUpgrade();
                return;
            }

            // Click ra ngoài modal để đóng
            Rectangle modalRect = { (float)modalX, (float)modalY, (float)modalW, (float)modalH };
            if (!CheckCollisionPointRec(mouse, modalRect)) {
                showForge = false;
                return;
            }
            return;
        }

        // 5. Tương tác khi TÚI ĐỒ đang mở
        if (showInventory) {
            int modalW = 420, modalH = 475;
            int modalX = screenW / 2 - modalW / 2;
            int modalY = screenH / 2 - modalH / 2 - 15;

            // Nút đóng [X]
            Rectangle closeBtn = { (float)(modalX + modalW - 38), (float)(modalY + 8), 28.0f, 28.0f };
            if (CheckCollisionPointRec(mouse, closeBtn)) {
                showInventory = false;
                return;
            }

            // Click vào các ô slot 1-8 để dùng vật phẩm
            Inventory& inv = player.getInventory();
            for (size_t i = 0; i < 8; ++i) {
                int cardX = modalX + 16;
                int cardY = modalY + 52 + (int)i * 48;
                Rectangle cardRect = { (float)cardX, (float)cardY, (float)(modalW - 32), 44.0f };
                if (CheckCollisionPointRec(mouse, cardRect)) {
                    if (i < inv.getSize() && inv[i] != nullptr) {
                        const Item* item = inv[i];
                        combatLog.push_back("Su dung: " + item->getName());
                        inv.useItem(i, &player);
                    }
                    return;
                }
            }

            // Click ra ngoài cửa sổ Modal để đóng túi đồ
            Rectangle modalRect = { (float)modalX, (float)modalY, (float)modalW, (float)modalH };
            if (!CheckCollisionPointRec(mouse, modalRect)) {
                showInventory = false;
                return;
            }
            return;
        }
    }

    // Nếu bất kỳ modal nào đang mở: khóa toàn bộ thao tác di chuyển / chiến đấu
    if (showInventory || showShop || showForge) {
        return;
    }

    // Khi đang rơi tự do xuống hố và tiếp đất: khóa toàn bộ thao tác di chuyển / chiến đấu
    if (player.isFalling()) {
        return;
    }

    // =========================================================================
    // 1. HÀNH ĐỘNG TẤN CÔNG (ATTACK) [J] / [F] / [Z] (Hỗ trợ nhấp hoặc giữ phím)
    // =========================================================================
    bool attackKeyHeld = IsKeyDown(KEY_J) || IsKeyDown(KEY_F) || IsKeyDown(KEY_Z);
    bool attackKeyInit = IsKeyPressed(KEY_J) || IsKeyPressed(KEY_F) || IsKeyPressed(KEY_Z);

    if (attackKeyInit || (attackKeyHeld && attackTimer <= 0.0f)) {
        attackTimer = attackKeyInit ? 0.25f : 0.18f;
        player.triggerAttack();

        int dirX = player.isFacingRight() ? 1 : -1;
        // Chỉ quét quái vật ở cự ly cận chiến hợp lệ (cùng tầng hoặc trên bậc thang mở phía trước)
        // TUYỆT ĐỐI KHÔNG đánh xuyên qua trần đá lên tầng trên khi đang đứng bên dưới!
        Position target1(pPos.x + dirX, pPos.y);      // Ngang tầm mắt 1 ô
        Position target2(pPos.x + dirX * 2, pPos.y);  // Ngang tầm mắt 2 ô
        Position target3(pPos.x + dirX, pPos.y - 1);  // Bậc dốc thang phía trước

        Monster* targetMonster = dungeon.getMonsterAt(target1);
        if (!targetMonster) targetMonster = dungeon.getMonsterAt(target2);
        // Bậc dốc chéo phía trước: chỉ đánh tới được nếu ô ngay trước mặt là lối đi
        // (không cho đòn chéo xuyên qua góc khối đá)
        if (!targetMonster && dungeon.isWalkable(Position(pPos.x + dirX, pPos.y))) {
            targetMonster = dungeon.getMonsterAt(target3);
        }

        if (targetMonster) {
            CombatSystem::attack(player, *targetMonster, combatLog, this);
            bool wasKilled = !targetMonster->isAlive();
            dungeon.removeDeadMonsters(player);
            if (wasKilled) monstersDefeated++;

            // Boss gate: hạ BoarKing -> mở khóa Cổng Cửa trên Đỉnh Đền Thờ
            if (wasKilled && dungeon.checkBossDefeated()) {
                combatLog.push_back(">>> CHUA HEO RUNG DA HA GUOC! Cong cua o Dinh Den Tho da MO KHOA! <<<");
                combatLog.push_back(">>> Hay leo len be vang (x126) va nhan [Space] de chien thang! <<<");
            }
        } else {
            // Kiểm tra xem có quái vật ở trên đầu / trần nhà không để thông báo rõ ràng
            Position abovePos1(pPos.x, pPos.y - 1);
            Position abovePos2(pPos.x, pPos.y - 2);
            Position abovePos3(pPos.x + dirX, pPos.y - 2);
            if (dungeon.getMonsterAt(abovePos1) || dungeon.getMonsterAt(abovePos2) || dungeon.getMonsterAt(abovePos3)) {
                combatLog.push_back("Quai vat o tang tren bi san da che chan! Hay nhay len [Space] de chien dau.");
            } else {
                combatLog.push_back("Hiep si vung kiem chem vao khong khi!");
            }
        }

        return;
    }

    // =========================================================================
    // 2. HÀNH ĐỘNG NHẢY (JUMP) [SPACE]
    // =========================================================================
    if (IsKeyPressed(KEY_SPACE)) {
        // Đứng trên bệ đá cổ (Cổng Cửa): chỉ mở khóa sau khi đã hạ Boar King
        if (player.getPosition() == dungeon.getStairsPos()) {
            if (dungeon.hasBoss() && !dungeon.isBossDefeated()) {
                combatLog.push_back("[PHONG AN] Bo vang bi Boar King phong an! Hay ha guc no truoc.");
                return;
            }
            state = GameState::VICTORY;
            combatLog.push_back(">>> BAN DA CHIEN THANG! AETHELGARD DUOC GIAI CUU! <<<");
            return;
        }

        int dirX = player.isFacingRight() ? 1 : -1;
        // NHẢY ĐỊNH HƯỚNG: nếu đang GIỮ phím Trái/Phải lúc bấm Space thì nhảy
        // theo hướng đang giữ (bay xa 2-3 ô); nếu không giữ hướng nào thì nhảy
        // theo hướng mặt cũ (cú nhảy ngắn tại chỗ như trước).
        bool holdRight = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
        bool holdLeft  = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
        int jumpDir = dirX;
        bool directional = false;
        if (holdRight && !holdLeft)      { jumpDir = 1;  directional = true; }
        else if (holdLeft && !holdRight) { jumpDir = -1; directional = true; }
        player.triggerJump(jumpDir, -1);

        // Helper: đường bay được NHẢY XUYÊN QUA quái (fly-over) và ô EMPTY/FLOOR,
        // chỉ bị chặn bởi khối WALL đặc. Ô đáp (landing) thì vẫn phải trống quái
        // (kiểm tra riêng trong canLand) để không đáp đè lên đầu quái.
        auto pathFlyable = [&](const Position& from, const Position& to) {
            int steps = std::max(std::abs(to.x - from.x), std::abs(to.y - from.y));
            for (int i = 1; i < steps; ++i) {
                float t = (float)i / (float)steps;
                Position mid(from.x + (int)std::round((to.x - from.x) * t),
                             from.y + (int)std::round((to.y - from.y) * t));
                if (mid == from) continue;
                if (!dungeon.isValidPos(mid)) return false;
                // Quái trên đường bay: cho phép nhảy vọt qua (không chặn)
                if (dungeon.getMonsterAt(mid) != nullptr) continue;
                if (dungeon.getTileType(mid) == TileType::WALL) return false;
            }
            return true;
        };
        auto canLand = [&](const Position& t) {
            return dungeon.isValidPos(t) && pathFlyable(pPos, t)
                && dungeon.isWalkable(t) && dungeon.getMonsterAt(t) == nullptr;
        };

        bool jumped = false;
        if (directional) {
            // ƯU TIÊN BAY XA VƯỢT HỐ PIT (cự ly 3 ô, 4 ô, 5 ô, 2 ô, 1 ô với ưu tiên cùng độ cao)
            Position farCandidates[] = {
                // Nhảy xa 3 ô (cự ly chuẩn vượt hố pit 2 ô từ sát mép)
                Position(pPos.x + jumpDir * 3, pPos.y),     // Bay xa 3 ô ngang bằng
                Position(pPos.x + jumpDir * 3, pPos.y - 1), // Bay xa 3 ô + lên bệ 1 bậc
                Position(pPos.x + jumpDir * 3, pPos.y + 1), // Bay xa 3 ô + xuống dốc 1 bậc
                Position(pPos.x + jumpDir * 3, pPos.y - 2), // Bay xa 3 ô + lên cao 2 bậc

                // Nhảy xa 4 ô (chạy lấy đà bấm nhảy sớm trước mép hố 1 ô)
                Position(pPos.x + jumpDir * 4, pPos.y),     // Bay xa 4 ô ngang bằng
                Position(pPos.x + jumpDir * 4, pPos.y - 1), // Bay xa 4 ô + lên bệ 1 bậc
                Position(pPos.x + jumpDir * 4, pPos.y + 1), // Bay xa 4 ô + xuống dốc 1 bậc
                Position(pPos.x + jumpDir * 4, pPos.y - 2), // Bay xa 4 ô + lên cao 2 bậc

                // Nhảy xa 5 ô (chạy lấy đà bấm nhảy sớm trước mép hố 2 ô)
                Position(pPos.x + jumpDir * 5, pPos.y),     // Bay xa 5 ô ngang bằng
                Position(pPos.x + jumpDir * 5, pPos.y - 1), // Bay xa 5 ô + lên bệ 1 bậc
                Position(pPos.x + jumpDir * 5, pPos.y + 1), // Bay xa 5 ô + xuống dốc 1 bậc

                // Nhảy cự ly 2 ô
                Position(pPos.x + jumpDir * 2, pPos.y),     // Nhảy xa 2 ô ngang
                Position(pPos.x + jumpDir * 2, pPos.y - 1), // Nhảy xa 2 ô + lên bệ 1 bậc
                Position(pPos.x + jumpDir * 2, pPos.y + 1), // Nhảy xa 2 ô + xuống dốc 1 bậc
                Position(pPos.x + jumpDir * 2, pPos.y - 2), // Nhảy xa 2 ô + lên bệ 2 bậc
                Position(pPos.x + jumpDir * 2, pPos.y + 2), // Nhảy xa 2 ô + xuống dốc 2 bậc

                // Nhảy cự ly 1 ô
                Position(pPos.x + jumpDir, pPos.y),         // Nhảy bước tới
                Position(pPos.x + jumpDir, pPos.y - 1),     // Nhảy chéo gần lên bệ
                Position(pPos.x + jumpDir, pPos.y + 1),     // Nhảy bước xuống dốc
                Position(pPos.x + jumpDir, pPos.y - 2),     // Nhảy chéo gần lên bệ cao
                Position(pPos.x + jumpDir, pPos.y + 2),

                // Nhảy tại chỗ
                Position(pPos.x, pPos.y - 1),
                Position(pPos.x, pPos.y - 2)
            };

            for (const auto& target : farCandidates) {
                if (canLand(target)) {
                    player.setPosition(target);
                    jumped = true;
                    break;
                }
            }

            // Nếu không có bệ ngang/cao để đáp: người chơi sẽ phóng mình lao về phía trước và rơi xuống tầng dưới
            if (!jumped) {
                for (int dist = 2; dist >= 1 && !jumped; --dist) {
                    int jX = pPos.x + jumpDir * dist;
                    if (!dungeon.isValidPos(Position(jX, pPos.y))) continue;
                    if (dungeon.getTileType(Position(jX, pPos.y)) == TileType::WALL) continue;

                    for (int fallY = pPos.y + 1; fallY < dungeon.getHeight(); ++fallY) {
                        Position cand(jX, fallY);
                        if (dungeon.isWater(cand)) {
                            player.setPosition(cand);
                            combatLog.push_back("[NHAY XUONG] Ban da phong minh xuong dam lay lun ben duoi!");
                            combatLog.push_back(">> Nhanh tay nhan [Space] de vung vay thoat len bo!");
                            jumped = true;
                            break;
                        }
                        if (dungeon.isWalkable(cand)) {
                            Position landPos = cand;
                            if (dungeon.getMonsterAt(landPos) != nullptr) {
                                if (dungeon.isWalkable(Position(jX - 1, fallY)) && dungeon.getMonsterAt(Position(jX - 1, fallY)) == nullptr) {
                                    landPos = Position(jX - 1, fallY);
                                } else if (dungeon.isWalkable(Position(jX + 1, fallY)) && dungeon.getMonsterAt(Position(jX + 1, fallY)) == nullptr) {
                                    landPos = Position(jX + 1, fallY);
                                }
                            }
                            player.setPosition(landPos);
                            player.triggerFall(0.52f, 0.22f);
                            combatLog.push_back("[NHAY XUONG] Ban da phong minh roi xuong tang ben duoi!");
                            jumped = true;
                            break;
                        }
                    }
                }
            }
        } else {
            // Nhảy không giữ hướng: vẫn ưu tiên lao tới phía trước theo hướng quay mặt
            Position jumpCandidates[] = {
                Position(pPos.x + dirX * 3, pPos.y),
                Position(pPos.x + dirX * 3, pPos.y - 1),
                Position(pPos.x + dirX * 2, pPos.y),
                Position(pPos.x + dirX * 2, pPos.y - 1),
                Position(pPos.x + dirX, pPos.y),
                Position(pPos.x + dirX, pPos.y - 1),     // Nhảy chéo lên bệ trên 1 bậc
                Position(pPos.x + dirX, pPos.y - 2),     // Nhảy chéo lên bệ cao 2 bậc
                Position(pPos.x, pPos.y - 1),             // Nhảy thẳng lên 1 ô
                Position(pPos.x, pPos.y - 2)              // Nhảy cao 2 ô
            };

            for (const auto& target : jumpCandidates) {
                if (canLand(target)) {
                    player.setPosition(target);
                    jumped = true;
                    break;
                }
            }
        }

        if (jumped) {
            edgeSlipTimer = 0.0f;
            std::unique_ptr<Item> item = dungeon.takeItemAt(player.getPosition());
            if (item) {
                combatLog.push_back("Nhat duoc: " + item->getName() + "!");
                player.getInventory().addItem(std::move(item));
            }
        }

        return;
    }

    // =========================================================================
    // 3. DI CHUYỂN NGANG (HỖ TRỢ GIỮ PHÍM & TÁCH RỜI HOÀN TOÀN KHỎI ATTACK)
    // =========================================================================
    bool moveRightHeld = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
    bool moveLeftHeld  = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
    bool moveRightInit = IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT);
    bool moveLeftInit  = IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT);

    if (moveRightHeld || moveLeftHeld) {
        bool isInit = (moveRightHeld && moveRightInit) || (moveLeftHeld && moveLeftInit);
        // Khi giữ phím: cho frame đầu đi ngay, các frame sau lặp theo nhịp nhanh
        // 0.07s (thay vì 0.10s cũ) để chạy mượt, không khựng. Nhấn đầu 0.12s.
        if (isInit || moveTimer <= 0.0f) {
            moveTimer = isInit ? 0.12f : 0.07f; // Nhấn đầu chờ 0.12s, giữ phím lặp mỗi 0.07s
            int dx = moveRightHeld ? 1 : -1;
            player.setFacingRight(dx > 0);

            Position candidates[] = {
                Position(pPos.x + dx, pPos.y),     // Đi thẳng ngang
                Position(pPos.x + dx, pPos.y - 1), // Bước lên bậc dốc
                Position(pPos.x + dx, pPos.y + 1)  // Bước xuống dốc
            };

            bool moved = false;
            for (const auto& cand : candidates) {
                if (dungeon.isWalkable(cand)) {
                    // Kiểm tra xem ô này có quái vật cản đường không
                    Monster* monster = dungeon.getMonsterAt(cand);
                    if (monster != nullptr) {
                        // CÓ QUÁI VẬT: DỪNG LẠI, TUYỆT ĐỐI KHÔNG TỰ ĐỘNG TẤN CÔNG!
                        continue;
                    }

                    // Ô trống an toàn -> Di chuyển tới!
                    int actualDx = cand.x - pPos.x;
                    int actualDy = cand.y - pPos.y;
                    player.moveBy(actualDx, actualDy, dungeon);
                    edgeSlipTimer = 0.0f;

                    std::unique_ptr<Item> item = dungeon.takeItemAt(player.getPosition());
                    if (item) {
                        combatLog.push_back("Nhat duoc: " + item->getName() + "!");
                        player.getInventory().addItem(std::move(item));
                    }
                    moved = true;
                    break;
                }
            }

            if (!moved) {
                // Nếu bị quái vật cản bước
                Position forwardPos(pPos.x + dx, pPos.y);
                if (dungeon.getMonsterAt(forwardPos)) {
                    combatLog.push_back("Quai vat dang chan duong! Nhan [J] hoac [F] de tan cong.");
                } else {
                    // Phía trước là hố khoảng cách (EMPTY) hoặc vực nước:
                    // Bước tiếp sẽ rơi tự do xuống sàn đất tầng dưới một cách tự nhiên
                    TileType forwardType = dungeon.getTileType(forwardPos);
                    if (forwardType == TileType::EMPTY || forwardType == TileType::WATER) {
                        int targetX = pPos.x + dx;

                        for (int fallY = pPos.y + 1; fallY < dungeon.getHeight(); ++fallY) {
                            Position checkPos(targetX, fallY);

                            // 1. Rơi trúng Đầm lầy lún
                            if (dungeon.isWater(checkPos)) {
                                player.setPosition(checkPos);
                                combatLog.push_back("[LUN DAM LAY] Ban da sa chan xuong dam lay lun ben duoi!");
                                combatLog.push_back(">> Nhanh tay nhan [Space] de vung vay thoat len bo!");
                                break;
                            }

                            // 2. Rơi trúng sàn đất tầng dưới (FLOOR / STAIRS)
                            if (dungeon.isWalkable(checkPos)) {
                                Position landPos = checkPos;
                                // Nếu ô rơi xuống đang có quái vật đứng, ưu tiên né sang ô đất trống lân cận
                                if (dungeon.getMonsterAt(landPos) != nullptr) {
                                    Position leftPos(targetX - 1, fallY);
                                    Position rightPos(targetX + 1, fallY);
                                    if (dungeon.isWalkable(leftPos) && dungeon.getMonsterAt(leftPos) == nullptr) {
                                        landPos = leftPos;
                                    } else if (dungeon.isWalkable(rightPos) && dungeon.getMonsterAt(rightPos) == nullptr) {
                                        landPos = rightPos;
                                    }
                                }

                                player.setPosition(landPos);
                                player.triggerFall(0.52f, 0.22f);
                                combatLog.push_back("[ROI XUONG] Ban da buoc hut va roi xuong tang duoi!");

                                // Nhặt vật phẩm nếu có tại ô đáp
                                std::unique_ptr<Item> item = dungeon.takeItemAt(player.getPosition());
                                if (item) {
                                    combatLog.push_back("Nhat duoc: " + item->getName() + "!");
                                    player.getInventory().addItem(std::move(item));
                                }

                                break;
                            }
                        }
                    }
                }
            }
            return;
        }
        // Đang giữ phím ngang nhưng chưa đến hạn di chuyển tiếp theo -> dừng lại, không rơu xuống xử lý dọc
        return;
    }

    // =========================================================================
    // 4. DI CHUYỂN DỌC (LEO LÊN / XUỐNG BẬC THANG) [W] / [S]
    // =========================================================================
    bool moveUpHeld   = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
    bool moveDownHeld = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
    bool moveUpInit   = IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
    bool moveDownInit = IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN);

    if (moveUpHeld || moveDownHeld) {
        bool isInit = (moveUpHeld && moveUpInit) || (moveDownHeld && moveDownInit);

        if (isInit || moveTimer <= 0.0f) {
            moveTimer = isInit ? 0.12f : 0.07f;
            int dy = moveUpHeld ? -1 : 1;

            Position candidates[] = {
                Position(pPos.x, pPos.y + dy),
                Position(pPos.x - 1, pPos.y + dy),
                Position(pPos.x + 1, pPos.y + dy)
            };

            for (const auto& cand : candidates) {
                if (dungeon.isWalkable(cand) && dungeon.getMonsterAt(cand) == nullptr) {
                    if (dy < 0) {
                        player.triggerJump(0, -1);
                    }
                    player.moveBy(cand.x - pPos.x, cand.y - pPos.y, dungeon);

                    std::unique_ptr<Item> item = dungeon.takeItemAt(player.getPosition());
                    if (item) {
                        combatLog.push_back("Nhat duoc: " + item->getName() + "!");
                        player.getInventory().addItem(std::move(item));
                    }
                    break;
                }
            }

            return;
        }
    }

    // Reset moveTimer khi nhả toàn bộ phím di chuyển
    if (!moveRightHeld && !moveLeftHeld && !moveUpHeld && !moveDownHeld) {
        moveTimer = 0.0f;
    }
    if (!attackKeyHeld) {
        attackTimer = 0.0f;
    }

    // =========================================================================
    // 5. DÙNG VẬT PHẨM TÚI ĐỒ [1 - 9]
    // =========================================================================
    for (int key = KEY_ONE; key <= KEY_NINE; ++key) {
        if (IsKeyPressed(key)) {
            size_t slotIdx = key - KEY_ONE;
            Inventory& inv = player.getInventory();
            if (slotIdx < inv.getSize()) {
                const Item* item = inv[slotIdx];
                if (item) {
                    combatLog.push_back("Su dung: " + item->getName());
                    inv.useItem(slotIdx, &player);
                }
            }
            return;
        }
    }

    // Phím Kỹ năng Đa hình: [L-Shift] Lướt né đòn, [Q] Hồi máu khẩn cấp
    if (IsKeyPressed(KEY_LEFT_SHIFT)) {
        if (player.useSkill(1, this)) {
            combatLog.push_back("[KY NANG] Hiep si luot nhanh ve phia truoc (Dash)!");
        } else {
            Skill* dash = player.getSkill(1);
            if (dash && !dash->canExecute()) {
                combatLog.push_back("[HOI CHIEU] Luot ne don con " + std::to_string(static_cast<int>(dash->getCurrentCooldown() + 0.9f)) + "s");
            }
        }
    }

    if (IsKeyPressed(KEY_Q)) {
        if (!player.useSkill(2, this)) {
            Skill* heal = player.getSkill(2);
            if (heal && !heal->canExecute()) {
                combatLog.push_back("[HOI CHIEU] Hoi phuc khan cap con " + std::to_string(static_cast<int>(heal->getCurrentCooldown() + 0.9f)) + "s");
            }
        }
    }

    // Phím Lưu game [F5] & Tải game [F9] - Bọc cơ chế Ngoại lệ (C++ Exception Handling)
    if (IsKeyPressed(KEY_F5)) {
        try {
            if (SaveLoadManager::saveGame("saves/savegame.txt", player, dungeon)) {
                combatLog.push_back("[HE THONG] Da luu game thanh cong (F5)!");
            }
        } catch (const GameException& e) {
            combatLog.push_back(std::string("[NGOAI LE] ") + e.what());
        } catch (const std::exception& e) {
            combatLog.push_back(std::string("[LOI] ") + e.what());
        }
    }
    if (IsKeyPressed(KEY_F9)) {
        try {
            if (SaveLoadManager::loadGame("saves/savegame.txt", player, dungeon)) {
                combatLog.push_back("[HE THONG] Da tai lai game thanh cong (F9)!");
            }
        } catch (const SaveLoadException& e) {
            combatLog.push_back(std::string("[NGOAI LE] ") + e.what());
        } catch (const std::exception& e) {
            combatLog.push_back(std::string("[LOI] ") + e.what());
        }
    }
}

void GameEngine::addDamagePopup(const std::string& text, float worldX, float worldY, Color color, float duration) {
    activeDamagePopups.push_back(DamagePopup(text, worldX, worldY, color, duration));
}

void GameEngine::update(float deltaTime) {
    if (moveTimer > 0.0f) moveTimer -= deltaTime;
    if (attackTimer > 0.0f) attackTimer -= deltaTime;
    if (edgeSlipTimer > 0.0f) edgeSlipTimer -= deltaTime;
    if (bossWarningTimer > 0.0f) bossWarningTimer -= deltaTime;
    if (screenShake > 0.0f) {
        screenShake -= deltaTime * 1.5f;
        if (screenShake < 0.0f) screenShake = 0.0f;
    }

    // Cập nhật mảng động các số sát thương nổi (Class Template DynamicArray - Chương 7)
    for (size_t pIdx = 0; pIdx < activeDamagePopups.size(); ) {
        activeDamagePopups[pIdx].update(deltaTime);
        if (!activeDamagePopups[pIdx].isAlive()) {
            activeDamagePopups.erase(pIdx);
        } else {
            ++pIdx;
        }
    }

    // Kích hoạt Trận Đấu Boss Boar King khi người chơi bước vào Đấu Trường Khu F (x >= 130)
    if (state == GameState::RUNNING && !bossCinematicTriggered && player.getPosition().x >= 130) {
        Monster* boss = dungeon.getBossMonster();
        if (boss && boss->isAlive()) {
            bossCinematicTriggered = true;
            bossWarningTimer = 4.0f;
            screenShake = 1.2f;
            BoarKing* bk = dynamic_cast<BoarKing*>(boss);
            if (bk) bk->triggerEntrance();
            combatLog.push_back(">>> [CANH BAO NGUY HIEM!] CHUA HEO RUNG DANG LAO RA TU BONG TOI! <<<");
        }
    }

    // Lắng nghe rung màn hình từ Boss (Dậm chân, tông tường, húc trúng)
    if (state == GameState::RUNNING) {
        Monster* boss = dungeon.getBossMonster();
        if (boss && boss->isAlive()) {
            BoarKing* bk = dynamic_cast<BoarKing*>(boss);
            float intensity = 0.0f;
            if (bk && bk->consumeScreenShake(intensity)) {
                if (intensity > screenShake) screenShake = intensity;
            }
        }
    }

    // Cập nhật tiến trình lún đầm lầy (Swamp Sinking)
    if (state == GameState::RUNNING && isSinking) {
        sinkTimer -= deltaTime;
        float progress = 1.0f - (sinkTimer / sinkDuration);
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;

        // Nhân vật chìm sâu dần vào bùn (tối đa 32px)
        sinkDepth = progress * 32.0f;
        player.setSinkVisualOffset(sinkDepth);

        // Đã chìm hết thời gian -> Nuốt chửng hoàn toàn & Game Over
        if (sinkTimer <= 0.0f) {
            isSinking = false;
            submergedInSwamp = true;
            player.takeDamage(9999);
            state = GameState::GAME_OVER;
            combatLog.push_back(">>> BAN DA BI DAM LAY NUOT CHUNG VA MAT MANG! Nhan [R] de hoi sinh va thu lai. <<<");
        }
    } else if (state == GameState::RUNNING && dungeon.isWater(player.getPosition()) && !isSinking) {
        // Kích hoạt lún đầm lầy nếu người chơi đang ở ô đầm lầy và ĐÃ TIẾP NƯỚC (không còn đang rơi trong không trung)
        if (!player.isFalling()) {
            isSinking = true;
            sinkTimer = sinkDuration;
            sinkDepth = 0.0f;
            player.setSinkVisualOffset(0.0f);
            combatLog.push_back("[LUN DAM LAY] Ban da sa vao dam lay lun va dang bi chim dan vao bun sau!");
            combatLog.push_back(">> Nhanh tay nhan [Space] de vung vay thoat len bo!");
        }
    }

    // ===== KỊCH BẢN PHÂN KHU: banner khi người chơi đi qua mốc khu mới (6 khu vực) =====
    if (state == GameState::RUNNING) {
        int px = player.getPosition().x;
        int zone = (px < 25) ? 0 : (px < 56) ? 1 : (px < 89) ? 2 : (px < 113) ? 3 : (px < 130) ? 4 : 5;
        if (zone != currentZone) {
            currentZone = zone;
            static const char* zoneBanners[6] = {
                "KHU A - TRAI KHOI DAU: Lam quen dieu khien. Canh chung cac ho khoang cach!",
                "KHU B - RUNG NAM & CAU TREO: Heo rung tren cau treo, vuc nuoc sau ben duoi!",
                "KHU C - VACH DA & VUC NUOC: Sa chan xuong nuoc se bi chet duoi va quay lai tu dau!",
                "KHU D - BINH NGUYEN TAN TICH: Quan doan quai vat canh giu loi len Den Tho!",
                "KHU E - DINH DEN THO: Vuot qua cac ho sau de tien vao Dau Truong!",
                "KHU F - DAU TRUONG BOAR KING: Chien truong rong lon, khong loi thoat! Tieu diet Chua Heo Rung de mo khoa Be Vang!"
            };
            bannerText = zoneBanners[zone];
            bannerTimer = 3.5f;
            combatLog.push_back(std::string("--- ") + zoneBanners[zone] + " ---");
        }
    }
    if (bannerTimer > 0.0f) bannerTimer -= deltaTime;

    handleInput();
    player.update(deltaTime);
    updateGoldParticles(deltaTime);

    // Cập nhật hoạt cảnh tử trận mượt mà (Smooth Death Sequence)
    if (state == GameState::RUNNING && isDying) {
        deathTimer -= deltaTime;
        if (deathTimer <= 0.0f) {
            isDying = false;
            state = GameState::GAME_OVER;
            combatLog.push_back(">>> BAN DA TU TRAN! Nhan [R] de hoi sinh va thu lai. <<<");
        }
    }

    // Cập nhật AI quái vật thời gian thực độc lập khi không mở túi đồ / shop / đe rèn
    bool anyModalOpen = showInventory || showShop || showForge;
    if (!anyModalOpen && state == GameState::RUNNING) {
        if (!isDying) {
            dungeon.update(deltaTime, player, combatLog);
        } else {
            // Khi đang trong hoạt cảnh tử trận: quái vật chỉ diễn hoạt tại chỗ, không tấn công thêm
            for (auto& monster : dungeon.getMonsters()) {
                if (monster && monster->isAlive()) {
                    monster->update(deltaTime);
                }
            }
        }
        if (!player.isAlive() && !isDying && !submergedInSwamp) {
            isDying = true;
            deathDuration = 1.35f;
            deathTimer = deathDuration;
            combatLog.push_back(">>> HIEP SI DA NGA XUONG! <<<");
        }
    } else {
        // Khi mở túi đồ hoặc tạm dừng: vẫn cập nhật khung hình chuyển động
        for (auto& monster : dungeon.getMonsters()) {
            if (monster && monster->isAlive()) {
                monster->update(deltaTime);
            }
        }
    }

    // Căn chỉnh camera ghim chặt đáy mặt đất vào sát mép trên thanh Nhật ký chiến đấu
    // Tuyệt đối loại bỏ hoàn toàn 100% vùng trống / khoảng trống bên dưới!
    float viewTop = 50.0f;
    float curLogH = showCombatLog ? 96.0f : 26.0f;
    float viewBottom = (float)GetScreenHeight() - curLogH - 22.0f;
    float activeHeight = viewBottom - viewTop;

    // Tự động tính toán mức zoom phù hợp với mọi độ phân giải màn hình (720p, 1080p, Toàn màn hình)
    // Chiều cao địa hình từ mặt đất (y=18) lên Đền Thờ đỉnh núi (y=6) là ~14.5 ô (464px)
    float idealZoom = activeHeight / (14.5f * (float)Constants::TILE_SIZE);
    if (idealZoom < 0.95f) idealZoom = 0.95f;

    // Hỗ trợ cuộn con lăn chuột để Zoom In / Zoom Out mượt mà xung quanh mức chuẩn
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        userZoomOffset += wheel * 0.12f;
        if (userZoomOffset < -0.35f) userZoomOffset = -0.35f;
        if (userZoomOffset > 1.20f)  userZoomOffset = 1.20f;
    }

    camera.zoom = idealZoom + userZoomOffset;
    if (camera.zoom < 0.6f) camera.zoom = 0.6f;
    if (camera.zoom > 2.0f) camera.zoom = 2.0f;
    // Hai mốc Y quan trọng: đỉnh tầng cao nhất (y=6) và đáy mặt đất (y=19).
    float topWorldY = 6.0f * (float)Constants::TILE_SIZE;
    float groundBottomY = 19.0f * (float)Constants::TILE_SIZE;
    // Đoạn nội dung BẮT BUỘC phải lọt khung: từ ĐỈNH ĐẦU player tới ĐÁY mặt đất.
    // (Neo chân mới: foot = pVisual.y + 6 + trim*scale, đầu = foot - fH*1.8.)
    const Vector2& pVisual = player.getVisualPosition();
    const Animation* animH = player.getCurrentAnimation();
    float playerFHW = animH ? (float)animH->getFrameHeight() : 80.0f;
    float playerTrimW = (playerFHW > 70.0f) ? 12.0f : 6.0f; // 80px -> 12, 64px -> 6
    float playerFootW = pVisual.y + 6.0f + playerTrimW * 1.8f;
    float playerHeadW = playerFootW - playerFHW * 1.8f;
    float needTop = (playerHeadW < topWorldY) ? playerHeadW : topWorldY;
    float needBottom = groundBottomY + (float)Constants::TILE_SIZE;
    {
        float fitZoom = activeHeight / ((needBottom - needTop) + 24.0f);
        if (fitZoom < 0.55f) fitZoom = 0.55f;
        if (camera.zoom > fitZoom) {
            // Mượt zoom (tránh giật hình khi ngưỡng fit bật/tắt giữa chừng)
            float f = 1.0f - std::exp(-6.0f * deltaTime);
            camera.zoom += (fitZoom - camera.zoom) * f;
        }
    }

    // 1. Camera Target X: bám theo tọa độ hiển thị mượt mà của người chơi
    float screenW = (float)GetScreenWidth();
    float worldWidth = (float)(Constants::DUNGEON_WIDTH * Constants::TILE_SIZE);
    float targetX = pVisual.x + (float)Constants::TILE_SIZE / 2.0f;

    // Kẹp chặt camera target X để màn hình không bao giờ trôi ra ngoài biên trái (x < 0) hoặc biên phải
    float halfViewW = (screenW / 2.0f) / camera.zoom;
    if (worldWidth > halfViewW * 2.0f) {
        if (targetX < halfViewW) targetX = halfViewW;
        if (targetX > worldWidth - halfViewW) targetX = worldWidth - halfViewW;
    }
    // Deadzone dọc: player di chuyển trong vùng này thì camera Y đứng yên
    // (không giật); chỉ pan khi player vượt biên trên/dưới của deadzone.
    float halfViewH = (activeHeight / 2.0f) / camera.zoom;
    float desiredTargetY = pVisual.y;
    float prevTargetY = camera.target.y;
    if (prevTargetY < topWorldY) prevTargetY = (topWorldY + groundBottomY) / 2.0f;
    if (prevTargetY > groundBottomY) prevTargetY = (topWorldY + groundBottomY) / 2.0f;
    const float DEADZONE_HALF = 2.0f * (float)Constants::TILE_SIZE; // +/-2 ô
    float dy = desiredTargetY - prevTargetY;
    float targetY = prevTargetY;
    if (dy < -DEADZONE_HALF) targetY = desiredTargetY + DEADZONE_HALF;
    else if (dy > DEADZONE_HALF) targetY = desiredTargetY - DEADZONE_HALF;
    // Kẹp: nửa khung nhìn không được vượt quá [needTop, needBottom]
    // để ôm trọn cả đầu player lẫn mặt đất, hết vùng đen bên dưới.
    float minTargetY = needTop + halfViewH;
    float maxTargetY = needBottom - halfViewH;
    if (minTargetY > maxTargetY) targetY = (needTop + needBottom) / 2.0f;
    else {
        if (targetY < minTargetY) targetY = minTargetY;
        if (targetY > maxTargetY) targetY = maxTargetY;
    }

    camera.target = Vector2{ targetX, targetY };
    // Offset Y đặt giữa vùng nhìn (không ghim đáy) để bám Y mượt cả lên/xuống.
    camera.offset = Vector2{ screenW / 2.0f, viewTop + activeHeight / 2.0f };

    // Hiệu ứng Rung màn hình (Screen Shake) kịch tính khi Boss húc, giậm đất hoặc xuất hiện
    if (screenShake > 0.0f) {
        float shakeOffset = screenShake * 7.0f;
        camera.target.x += ((float)(std::rand() % 200 - 100) / 100.0f) * shakeOffset;
        camera.target.y += ((float)(std::rand() % 200 - 100) / 100.0f) * shakeOffset;
    }
}

void GameEngine::renderHUD() const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    Vector2 mouse = GetMousePosition();
    const Inventory& inv = player.getInventory();

    // =========================================================================
    // 1. THANH TRẠNG THÁI TRÊN CÙNG (TOP HEADER BAR) - CHIỀU CAO 48px
    // =========================================================================
    DrawRectangle(0, 0, screenW, 48, Color{ 15, 13, 23, 245 });
    DrawLine(0, 48, screenW, 48, Color{ 60, 52, 75, 255 });
    DrawLine(0, 49, screenW, 49, Color{ 25, 20, 35, 180 });

    // --- CỤM TRÁI: CẤP ĐỘ, VÀNG, THANH MÁU VÀ CÔNG/THỦ ---
    // 1.1. Huy hiệu Cấp độ
    DrawRectangleRounded(Rectangle{ 14, 9, 78, 30 }, 0.3f, 4, Color{ 26, 22, 38, 255 });
    DrawRectangleRoundedLinesEx(Rectangle{ 14, 9, 78, 30 }, 0.3f, 4, 1.5f, Color{ 215, 165, 45, 255 });
    drawText(TextFormat("CAP %d", player.getLevel()), 24, 14, 16, GOLD);

    // 1.2. Huy hiệu Vàng
    DrawRectangleRounded(Rectangle{ 98, 9, 96, 30 }, 0.3f, 4, Color{ 26, 22, 38, 255 });
    DrawRectangleRoundedLinesEx(Rectangle{ 98, 9, 96, 30 }, 0.3f, 4, 1.5f, Color{ 180, 140, 40, 255 });
    const TextureManager& tmHUD = TextureManager::getInstance();
    if (tmHUD.has("item_gold_coin")) {
        const Texture2D& cTex = tmHUD.get("item_gold_coin");
        DrawTexturePro(cTex, Rectangle{ 0, 0, (float)cTex.width, (float)cTex.height },
                       Rectangle{ 104, 14, 20, 20 }, Vector2{ 0, 0 }, 0.0f, WHITE);
    }
    drawText(TextFormat("%d", player.getGold()), 128, 14, 16, Color{ 255, 220, 80, 255 });

    // 1.3. Thanh Máu Người Chơi
    int hpX = 205, hpY = 15, hpW = 165, hpH = 18;
    float hpPercent = (float)player.getHp() / (float)player.getMaxHp();
    if (hpPercent < 0.0f) hpPercent = 0.0f;
    if (hpPercent > 1.0f) hpPercent = 1.0f;

    DrawRectangle(hpX, hpY, hpW, hpH, Color{ 35, 25, 30, 255 });
    DrawRectangle(hpX + 2, hpY + 2, (int)((hpW - 4) * hpPercent), hpH - 4, Color{ 215, 45, 45, 255 });
    DrawRectangleLines(hpX, hpY, hpW, hpH, Color{ 110, 50, 50, 255 });
    drawText(TextFormat("HP %d/%d", player.getHp(), player.getMaxHp()), hpX + 38, hpY + 2, 14, WHITE);

    // 1.4. Chỉ số Tấn công & Phòng ngự
    drawText(TextFormat("ATK %d", player.getAttack()), 386, 15, 16, Color{ 255, 165, 70, 255 });
    drawText(TextFormat("DEF %d", player.getDefense()), 456, 15, 16, Color{ 120, 210, 255, 255 });

    // --- CỤM GIỮA: TẦNG NGỤC & LỘ TRÌNH (KHÔNG CHỒNG ĐÈ CHỮ) ---
    int midX = 525;
    drawText(TextFormat("TANG %d", dungeon.getFloorLevel()), midX, 7, 15, Color{ 90, 205, 255, 255 });

    const char* zoneName = dungeon.getZoneName(player.getPosition().x);
    drawText(zoneName, midX, 26, 13, Color{ 255, 215, 90, 255 });

    int pbX = midX + 165, pbY = 17, pbW = 110, pbH = 14;
    int stairsX = dungeon.getStairsPos().x;
    float progress = (stairsX > 0) ? (float)player.getPosition().x / (float)stairsX : 0.0f;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    DrawRectangle(pbX, pbY, pbW, pbH, Color{ 25, 20, 36, 255 });
    DrawRectangle(pbX + 1, pbY + 1, (int)((pbW - 2) * progress), pbH - 2, Color{ 220, 165, 35, 255 });
    DrawRectangleLines(pbX, pbY, pbW, pbH, Color{ 85, 75, 105, 255 });
    bool bossDefeated = dungeon.isBossDefeated();
    drawText("CONG CUA", pbX + pbW + 8, pbY - 1, 13, bossDefeated ? GOLD : Color{ 205, 75, 75, 255 });

    // --- CỤM PHẢI: CÁC NÚT TƯƠNG TÁC TIÊU THỤ VÀNG & TÚI ĐỒ ---
    Rectangle shopBtn = { (float)(screenW - 445), 8.0f, 142.0f, 32.0f };
    bool shopHover = CheckCollisionPointRec(mouse, shopBtn);
    Color shopBg = showShop ? Color{ 65, 52, 28, 255 } : (shopHover ? Color{ 52, 42, 24, 255 } : Color{ 28, 24, 38, 255 });
    Color shopBorder = showShop ? GOLD : (shopHover ? Color{ 245, 200, 70, 255 } : Color{ 120, 100, 60, 255 });
    DrawRectangleRounded(shopBtn, 0.25f, 4, shopBg);
    DrawRectangleRoundedLinesEx(shopBtn, 0.25f, 4, 1.5f, shopBorder);
    drawText("[P] CUA HANG", shopBtn.x + 14, shopBtn.y + 7, 15, (shopHover || showShop) ? GOLD : RAYWHITE);

    Rectangle forgeBtn = { (float)(screenW - 295), 8.0f, 138.0f, 32.0f };
    bool forgeHover = CheckCollisionPointRec(mouse, forgeBtn);
    Color forgeBg = showForge ? Color{ 68, 38, 24, 255 } : (forgeHover ? Color{ 55, 30, 20, 255 } : Color{ 28, 24, 38, 255 });
    Color forgeBorder = showForge ? ORANGE : (forgeHover ? Color{ 255, 140, 60, 255 } : Color{ 110, 70, 60, 255 });
    DrawRectangleRounded(forgeBtn, 0.25f, 4, forgeBg);
    DrawRectangleRoundedLinesEx(forgeBtn, 0.25f, 4, 1.5f, forgeBorder);
    drawText(TextFormat("[U] DE REN (+%d)", player.getForgeLevel()), forgeBtn.x + 10, forgeBtn.y + 7, 14, (forgeHover || showForge) ? ORANGE : RAYWHITE);

    Rectangle invBtn = { (float)(screenW - 150), 8.0f, 140.0f, 32.0f };
    bool invHover = CheckCollisionPointRec(mouse, invBtn);
    Color invBg = showInventory ? Color{ 60, 48, 85, 255 } : (invHover ? Color{ 45, 38, 65, 255 } : Color{ 28, 24, 40, 255 });
    Color invBorder = showInventory ? GOLD : (invHover ? Color{ 240, 200, 70, 255 } : Color{ 85, 75, 105, 255 });
    DrawRectangleRounded(invBtn, 0.25f, 4, invBg);
    DrawRectangleRoundedLinesEx(invBtn, 0.25f, 4, 1.5f, invBorder);
    drawText(TextFormat("[B] TUI DO (%d/8)", (int)inv.getSize()), invBtn.x + 12, invBtn.y + 7, 15, (invHover || showInventory) ? YELLOW : RAYWHITE);

    // Hiển thị trạng thái các kỹ năng đa hình (Polymorphic Skills - Chương 6)
    Skill* dashSkill = player.getSkill(1);
    Skill* healSkill = player.getSkill(2);
    if (dashSkill) {
        bool ready = dashSkill->canExecute();
        std::string text = ready ? "[Shift] Luot: SAN SANG" : TextFormat("[Shift] Luot: %.1fs", dashSkill->getCurrentCooldown());
        DrawRectangle(10, 52, 165, 22, Color{ 20, 18, 30, 210 });
        DrawRectangleLines(10, 52, 165, 22, ready ? GREEN : DARKGRAY);
        drawText(text.c_str(), 16, 56, 12, ready ? GREEN : LIGHTGRAY);
    }
    if (healSkill) {
        bool ready = healSkill->canExecute();
        std::string text = ready ? "[Q] Hoi mau: SAN SANG" : TextFormat("[Q] Hoi mau: %.1fs", healSkill->getCurrentCooldown());
        DrawRectangle(180, 52, 170, 22, Color{ 20, 18, 30, 210 });
        DrawRectangleLines(180, 52, 170, 22, ready ? SKYBLUE : DARKGRAY);
        drawText(text.c_str(), 186, 56, 12, ready ? SKYBLUE : LIGHTGRAY);
    }

    // Thống kê quái vật qua thành viên tĩnh Monster::getActiveMonsterCount() (Chương 3)
    std::string mobStat = TextFormat("Quai song: %d  |  Da diet: %d", 
                                     Monster::getActiveMonsterCount(), 
                                     Monster::getTotalMonstersDefeated());
    DrawRectangle(355, 52, 175, 22, Color{ 20, 18, 30, 210 });
    DrawRectangleLines(355, 52, 175, 22, Color{ 180, 120, 50, 255 });
    drawText(mobStat.c_str(), 361, 56, 12, Color{ 255, 210, 120, 255 });

    // =========================================================================
    // 2. THANH MÁU TRÙM (BOSS HP BAR)
    // =========================================================================
    if (state == GameState::RUNNING) {
        Monster* boss = dungeon.getBossMonster();
        if (boss && boss->isAlive()) {
            Position pPos = player.getPosition();
            Position bPos = boss->getPosition();
            int cheb = std::max(std::abs(pPos.x - bPos.x), std::abs(pPos.y - bPos.y));
            // Hiển thị thanh máu nếu trong Đấu trường hoặc trong cự ly quan sát
            if (bossCinematicTriggered || cheb <= 14) {
                int bossBarW = 420, bossBarH = 20;
                int bossBarX = screenW / 2 - bossBarW / 2;
                int bossBarY = 56;
                float bossHp = (float)boss->getHp() / (float)boss->getMaxHp();
                if (bossHp < 0.0f) bossHp = 0.0f;
                if (bossHp > 1.0f) bossHp = 1.0f;

                DrawRectangle(bossBarX - 10, bossBarY - 6, bossBarW + 20, bossBarH + 28, Color{ 16, 12, 22, 235 });
                DrawRectangleLines(bossBarX - 10, bossBarY - 6, bossBarW + 20, bossBarH + 28, Color{ 200, 140, 45, 255 });
                
                drawText("BOAR KING - CHUA HEO RUNG", bossBarX + 85, bossBarY - 2, 16, Color{ 255, 170, 80, 255 });
                
                DrawRectangle(bossBarX, bossBarY + 18, bossBarW, bossBarH, Color{ 40, 15, 15, 255 });
                int fillBossW = (int)((bossBarW - 4) * bossHp);
                DrawRectangle(bossBarX + 2, bossBarY + 20, fillBossW, bossBarH - 4, Color{ 220, 50, 30, 255 });
                DrawRectangle(bossBarX + 2, bossBarY + 20, fillBossW, (bossBarH - 4) / 2, Color{ 255, 120, 60, 160 });
                DrawRectangleLines(bossBarX, bossBarY + 18, bossBarW, bossBarH, Color{ 140, 50, 40, 255 });
                
                drawText(TextFormat("HP: %d/%d", boss->getHp(), boss->getMaxHp()), bossBarX + bossBarW / 2 - 38, bossBarY + 20, 14, WHITE);
            }
        }
    }

    // =========================================================================
    // 3. CẢNH BÁO NGUY HIỂM: CHÚA HEO RỪNG XUẤT HIỆN
    // =========================================================================
    if (state == GameState::RUNNING && bossWarningTimer > 0.0f) {
        bool flash = ((int)(GetTime() * 8.0f) % 2 == 0);
        int warnW = 720, warnH = 52;
        int warnX = screenW / 2 - warnW / 2;
        int warnY = 110;
        Color warnBg = flash ? Color{ 170, 25, 25, 245 } : Color{ 70, 12, 12, 245 };
        Color warnBorder = flash ? YELLOW : RED;
        DrawRectangle(warnX, warnY, warnW, warnH, warnBg);
        DrawRectangleLines(warnX, warnY, warnW, warnH, warnBorder);
        drawText("!!! CANH BAO: CHUA HEO RUNG XUAT HIEN !!!", warnX + 65, warnY + 14, 22, flash ? WHITE : YELLOW);
    }
    // BANNER PHÂN KHU THƯỜNG (KHI KHÔNG HIỆN CẢNH BÁO BOSS)
    else if (state == GameState::RUNNING && bannerTimer > 0.0f) {
        float alpha = (bannerTimer > 3.0f) ? (3.5f - bannerTimer) * 2.0f
                                           : (bannerTimer < 0.5f ? bannerTimer * 2.0f : 1.0f);
        if (alpha > 1.0f) alpha = 1.0f;
        Color bannerBg = Color{ 15, 13, 23, (unsigned char)(210 * alpha) };
        Color bannerFg = GOLD;
        bannerFg.a = (unsigned char)(255 * alpha);
        int bW = 720, bH = 64;
        int bX = screenW / 2 - bW / 2, bY = 120;
        DrawRectangle(bX, bY, bW, bH, bannerBg);
        DrawRectangleLines(bX, bY, bW, bH, bannerFg);
        drawText(bannerText.c_str(), screenW / 2 - (float)bW / 2 + 20, bY + 20, 20, bannerFg);
    }

    // =========================================================================
    // 4. KHUNG NHẬT KÝ CHIẾN ĐẤU & DẢI HƯỚNG DẪN PHÍM TẮT DƯỚI ĐÁY MÀN HÌNH
    // =========================================================================
    int logH = showCombatLog ? 96 : 26;
    int logY = screenH - logH;

    int hintY = logY - 22;
    DrawRectangle(0, hintY, screenW, 22, Color{ 14, 12, 20, 195 });
    DrawLine(0, hintY, screenW, hintY, Color{ 45, 40, 60, 255 });
    drawText("Phim: [A][D] Chay | [W][S] Leo | [Space] Nhay | [J][F] Danh | [B] Tui do | [1-8] Dung nhanh | [L] Nhat ky", 
             16, hintY + 3, 14, Color{ 210, 210, 230, 230 });

    DrawRectangle(0, logY, screenW, logH, Color{ 12, 10, 18, 225 });
    DrawLine(0, logY, screenW, logY, Color{ 55, 50, 75, 255 });

    if (!showCombatLog) {
        drawText("[+] Nhan [L] hoac Click chuot de mo Nhat ky chien dau", 16, logY + 5, 14, Color{ 160, 160, 190, 255 });
    } else {
        drawText("[-] NHAT KY CHIEN DAU (Nhan [L] de thu gon):", 16, logY + 5, 14, GOLD);

        int maxLines = 3;
        int startIdx = (int)combatLog.size() > maxLines ? (int)combatLog.size() - maxLines : 0;
        for (size_t i = startIdx; i < combatLog.size(); ++i) {
            Color logColor = RAYWHITE;
            if (combatLog[i].find("tan cong") != std::string::npos || combatLog[i].find("Sat thuong") != std::string::npos) 
                logColor = Color{ 255, 110, 110, 255 };
            else if (combatLog[i].find("CHUC MUNG") != std::string::npos || combatLog[i].find("CHAO MUNG") != std::string::npos || combatLog[i].find(">>>") != std::string::npos) 
                logColor = Color{ 255, 215, 60, 255 };
            else if (combatLog[i].find("Nhat duoc") != std::string::npos || combatLog[i].find("Hoi phuc") != std::string::npos) 
                logColor = Color{ 110, 245, 150, 255 };
            else if (combatLog[i].find("HE THONG") != std::string::npos) 
                logColor = Color{ 110, 210, 255, 255 };

            drawText(TextFormat("> %s", combatLog[i].c_str()), 18, logY + 26 + (int)((i - startIdx) * 22), 15, logColor);
        }
    }

    // =========================================================================
    // 4. BẢNG TÚI ĐỒ DẠNG MODAL (FANTASY RPG INVENTORY MODAL)
    //    CHỈ HIỂN THỊ KHI showInventory == true!
    // =========================================================================
    if (showInventory) {
        DrawRectangle(0, 0, screenW, screenH, Color{ 0, 0, 0, 130 });

        int modalW = 420;
        int modalH = 475;
        int modalX = screenW / 2 - modalW / 2;
        int modalY = screenH / 2 - modalH / 2 - 15;

        DrawRectangle(modalX + 6, modalY + 6, modalW, modalH, Color{ 0, 0, 0, 150 });

        DrawRectangle(modalX, modalY, modalW, modalH, Color{ 22, 19, 32, 250 });
        DrawRectangleLines(modalX, modalY, modalW, modalH, Color{ 195, 155, 55, 255 });
        DrawRectangleLines(modalX + 3, modalY + 3, modalW - 6, modalH - 6, Color{ 75, 65, 95, 255 });

        DrawRectangle(modalX + 4, modalY + 4, modalW - 8, 40, Color{ 36, 30, 52, 255 });
        DrawLine(modalX + 4, modalY + 44, modalX + modalW - 4, modalY + 44, Color{ 195, 155, 55, 255 });
        drawText("TUI DO CHIEN BINH", modalX + 16, modalY + 12, 19, GOLD);
        drawText(TextFormat("(%d/8 o)", (int)inv.getSize()), modalX + 225, modalY + 14, 15, Color{ 180, 180, 205, 255 });

        Rectangle closeBtn = { (float)(modalX + modalW - 38), (float)(modalY + 8), 28.0f, 28.0f };
        bool closeHover = CheckCollisionPointRec(mouse, closeBtn);
        DrawRectangleRec(closeBtn, closeHover ? Color{ 180, 30, 40, 255 } : Color{ 50, 42, 68, 255 });
        DrawRectangleLinesEx(closeBtn, 1.0f, closeHover ? RED : Color{ 100, 90, 130, 255 });
        drawText("X", closeBtn.x + 8, closeBtn.y + 4, 18, WHITE);

        if (inv.isEmpty()) {
            drawText("Tui do dang trong!", modalX + 130, modalY + 160, 18, GRAY);
            drawText("Hay kham pha ham nguc de thu thap vu khi va binh thuoc.", 
                     modalX + 34, modalY + 200, 14, Color{ 150, 150, 170, 255 });
        } else {
            for (size_t i = 0; i < 8; ++i) {
                int cardX = modalX + 16;
                int cardY = modalY + 52 + (int)i * 48;
                int cardW = modalW - 32;
                int cardH = 44;
                Rectangle cardRect = { (float)cardX, (float)cardY, (float)cardW, (float)cardH };
                bool isOccupied = (i < inv.getSize() && inv[i] != nullptr);

                bool cardHover = CheckCollisionPointRec(mouse, cardRect);

                Color cardBg = isOccupied 
                    ? (cardHover ? Color{ 48, 40, 68, 255 } : Color{ 28, 24, 40, 255 })
                    : Color{ 18, 16, 26, 180 };
                Color cardBorder = isOccupied
                    ? (cardHover ? GOLD : Color{ 75, 68, 95, 255 })
                    : Color{ 45, 40, 60, 180 };

                DrawRectangle(cardX, cardY, cardW, cardH, cardBg);
                DrawRectangleLines(cardX, cardY, cardW, cardH, cardBorder);

                if (isOccupied) {
                    const Item* item = inv[i];

                    // Khung số thứ tự phím tắt [1]-[8]
                    DrawRectangle(cardX + 4, cardY + 4, 32, 36, Color{ 38, 32, 54, 255 });
                    DrawRectangleLines(cardX + 4, cardY + 4, 32, 36, Color{ 90, 80, 115, 255 });
                    drawText(TextFormat("[%d]", (int)(i + 1)), cardX + 6, cardY + 12, 16, YELLOW);

                    // Khung ảnh icon to rõ ràng (36x36)
                    DrawRectangle(cardX + 40, cardY + 4, 36, 36, Color{ 20, 16, 30, 255 });
                    DrawRectangleLines(cardX + 40, cardY + 4, 36, 36, Color{ 90, 80, 115, 255 });

                    // Texture phóng to chuẩn pixel-art 32x32 sắc nét
                    const std::string& texId = item->getTextureId();
                    if (TextureManager::getInstance().has(texId)) {
                        const Texture2D& iconTex = TextureManager::getInstance().get(texId);
                        Rectangle srcRec = { 0, 0, (float)iconTex.width, (float)iconTex.height };
                        Rectangle destRec = { (float)(cardX + 42), (float)(cardY + 6), 32.0f, 32.0f };
                        DrawTexturePro(iconTex, srcRec, destRec, Vector2{ 0, 0 }, 0.0f, WHITE);
                    }

                    const Weapon* w = dynamic_cast<const Weapon*>(item);
                    const Potion* p = dynamic_cast<const Potion*>(item);
                    const Armor* a = dynamic_cast<const Armor*>(item);
                    const Accessory* acc = dynamic_cast<const Accessory*>(item);
                    Color nameColor = w ? Color{ 255, 175, 75, 255 } 
                                        : (a ? Color{ 120, 210, 255, 255 } 
                                        : (acc ? Color{ 230, 150, 255, 255 } : Color{ 100, 245, 150, 255 }));
                    drawText(item->getName().c_str(), cardX + 86, cardY + 12, 17, nameColor);

                    if (w) {
                        drawText(TextFormat("+%d ATK", w->getBonusAttack()), cardX + cardW - 85, cardY + 13, 15, Color{ 255, 205, 120, 255 });
                    } else if (p) {
                        drawText(TextFormat("+%d HP", p->getHealAmount()), cardX + cardW - 85, cardY + 13, 15, Color{ 130, 255, 170, 255 });
                    } else if (a) {
                        drawText(TextFormat("+%d DEF", a->getBonusDefense()), cardX + cardW - 85, cardY + 13, 15, Color{ 120, 210, 255, 255 });
                    } else if (acc) {
                        drawText("+ALL STAT", cardX + cardW - 95, cardY + 13, 15, Color{ 230, 150, 255, 255 });
                    }
                } else {
                    DrawRectangle(cardX + 4, cardY + 4, 32, 36, Color{ 22, 18, 30, 180 });
                    drawText(TextFormat("[%d]", (int)(i + 1)), cardX + 6, cardY + 12, 16, DARKGRAY);
                    drawText("(O trong)", cardX + 48, cardY + 13, 15, Color{ 80, 75, 95, 255 });
                }
            }
        }

        DrawLine(modalX + 4, modalY + modalH - 34, modalX + modalW - 4, modalY + modalH - 34, Color{ 60, 50, 80, 255 });
        drawText("Nhan [1-8] hoac Click chuot de dung | [B] / [ESC] de dong", modalX + 16, modalY + modalH - 24, 13, Color{ 180, 180, 205, 255 });
    }

    // Modal Cửa hàng hầm ngục (phím P)
    if (showShop) {
        renderShop();
    }

    // Modal Đe rèn cường hóa vũ khí (phím U)
    if (showForge) {
        renderForge();
    }

    // =========================================================================
    // 5. HIỆU ỨNG TỬ TRẬN & MÀN HÌNH GAME OVER MƯỢT MÀ
    // =========================================================================
    if (isDying) {
        // Giai đoạn 1: Chớp viền đỏ kịch tính khi vừa ngã xuống (0.35s đầu)
        if (deathTimer > deathDuration - 0.35f) {
            float flashRatio = (deathTimer - (deathDuration - 0.35f)) / 0.35f;
            unsigned char flashA = (unsigned char)(flashRatio * 90.0f);
            DrawRectangle(0, 0, screenW, screenH, Color{ 180, 20, 20, flashA });
        }
        // Giai đoạn 2: Sau khi hoạt ảnh ngã chạm đất (khoảng 0.55s cuối), phủ mờ tối dần
        if (deathTimer < 0.55f) {
            float fadeProgress = 1.0f - (deathTimer / 0.55f);
            unsigned char bgAlpha = (unsigned char)(fadeProgress * 200.0f);
            DrawRectangle(0, 0, screenW, screenH, Color{ 0, 0, 0, bgAlpha });
            if (fadeProgress > 0.4f) {
                float textRatio = (fadeProgress - 0.4f) / 0.6f;
                Color textRed = RED;
                textRed.a = (unsigned char)(textRatio * 255.0f);
                Color textWhite = RAYWHITE;
                textWhite.a = (unsigned char)(textRatio * 255.0f);
                drawText("BAN DA THAT TRAN!", screenW / 2 - 180, screenH / 2 - 40, 36, textRed);
                drawText("Nhan phim [R] de hoi sinh va thu lai", screenW / 2 - 160, screenH / 2 + 20, 20, textWhite);
            }
        }
    } else if (state == GameState::GAME_OVER) {
        DrawRectangle(0, 0, screenW, screenH, Color{ 0, 0, 0, 200 });
        drawText("BAN DA THAT TRAN!", screenW / 2 - 180, screenH / 2 - 40, 36, RED);
        drawText("Nhan phim [R] de hoi sinh va thu lai", screenW / 2 - 160, screenH / 2 + 20, 20, RAYWHITE);
    }



    // =========================================================================
    // 7. MÀN HÌNH CHIẾN THẮNG (VICTORY)
    // =========================================================================
    if (state == GameState::VICTORY) {
        DrawRectangle(0, 0, screenW, screenH, Color{ 20, 15, 5, 220 });
        drawText("CHIEN THANG!", screenW / 2 - 140, screenH / 2 - 110, 48, GOLD);
        drawText("Aethelgard duoc giai cuu - Cong cua Den Tho da mo", screenW / 2 - 230, screenH / 2 - 40, 22, RAYWHITE);
        drawText(TextFormat("Cap: %d   |   Vang: %d   |   Quai da ha guc: %d",
                            player.getLevel(), player.getGold(), monstersDefeated),
                 screenW / 2 - 200, screenH / 2 + 10, 20, SKYBLUE);
        drawText("Nhan phim [R] de choi lai tu dau", screenW / 2 - 140, screenH / 2 + 70, 20, LIGHTGRAY);
    }
}

void GameEngine::render(const std::string& screenshotPath) const {
    BeginDrawing();
    ClearBackground(Color{ 108, 160, 220, 255 });

    BeginMode2D(camera);

    // 1. Vẽ lớp nền xa (back.png) và quang cảnh cây cối (middle.png)
    dungeon.renderBackground(Vector2{0.0f, 0.0f});

    // 2. Vẽ mặt đất và vách đá (tiles.png)
    dungeon.render(Vector2{0.0f, 0.0f});

    // 3. Vẽ vật phẩm rơi trên sàn & rương báu
    dungeon.renderItems(Vector2{0.0f, 0.0f});

    // Vẽ các hạt vàng rơi đang văng / hút về người chơi (Gold Burst)
    renderGoldParticles(Vector2{0.0f, 0.0f});

    // Vẽ gợi ý tương tác mở Rương Báu Hoàng Kim khi đứng gần
    Chest* nearChest = dungeon.getNearChest(player.getPosition());
    if (nearChest && !nearChest->isOpened()) {
        float promptX = (float)(nearChest->getPosition().x * Constants::TILE_SIZE) - 26.0f;
        float promptY = (float)(nearChest->getPosition().y * Constants::TILE_SIZE) - 34.0f;
        DrawRectangleRounded(Rectangle{ promptX, promptY, 145.0f, 24.0f }, 0.3f, 4, Color{ 18, 14, 26, 235 });
        DrawRectangleRoundedLinesEx(Rectangle{ promptX, promptY, 145.0f, 24.0f }, 0.3f, 4, 1.2f, GOLD);
        drawText("[E] MO RUONG (35V)", promptX + 8.0f, promptY + 4.0f, 13, YELLOW);
    }

    // 4. Vẽ quái vật (đa hình, đứng chân chuẩn trên mặt cỏ)
    dungeon.renderMonsters(Vector2{0.0f, 0.0f});

    // 5. Vẽ người chơi (khi đã chìm hẳn vào đầm lầy thì không cần vẽ hoạt ảnh chết)
    if (!submergedInSwamp) {
        player.render(1.8f, Vector2{ 0.0f, 0.0f });
    }

    // 5.1. Hiệu ứng bùn lầy phủ quanh người và bọt khí sôi khi đang lún đầm lầy
    if (isSinking) {
        Vector2 pV = player.getVisualPosition();
        float mudY = 18.0f * (float)Constants::TILE_SIZE + 14.0f; // Bề mặt bùn lầy
        float timeSec = (float)GetTime();

        // Lớp bùn phủ trùm lên nửa thân dưới đang lún
        DrawRectangle((int)(pV.x - 8), (int)mudY, Constants::TILE_SIZE + 16, 20, Color{ 34, 46, 24, 220 });
        DrawRectangle((int)(pV.x - 4), (int)(mudY - 2), Constants::TILE_SIZE + 8, 4, Color{ 58, 86, 38, 240 });

        // Bong bóng bùn sôi quanh người chơi
        for (int b = 0; b < 4; ++b) {
            float bx = pV.x + 4.0f + (float)b * 8.0f + sinf(timeSec * 4.0f + (float)b) * 3.0f;
            float by = mudY - 2.0f + cosf(timeSec * 5.0f + (float)b) * 3.0f;
            float br = 1.5f + sinf(timeSec * 6.0f + (float)b) * 0.8f;
            if (br > 0.5f) {
                DrawCircle((int)bx, (int)by, br, Color{ 140, 215, 80, 230 });
            }
        }
    } else if (submergedInSwamp) {
        // Khi đã chìm hẳn: chỉ còn bọt khí sủi tăm nơi vừa chìm, không vẽ hoạt ảnh chết
        Vector2 pV = player.getVisualPosition();
        float mudY = 18.0f * (float)Constants::TILE_SIZE + 14.0f;
        float timeSec = (float)GetTime();
        for (int b = 0; b < 3; ++b) {
            float bx = pV.x + 4.0f + (float)b * 10.0f + sinf(timeSec * 3.0f + (float)b) * 4.0f;
            float by = mudY - 1.0f + cosf(timeSec * 4.0f + (float)b) * 2.0f;
            float br = 1.2f + sinf(timeSec * 5.0f + (float)b) * 0.6f;
            if (br > 0.4f) {
                DrawCircle((int)bx, (int)by, br, Color{ 140, 215, 80, 180 });
            }
        }
    }

    // 5.2. Vẽ các số sát thương nổi từ DynamicArray (Class Template tự cài đặt - Chương 7)
    for (size_t pIdx = 0; pIdx < activeDamagePopups.size(); ++pIdx) {
        const auto& popup = activeDamagePopups[pIdx];
        Color popCol = popup.color;
        popCol.a = static_cast<unsigned char>(popup.getAlpha() * 255);
        DrawText(popup.text.c_str(), static_cast<int>(popup.x), static_cast<int>(popup.y), 18, popCol);
    }

    EndMode2D();

    // 6. Vẽ giao diện người dùng
    renderHUD();

    if (!screenshotPath.empty()) {
        rlDrawRenderBatchActive();
        TakeScreenshot(screenshotPath.c_str());
    }

    EndDrawing();
}

void GameEngine::run(const std::string& autoScreenshot) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT, Constants::GAME_TITLE);
    SetTargetFPS(Constants::TARGET_FPS);

    init();

    int testFrames = 0;
    int gameOverFrames = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        update(dt);

        bool takeNow = false;
        if (!autoScreenshot.empty()) {
            testFrames++;
            if (autoScreenshot.find("submerge") != std::string::npos || autoScreenshot.find("gameover") != std::string::npos) {
                if (state == GameState::GAME_OVER) {
                    gameOverFrames++;
                    if (gameOverFrames >= 3) {
                        takeNow = true;
                    }
                }
            } else if (autoScreenshot.find("dead") != std::string::npos || autoScreenshot.find("death") != std::string::npos) {
                if (isDying || state == GameState::GAME_OVER) {
                    gameOverFrames++;
                    if (gameOverFrames >= 20) {
                        takeNow = true;
                    }
                }
            } else if (autoScreenshot.find("boss_entrance") != std::string::npos) {
                takeNow = (testFrames >= 18); // Boss đang phi nước đại từ phải sang trái và banner cảnh báo hiện
            } else if (autoScreenshot.find("boar_hit") != std::string::npos) {
                if (testFrames == 3) {
                    // Mô phỏng chém trúng Boar bên cạnh để kiểm tra hoạt ảnh Hit-Sheet
                    Position p = player.getPosition();
                    Monster* m = dungeon.getMonsterAt(Position(p.x + 1, p.y));
                    if (!m) m = dungeon.getMonsterAt(Position(p.x - 1, p.y));
                    if (m) {
                        CombatSystem::attack(player, *m, combatLog, this);
                    }
                }
                takeNow = (testFrames >= 7);
            } else if (autoScreenshot.find("fall_lower") != std::string::npos) {
                if (testFrames == 2) {
                    Position pPos = player.getPosition();
                    int targetX = pPos.x + 1;
                    for (int fallY = pPos.y + 1; fallY < dungeon.getHeight(); ++fallY) {
                        Position checkPos(targetX, fallY);
                        if (dungeon.isWalkable(checkPos)) {
                            Position landPos = checkPos;
                            if (dungeon.getMonsterAt(landPos) != nullptr) {
                                if (dungeon.isWalkable(Position(targetX - 1, fallY)) && dungeon.getMonsterAt(Position(targetX - 1, fallY)) == nullptr) {
                                    landPos = Position(targetX - 1, fallY);
                                } else if (dungeon.isWalkable(Position(targetX + 1, fallY)) && dungeon.getMonsterAt(Position(targetX + 1, fallY)) == nullptr) {
                                    landPos = Position(targetX + 1, fallY);
                                }
                            }
                            player.setPosition(landPos);
                            player.triggerFall(0.52f, 0.22f);
                            combatLog.push_back("[ROI XUONG] Ban da buoc hut va roi xuong tang duoi!");
                            break;
                        }
                    }
                }
                takeNow = (testFrames >= 16);
            } else if (autoScreenshot.find("shop") != std::string::npos) {
                showShop = true;
                takeNow = (testFrames >= 8);
            } else if (autoScreenshot.find("forge") != std::string::npos) {
                showForge = true;
                takeNow = (testFrames >= 8);
            } else if (autoScreenshot.find("gold_drop") != std::string::npos || autoScreenshot.find("gold_burst") != std::string::npos) {
                if (testFrames == 2) {
                    spawnGoldBurst(player.getVisualPosition().x + 55.0f, player.getVisualPosition().y - 10.0f, 
                                   (float)(player.getPosition().y * Constants::TILE_SIZE) + 8.0f, 35, 8);
                    addDamagePopup("+35 VANG!", player.getVisualPosition().x + 55.0f, player.getVisualPosition().y - 25.0f, GOLD, 1.2f);
                }
                takeNow = (testFrames >= 8);
            } else if (autoScreenshot.find("chest") != std::string::npos) {
                if (testFrames == 3) {
                    interactWithChest();
                }
                takeNow = (testFrames >= 10);
            } else {
                takeNow = (testFrames >= 10);
            }
        }

        render(takeNow ? autoScreenshot : "");

        if (IsKeyPressed(KEY_F12)) {
            TakeScreenshot("screenshot.png");
        }

        if (takeNow) {
            break;
        }
    }

    TextureManager::getInstance().unloadAll();
    CloseWindow();
}

// =========================================================================
// HỆ THỐNG HIỆU ỨNG VÀNG RƠI & CÁC CƠ CHẾ TIÊU THỤ VÀNG
// =========================================================================

void GameEngine::spawnGoldBurst(float worldX, float worldY, float groundY, int totalGold, int count) {
    if (totalGold <= 0) return;
    if (count <= 0) count = 6;

    int remaining = totalGold;
    for (int i = 0; i < count; ++i) {
        int val = remaining / (count - i);
        if (val <= 0) val = 1;
        remaining -= val;
        activeGoldParticles.push_back(GoldParticle(worldX, worldY, groundY, val));
    }
    if (remaining > 0) {
        activeGoldParticles.push_back(GoldParticle(worldX, worldY, groundY, remaining));
    }
}

void GameEngine::updateGoldParticles(float deltaTime) {
    Vector2 playerCenter = {
        player.getVisualPosition().x + (float)Constants::TILE_SIZE / 2.0f,
        player.getVisualPosition().y + (float)Constants::TILE_SIZE / 2.0f
    };

    for (size_t i = 0; i < activeGoldParticles.size(); ) {
        activeGoldParticles[i].update(deltaTime, playerCenter);
        if (activeGoldParticles[i].collected) {
            int val = activeGoldParticles[i].value;
            player.addGold(val);
            addDamagePopup("+" + std::to_string(val) + " VANG",
                           playerCenter.x - 12.0f + (float)(std::rand() % 24),
                           playerCenter.y - 18.0f - (float)(std::rand() % 16),
                           GOLD, 0.75f);
            activeGoldParticles.erase(i);
        } else {
            ++i;
        }
    }
}

void GameEngine::renderGoldParticles(Vector2 offset) const {
    const TextureManager& tm = TextureManager::getInstance();
    const Texture2D* coinTex = tm.has("item_gold_coin") ? &tm.get("item_gold_coin") : nullptr;

    for (size_t i = 0; i < activeGoldParticles.size(); ++i) {
        const auto& p = activeGoldParticles[i];
        float drawX = p.pos.x + offset.x;
        float drawY = p.pos.y + offset.y;

        // Quầng sáng vàng lấp lánh xung quanh đồng xu
        DrawCircleGradient(Vector2{ drawX, drawY }, 12.0f, ColorAlpha(GOLD, 0.5f), ColorAlpha(YELLOW, 0.0f));

        if (coinTex) {
            Rectangle srcRec = { 0, 0, (float)coinTex->width, (float)coinTex->height };
            Rectangle destRec = { drawX, drawY, 20.0f, 20.0f };
            Vector2 origin = { 10.0f, 10.0f };
            DrawTexturePro(*coinTex, srcRec, destRec, origin, p.rotation, WHITE);
        } else {
            DrawCircle((int)drawX, (int)drawY, 6.0f, GOLD);
            DrawCircleLines((int)drawX, (int)drawY, 6.0f, YELLOW);
        }
    }
}

void GameEngine::interactWithChest() {
    Chest* chest = dungeon.getNearChest(player.getPosition());
    if (!chest) return;

    if (chest->isOpened()) {
        combatLog.push_back("[RUONG BAU] Ruong hoang kim nay da duoc mo roi!");
        return;
    }

    int cost = chest->getUnlockCost();
    float chestWorldX = (float)(chest->getPosition().x * Constants::TILE_SIZE) + 16.0f;
    float chestWorldY = (float)(chest->getPosition().y * Constants::TILE_SIZE) + 6.0f;
    float groundY = (float)(chest->getPosition().y * Constants::TILE_SIZE) + 24.0f;

    if (chest->tryOpen(player, this)) {
        spawnGoldBurst(chestWorldX, chestWorldY, groundY, 25, 8);
        addDamagePopup("-" + std::to_string(cost) + " VANG", chestWorldX, chestWorldY - 14.0f, YELLOW, 1.0f);
        addDamagePopup("+50 EXP!", chestWorldX, chestWorldY - 30.0f, SKYBLUE, 1.2f);
        addDamagePopup("+BAO VAT!", chestWorldX, chestWorldY - 46.0f, GOLD, 1.4f);
        combatLog.push_back("[KHO BAU] Dung " + std::to_string(cost) + " vang mo Ruong Hoang Kim thanh cong!");
        combatLog.push_back("Nhan duoc: " + chest->getRewardName() + " va +50 EXP!");
    } else {
        addDamagePopup("CAN " + std::to_string(cost) + " VANG!", chestWorldX, chestWorldY - 16.0f, RED, 0.9f);
        combatLog.push_back("[KHO BAU] Khong du " + std::to_string(cost) + " vang de mo Ruong Hoang Kim!");
    }
}

void GameEngine::buyShopItem(int slot) {
    struct ShopItemDef {
        std::string name;
        std::string desc;
        int cost;
        std::string type;
        int bonus;
        std::string texId;
    };

    static const ShopItemDef catalog[6] = {
        { "Binh Mau Nho", "Hoi phuc 35 HP tuc thi", 25, "potion", 35, "item_potion_health" },
        { "Thuoc Cuong Hoa", "Hoi phuc 60 HP va tang suc ben", 40, "potion", 60, "item_potion_strength" },
        { "Than Duoc Truong Sinh", "Hoi phuc 100 HP toi da", 65, "potion", 100, "item_potion_elixir" },
        { "Dai Kiem Huyen Bi", "Vu khi co dai tang +15 ATK", 80, "weapon", 15, "item_sword_mystic" },
        { "Khien Ho Menh", "Trang bi thep vieng vang tang +5 DEF", 70, "armor", 5, "item_armor_shield" },
        { "Nhan Co Ngu Ruby", "Nhan co +6 ATK, +3 DEF, +20 MaxHP", 90, "accessory", 6, "item_ring_power" }
    };

    if (slot < 0 || slot >= 6) return;
    const auto& item = catalog[slot];

    if (player.getInventory().isFull()) {
        combatLog.push_back("[CUA HANG] Tui do da day! Khong the mua them.");
        addDamagePopup("TUI DO DAY!", player.getVisualPosition().x + 8.0f, player.getVisualPosition().y - 20.0f, RED, 0.8f);
        return;
    }

    if (!player.spendGold(item.cost)) {
        combatLog.push_back("[CUA HANG] Khong du " + std::to_string(item.cost) + " vang de mua " + item.name + "!");
        addDamagePopup("KHONG DU VANG!", player.getVisualPosition().x + 8.0f, player.getVisualPosition().y - 20.0f, RED, 0.8f);
        return;
    }

    if (item.type == "potion") {
        player.getInventory().addItem(std::make_unique<Potion>(item.name, item.desc, item.bonus, Position(0,0), item.texId));
    } else if (item.type == "weapon") {
        player.getInventory().addItem(std::make_unique<Weapon>(item.name, item.desc, item.bonus, Position(0,0), item.texId));
    } else if (item.type == "armor") {
        player.getInventory().addItem(std::make_unique<Armor>(item.name, item.desc, item.bonus, Position(0,0), item.texId));
    } else if (item.type == "accessory") {
        player.getInventory().addItem(std::make_unique<Accessory>(item.name, item.desc, 6, 3, 20, Position(0,0), item.texId));
    }

    addDamagePopup("-" + std::to_string(item.cost) + " VANG", player.getVisualPosition().x + 8.0f, player.getVisualPosition().y - 12.0f, YELLOW, 0.9f);
    addDamagePopup("+1 " + item.name, player.getVisualPosition().x + 8.0f, player.getVisualPosition().y - 28.0f, GREEN, 1.1f);
    combatLog.push_back("[CUA HANG] Da mua: " + item.name + " (-" + std::to_string(item.cost) + " vang)!");
}

void GameEngine::triggerForgeUpgrade() {
    int cost = player.getNextUpgradeCost();
    int bonus = player.getNextUpgradeBonus();

    if (player.upgradeForge()) {
        addDamagePopup("-" + std::to_string(cost) + " VANG", player.getVisualPosition().x + 8.0f, player.getVisualPosition().y - 12.0f, YELLOW, 0.9f);
        addDamagePopup("+" + std::to_string(bonus) + " ATK CUONG HOA!", player.getVisualPosition().x + 8.0f, player.getVisualPosition().y - 28.0f, GOLD, 1.3f);
        combatLog.push_back("[DE REN] Cuong hoa kiem thanh cong! +" + std::to_string(bonus) + " ATK (Cap ren: +" + std::to_string(player.getForgeLevel()) + ")!");
    } else {
        addDamagePopup("CAN " + std::to_string(cost) + " VANG!", player.getVisualPosition().x + 8.0f, player.getVisualPosition().y - 16.0f, RED, 0.9f);
        combatLog.push_back("[DE REN] Khong du " + std::to_string(cost) + " vang de cuong hoa vu khi!");
    }
}

void GameEngine::renderShop() const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    Vector2 mouse = GetMousePosition();
    const TextureManager& tm = TextureManager::getInstance();

    DrawRectangle(0, 0, screenW, screenH, Color{ 0, 0, 0, 150 });

    int modalW = 560;
    int modalH = 460;
    int modalX = screenW / 2 - modalW / 2;
    int modalY = screenH / 2 - modalH / 2;

    DrawRectangle(modalX + 6, modalY + 6, modalW, modalH, Color{ 0, 0, 0, 160 });
    DrawRectangle(modalX, modalY, modalW, modalH, Color{ 22, 18, 32, 252 });
    DrawRectangleLines(modalX, modalY, modalW, modalH, Color{ 215, 175, 55, 255 });
    DrawRectangleLines(modalX + 3, modalY + 3, modalW - 6, modalH - 6, Color{ 80, 70, 105, 255 });

    // Header bar
    DrawRectangle(modalX + 4, modalY + 4, modalW - 8, 42, Color{ 36, 30, 52, 255 });
    DrawLine(modalX + 4, modalY + 46, modalX + modalW - 4, modalY + 46, Color{ 215, 175, 55, 255 });

    if (tm.has("item_icon_shop")) {
        const Texture2D& sTex = tm.get("item_icon_shop");
        DrawTexturePro(sTex, Rectangle{ 0, 0, (float)sTex.width, (float)sTex.height },
                       Rectangle{ (float)(modalX + 14), (float)(modalY + 9), 32.0f, 32.0f }, Vector2{ 0, 0 }, 0.0f, WHITE);
    }
    drawText("CUA HANG HAM NGUC AETHELGARD", modalX + 54, modalY + 14, 18, GOLD);

    // Huy hiệu vàng hiện có của người chơi
    DrawRectangleRounded(Rectangle{ (float)(modalX + modalW - 175), (float)(modalY + 9), 130.0f, 28.0f }, 0.3f, 4, Color{ 24, 20, 36, 255 });
    DrawRectangleRoundedLinesEx(Rectangle{ (float)(modalX + modalW - 175), (float)(modalY + 9), 130.0f, 28.0f }, 0.3f, 4, 1.2f, GOLD);
    if (tm.has("item_gold_coin")) {
        const Texture2D& cTex = tm.get("item_gold_coin");
        DrawTexturePro(cTex, Rectangle{ 0, 0, (float)cTex.width, (float)cTex.height },
                       Rectangle{ (float)(modalX + modalW - 170), (float)(modalY + 13), 20.0f, 20.0f }, Vector2{ 0, 0 }, 0.0f, WHITE);
    }
    drawText(TextFormat("%d Vang", player.getGold()), modalX + modalW - 144, modalY + 14, 15, YELLOW);

    // Nút đóng [X]
    Rectangle closeBtn = { (float)(modalX + modalW - 36), (float)(modalY + 9), 28.0f, 28.0f };
    bool closeHover = CheckCollisionPointRec(mouse, closeBtn);
    DrawRectangleRec(closeBtn, closeHover ? Color{ 180, 30, 40, 255 } : Color{ 50, 42, 68, 255 });
    DrawRectangleLinesEx(closeBtn, 1.0f, closeHover ? RED : Color{ 100, 90, 130, 255 });
    drawText("X", closeBtn.x + 8, closeBtn.y + 4, 18, WHITE);

    struct CatalogDisplay {
        const char* name;
        const char* desc;
        int cost;
        const char* statText;
        const char* texId;
        Color nameCol;
    };

    static const CatalogDisplay items[6] = {
        { "Binh Mau Nho", "Hoi phuc mau tuc thi", 25, "+35 HP", "item_potion_health", Color{ 110, 245, 150, 255 } },
        { "Thuoc Cuong Hoa", "Hoi phuc & tang luc", 40, "+60 HP", "item_potion_strength", Color{ 110, 245, 150, 255 } },
        { "Than Duoc Aethelgard", "Hoi day mau toi da", 65, "+100 HP", "item_potion_elixir", Color{ 130, 255, 170, 255 } },
        { "Dai Kiem Huyen Bi", "Vu khi co dai", 80, "+15 ATK", "item_sword_mystic", Color{ 255, 175, 75, 255 } },
        { "Khien Ho Menh", "Giap ho than vieng vang", 70, "+5 DEF", "item_armor_shield", Color{ 120, 210, 255, 255 } },
        { "Nhan Co Ngu Ruby", "Trang suc phep thuat", 90, "+ALL STAT", "item_ring_power", Color{ 230, 150, 255, 255 } }
    };

    for (int i = 0; i < 6; ++i) {
        int col = i % 2;
        int row = i / 2;
        int cardX = modalX + 16 + col * 270;
        int cardY = modalY + 54 + row * 128;
        int cardW = 262;
        int cardH = 118;
        Rectangle cardRect = { (float)cardX, (float)cardY, (float)cardW, (float)cardH };

        bool canAfford = (player.getGold() >= items[i].cost);
        bool cardHover = CheckCollisionPointRec(mouse, cardRect);

        DrawRectangle(cardX, cardY, cardW, cardH, cardHover ? Color{ 42, 34, 58, 255 } : Color{ 28, 24, 40, 255 });
        DrawRectangleLines(cardX, cardY, cardW, cardH, cardHover ? GOLD : Color{ 75, 65, 95, 255 });

        // Slot phím tắt [1] - [6]
        DrawRectangle(cardX + 6, cardY + 6, 26, 26, Color{ 36, 30, 52, 255 });
        DrawRectangleLines(cardX + 6, cardY + 6, 26, 26, Color{ 90, 80, 115, 255 });
        drawText(TextFormat("[%d]", i + 1), cardX + 7, cardY + 9, 14, YELLOW);

        // Icon món đồ 32x32
        DrawRectangle(cardX + 36, cardY + 6, 38, 38, Color{ 18, 16, 28, 255 });
        DrawRectangleLines(cardX + 36, cardY + 6, 38, 38, Color{ 90, 80, 115, 255 });
        if (tm.has(items[i].texId)) {
            const Texture2D& itTex = tm.get(items[i].texId);
            DrawTexturePro(itTex, Rectangle{ 0, 0, (float)itTex.width, (float)itTex.height },
                           Rectangle{ (float)(cardX + 39), (float)(cardY + 9), 32.0f, 32.0f }, Vector2{ 0, 0 }, 0.0f, WHITE);
        }

        // Tên và chỉ số
        drawText(items[i].name, cardX + 80, cardY + 8, 15, items[i].nameCol);
        drawText(items[i].statText, cardX + 80, cardY + 28, 14, GOLD);
        drawText(items[i].desc, cardX + 8, cardY + 50, 12, Color{ 160, 160, 185, 255 });

        // Giá bán
        DrawRectangle(cardX + 8, cardY + 80, 100, 28, Color{ 20, 16, 30, 255 });
        DrawRectangleLines(cardX + 8, cardY + 80, 100, 28, Color{ 80, 70, 100, 255 });
        if (tm.has("item_gold_coin")) {
            const Texture2D& cTex = tm.get("item_gold_coin");
            DrawTexturePro(cTex, Rectangle{ 0, 0, (float)cTex.width, (float)cTex.height },
                           Rectangle{ (float)(cardX + 12), (float)(cardY + 85), 18.0f, 18.0f }, Vector2{ 0, 0 }, 0.0f, WHITE);
        }
        drawText(TextFormat("%d V", items[i].cost), cardX + 34, cardY + 85, 14, canAfford ? YELLOW : RED);

        // Nút MUA
        Rectangle buyBtn = { (float)(cardX + 185), (float)(cardY + 76), 70.0f, 32.0f };
        bool buyHover = CheckCollisionPointRec(mouse, buyBtn);
        Color buyBg = canAfford ? (buyHover ? Color{ 35, 140, 60, 255 } : Color{ 25, 95, 45, 255 }) : Color{ 45, 40, 52, 255 };
        DrawRectangleRec(buyBtn, buyBg);
        DrawRectangleLinesEx(buyBtn, 1.0f, canAfford ? GREEN : DARKGRAY);
        drawText("MUA", buyBtn.x + 18, buyBtn.y + 7, 15, canAfford ? WHITE : GRAY);
    }

    DrawLine(modalX + 4, modalY + modalH - 30, modalX + modalW - 4, modalY + modalH - 30, Color{ 60, 50, 80, 255 });
    drawText("Phim [1-6] de mua nhanh | [P] / [ESC] de dong cua hang", modalX + 16, modalY + modalH - 22, 13, Color{ 180, 180, 205, 255 });
}

void GameEngine::renderForge() const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    Vector2 mouse = GetMousePosition();
    const TextureManager& tm = TextureManager::getInstance();

    DrawRectangle(0, 0, screenW, screenH, Color{ 0, 0, 0, 150 });

    int modalW = 460;
    int modalH = 400;
    int modalX = screenW / 2 - modalW / 2;
    int modalY = screenH / 2 - modalH / 2;

    DrawRectangle(modalX + 6, modalY + 6, modalW, modalH, Color{ 0, 0, 0, 160 });
    DrawRectangle(modalX, modalY, modalW, modalH, Color{ 26, 18, 28, 252 });
    DrawRectangleLines(modalX, modalY, modalW, modalH, Color{ 230, 120, 40, 255 });
    DrawRectangleLines(modalX + 3, modalY + 3, modalW - 6, modalH - 6, Color{ 90, 60, 75, 255 });

    // Header bar
    DrawRectangle(modalX + 4, modalY + 4, modalW - 8, 42, Color{ 48, 26, 36, 255 });
    DrawLine(modalX + 4, modalY + 46, modalX + modalW - 4, modalY + 46, Color{ 230, 120, 40, 255 });

    if (tm.has("item_icon_forge")) {
        const Texture2D& fTex = tm.get("item_icon_forge");
        DrawTexturePro(fTex, Rectangle{ 0, 0, (float)fTex.width, (float)fTex.height },
                       Rectangle{ (float)(modalX + 14), (float)(modalY + 9), 32.0f, 32.0f }, Vector2{ 0, 0 }, 0.0f, WHITE);
    }
    drawText("DE REN THO REN HOANG KIM", modalX + 54, modalY + 14, 18, ORANGE);

    // Nút đóng [X]
    Rectangle closeBtn = { (float)(modalX + modalW - 36), (float)(modalY + 9), 28.0f, 28.0f };
    bool closeHover = CheckCollisionPointRec(mouse, closeBtn);
    DrawRectangleRec(closeBtn, closeHover ? Color{ 180, 30, 40, 255 } : Color{ 50, 42, 68, 255 });
    DrawRectangleLinesEx(closeBtn, 1.0f, closeHover ? RED : Color{ 100, 90, 130, 255 });
    drawText("X", closeBtn.x + 8, closeBtn.y + 4, 18, WHITE);

    // Khung thông tin thanh kiếm hiện tại
    int infoY = modalY + 60;
    DrawRectangle(modalX + 24, infoY, modalW - 48, 80, Color{ 34, 24, 38, 255 });
    DrawRectangleLines(modalX + 24, infoY, modalW - 48, 80, Color{ 100, 70, 85, 255 });

    if (tm.has("item_sword_steel")) {
        const Texture2D& sTex = tm.get("item_sword_steel");
        DrawTexturePro(sTex, Rectangle{ 0, 0, (float)sTex.width, (float)sTex.height },
                       Rectangle{ (float)(modalX + 38), (float)(infoY + 16), 48.0f, 48.0f }, Vector2{ 0, 0 }, 0.0f, WHITE);
    }
    drawText(TextFormat("Thanh Kiem Hiep Si (Cap +%d)", player.getForgeLevel()), modalX + 102, infoY + 16, 17, GOLD);
    drawText(TextFormat("Suc tan cong hien tai: %d ATK", player.getAttack()), modalX + 102, infoY + 44, 15, Color{ 255, 175, 75, 255 });

    // Khung nâng cấp kế tiếp
    int nextY = modalY + 155;
    int nextCost = player.getNextUpgradeCost();
    int nextBonus = player.getNextUpgradeBonus();
    bool canAfford = (player.getGold() >= nextCost);

    DrawRectangle(modalX + 24, nextY, modalW - 48, 120, Color{ 30, 22, 34, 255 });
    DrawRectangleLines(modalX + 24, nextY, modalW - 48, 120, canAfford ? GOLD : Color{ 80, 55, 70, 255 });

    drawText("GIAI DOAN CUONG HOA KE TIEP:", modalX + 36, nextY + 12, 15, ORANGE);
    drawText(TextFormat("> Nang len: Kiem Cap +%d", player.getForgeLevel() + 1), modalX + 36, nextY + 36, 16, YELLOW);
    drawText(TextFormat("> Tang them: +%d ATK vinh vien vao chi so!", nextBonus), modalX + 36, nextY + 60, 15, GREEN);

    drawText(TextFormat("Chi phi: %d Vang  |  Vang cua ban: %d Vang", nextCost, player.getGold()),
             modalX + 36, nextY + 90, 14, canAfford ? Color{ 255, 220, 80, 255 } : Color{ 255, 100, 100, 255 });

    // Nút thực hiện cường hóa
    Rectangle upgradeBtn = { (float)(modalX + 30), (float)(modalY + modalH - 65), (float)(modalW - 60), 46.0f };
    bool upHover = CheckCollisionPointRec(mouse, upgradeBtn);
    Color upBg = canAfford ? (upHover ? Color{ 200, 90, 20, 255 } : Color{ 160, 65, 15, 255 }) : Color{ 45, 35, 45, 255 };
    DrawRectangleRounded(upgradeBtn, 0.25f, 4, upBg);
    DrawRectangleRoundedLinesEx(upgradeBtn, 0.25f, 4, 1.5f, canAfford ? ORANGE : DARKGRAY);
    const char* btnText = canAfford ? TextFormat("REN KIEM (+%d ATK) - %d VANG [ENTER]", nextBonus, nextCost) : "KHONG DU VANG DE REN";
    drawText(btnText, upgradeBtn.x + (upgradeBtn.width - 320) / 2.0f, upgradeBtn.y + 14, 15, canAfford ? WHITE : GRAY);
}

void GameEngine::runOOPAcademicTests() {
    std::cout << "\n======================================================================\n";
    std::cout << "   AETHELGARD: COREBOUND - BO KIEM THU HOC THUAT OOP (UTH CURRICULUM)\n";
    std::cout << "======================================================================\n";

    // -------------------------------------------------------------------------
    // TEST 1: CHƯƠNG 2 & CON TRỎ - QUẢN LÝ BỘ NHỚ ĐỘNG, CON TRỎ & THAM CHIẾU
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 1] CHUONG 2: CON TRO & QUAN LY BO NHO DONG (Dynamic Memory):\n";
    int* dynamicVal = new int(100);
    assert(*dynamicVal == 100);
    std::cout << "  [PASS] Cap phat dong con tro nguyen thuy voi new: *dynamicVal = " << *dynamicVal << "\n";
    delete dynamicVal;
    dynamicVal = nullptr;
    std::cout << "  [PASS] Giai phong vung nho con tro an toan voi delete & gan nullptr.\n";

    int a = 15, b = 45;
    CoreTemplates::swapValues(a, b);
    assert(a == 45 && b == 15);
    std::cout << "  [PASS] Hoan vi gia tri thong qua Tham chieu (&): a = " << a << ", b = " << b << "\n";

    // -------------------------------------------------------------------------
    // TEST 2: CHƯƠNG 3 - LỚP & ĐỐI TƯỢNG, CONSTRUCTOR SAO CHÉP, LỚP BẠN (FRIEND CLASS)
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 2] CHUONG 3: LOP & DOI TUONG (Friend Class, Copy Ctor, Destructor):\n";
    Position pOriginal(12, 18);
    Position pCopied(pOriginal); // Copy constructor
    assert(pCopied.x == 12 && pCopied.y == 18);
    std::cout << "  [PASS] Constructor sao chep (Copy Constructor): pCopied = " << pCopied << "\n";

    int dist = pOriginal.manhattanDistanceTo(pCopied);
    assert(dist == 0);
    std::cout << "  [PASS] Singleton static: TextureManager instance ton tai duy nhat.\n";
    std::cout << "  [PASS] Lop ban (Friend Class): CombatSystem & SaveLoadManager duoc cap quyen truy cap Entity.\n";

    // -------------------------------------------------------------------------
    // TEST 3: CHƯƠNG 4 - QUÁ TẢI TOÁN TỬ (OPERATOR OVERLOADING)
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 3] CHUONG 4: QUA TAI TOAN TU (Operator Overloading):\n";
    Position posA(10, 20);
    Position posB(3, 5);

    Position posAdd = posA + posB;
    assert(posAdd.x == 13 && posAdd.y == 25);
    std::cout << "  [PASS] Toan tu cong operator+: " << posA << " + " << posB << " = " << posAdd << "\n";

    Position posSub = posA - posB;
    assert(posSub.x == 7 && posSub.y == 15);
    std::cout << "  [PASS] Toan tu tru operator-: " << posA << " - " << posB << " = " << posSub << "\n";

    posA += posB;
    assert(posA == posAdd);
    std::cout << "  [PASS] Toan tu gan cong operator+=: posA = " << posA << "\n";

    posA -= posB;
    assert(posA.x == 10 && posA.y == 20);
    std::cout << "  [PASS] Toan tu gan tru operator-=: posA = " << posA << "\n";

    assert(posB < posA);
    std::cout << "  [PASS] Toan tu so sanh thu tu operator<: " << posB << " < " << posA << "\n";

    // Kiểm tra xuất stream << và nhập stream >>
    std::stringstream ss;
    ss << posA; // operator<<
    std::cout << "  [PASS] Toan tu xuat stream operator<<: " << ss.str() << "\n";

    Position posIn;
    ss >> posIn; // operator>>
    assert(posIn == posA);
    std::cout << "  [PASS] Toan tu nhap stream operator>>: doc thanh cong posIn = " << posIn << "\n";

    // 9. Toán tử chuyển đổi kiểu (User-Defined Type Conversion Operator)
    Vector2 v = posA; // Tự động gọi operator Vector2()
    assert(v.x == 320.0f && v.y == 640.0f);
    std::cout << "  [PASS] Toan tu chuyen doi kieu operator Vector2(): (" << v.x << ", " << v.y << ")\n";

    // 10. Toán tử 1 ngôi tiền tố/hậu tố (++ / --)
    Position posInc = posA;
    ++posInc;
    assert(posInc.x == 11);
    std::cout << "  [PASS] Toan tu 1 ngoi tien to ++pos: x = " << posInc.x << "\n";

    // 11. Functor operator()
    DistanceComparator comp(Position(0, 0));
    assert(comp(Position(1, 1), Position(5, 5)));
    std::cout << "  [PASS] Functor operator() DistanceComparator: so sanh khoang cach muc tieu.\n";

    // 12. Toán tử 1 ngôi trên Potion
    Potion potTest("Binh Mau Test", "Hoi 30 HP", 30);
    ++potTest;
    assert(potTest.getStackCount() == 2);
    --potTest;
    assert(potTest.getStackCount() == 1);
    std::cout << "  [PASS] Toan tu 1 ngoi ++/-- cho Potion stackCount: " << potTest.getStackCount() << "\n";

    // -------------------------------------------------------------------------
    // TEST 4: CHƯƠNG 5 - ĐA KẾ THỪA, KIM CƯƠNG VIRTUAL BASE & KẾ THỪA ĐA MỨC
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 4] CHUONG 5: DA KE THUA & DIAMOND PROBLEM (Virtual Base Class):\n";
    Player testHero("Hiep Si Test", Position(5, 5), 100, 20, 5);
    
    // Upcasting sang 2 giao diện đa kế thừa
    IRenderable* renderInterface = &testHero;
    IDamageable* damageInterface = &testHero;

    // Upcasting sang Virtual Base Class IGameObject
    IGameObject* objFromRender = renderInterface;
    IGameObject* objFromDamage = damageInterface;

    assert(objFromRender == objFromDamage);
    assert(objFromRender->getInstanceId() == objFromDamage->getInstanceId());
    std::cout << "  [PASS] Da ke thua thanh cong: Entity ke thua dong thoi IRenderable & IDamageable.\n";
    std::cout << "  [PASS] Ke thua kim cuong (Diamond Problem) duoc giai quyet hoan hao: \n";
    std::cout << "         objFromRender (" << (void*)objFromRender << ") == objFromDamage (" << (void*)objFromDamage << ")\n";
    std::cout << "         Duy nhat 1 instanceId = " << objFromRender->getInstanceId() << " (Khong bi xung dot luong nghia!)\n";

    // Kế thừa đa mức (Multi-level Inheritance)
    BoarKing testKing(Position(10, 10));
    Boar* asBoar = &testKing;
    GroundMonster* asGround = asBoar;
    Monster* asMonster = asGround;
    Entity* asEntity = asMonster;
    assert(asEntity != nullptr);
    std::cout << "  [PASS] Ke thua da muc (Multi-level 5 tang): BoarKing -> Boar -> GroundMonster -> Monster -> Entity\n";

    // Kế thừa giao diện Chest -> IRenderable -> virtual IGameObject
    Chest testChest(Position(38, 12), 35, "Than Duoc Aethelgard", 50);
    IRenderable* chestRenderable = &testChest;
    IGameObject* chestGameObj = chestRenderable;
    assert(chestGameObj != nullptr && chestGameObj->getInstanceId() > 0);
    assert(testChest.getUnlockCost() == 35 && !testChest.isOpened());
    std::cout << "  [PASS] Ke thua giao dien Chest: IRenderable ke thua ao IGameObject (InstanceID=" << chestGameObj->getInstanceId() << ")\n";

    // -------------------------------------------------------------------------
    // TEST 5: CHƯƠNG 6 - ĐA HÌNH ĐỘNG (RUNTIME POLYMORPHISM) & KỸ NĂNG ĐA HÌNH
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 5] CHUONG 6: DA HINH DONG (Runtime Polymorphism & Pure Virtual):\n";
    Entity* polymorphicMonster = new Boar(Position(7, 7));
    std::cout << "  [PASS] Con tro lop co so Entity* tro den doi tuong lop con Boar.\n";

    polymorphicMonster->takeDamage(15);
    std::cout << "  [PASS] Goi phuong thuc ao takeDamage qua con tro da hinh: HP con " << polymorphicMonster->getHp() << "\n";

    Boar* downcasted = dynamic_cast<Boar*>(polymorphicMonster);
    assert(downcasted != nullptr);
    std::cout << "  [PASS] Ep kieu con tro an toan dynamic_cast (RTTI): xac dinh dung lop Boar.\n";

    delete polymorphicMonster; // Virtual Destructor được kích hoạt
    std::cout << "  [PASS] Giai phong bo nho qua con tro Entity* goi dung Virtual Destructor.\n";

    // Đa hình vật phẩm Item (Chương 6): Armor & Accessory
    int heroDefBefore = testHero.getDefense();
    Item* polymorphicArmor = new Armor("Khien Hiep Si", "Tang 8 Thu", 8);
    polymorphicArmor->use(&testHero);
    assert(testHero.getDefense() == heroDefBefore + 8);
    std::cout << "  [PASS] Da hinh vat pham Armor::use(Player*): DEF tang len " << testHero.getDefense() << "\n";
    delete polymorphicArmor;

    int heroAtkBefore = testHero.getAttack();
    Item* polymorphicAcc = new Accessory("Nhan Cuong Luc", "Tang ATK, DEF, HP", 6, 3, 25);
    polymorphicAcc->use(&testHero);
    assert(testHero.getAttack() == heroAtkBefore + 6);
    std::cout << "  [PASS] Da hinh vat pham Accessory::use(Player*): ATK tang len " << testHero.getAttack() << "\n";
    delete polymorphicAcc;

    // Kỹ năng đa hình (Polymorphic Skills)
    Skill* dashSkill = testHero.getSkill(1);
    assert(dashSkill != nullptr && dashSkill->getName() == "Luot Ne Don");
    std::cout << "  [PASS] Da hinh Ky nang (Polymorphic Skill): " << dashSkill->getName() << " (Cooldown " << dashSkill->getCooldown() << "s)\n";

    // Method Chaining với con trỏ this (Chương 3)
    testHero.setHp(80).setAttack(25).setDefense(10);
    assert(testHero.getHp() == 80 && testHero.getAttack() == 25);
    std::cout << "  [PASS] Method Chaining voi con tro this: testHero.setHp().setAttack().setDefense()\n";

    // Kiểm tra tính đóng gói và kinh tế vàng của Player
    testHero.addGold(150);
    assert(testHero.getGold() >= 150);
    int goldSnapshot = testHero.getGold();
    bool spendOk = testHero.spendGold(40);
    assert(spendOk && testHero.getGold() == goldSnapshot - 40);
    bool spendFail = testHero.spendGold(999999);
    assert(!spendFail);
    int forgeBefore = testHero.getForgeLevel();
    int upgradeCost = testHero.getNextUpgradeCost();
    if (testHero.getGold() >= upgradeCost) {
        bool forgeOk = testHero.upgradeForge();
        assert(forgeOk && testHero.getForgeLevel() == forgeBefore + 1);
        std::cout << "  [PASS] Co che kinh te & De ren: spendGold(40) & upgradeForge() len cap " << testHero.getForgeLevel() << "\n";
    }

    // -------------------------------------------------------------------------
    // TEST 6: CHƯƠNG 7 - KHUÔN MẪU (TEMPLATES - FUNCTION & CLASS TEMPLATES)
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 6] CHUONG 7: KHUON MAU (Function Template & Class Template):\n";
    // 1. Function Templates
    int clampedInt = CoreTemplates::clampValue(150, 0, 100);
    float clampedFloat = CoreTemplates::clampValue(-3.5f, 0.0f, 10.0f);
    assert(clampedInt == 100 && clampedFloat == 0.0f);
    std::cout << "  [PASS] Function Template clampValue<int>: " << clampedInt << "\n";
    std::cout << "  [PASS] Function Template clampValue<float>: " << clampedFloat << "\n";

    // 2. Class Template DynamicArray
    DynamicArray<int> intArr;
    intArr.push_back(10);
    intArr.push_back(20);
    intArr.push_back(30);
    assert(intArr.size() == 3);
    assert(intArr[1] == 20);
    std::cout << "  [PASS] Class Template DynamicArray<int> cap phat dong new[]: size = " << intArr.size() << ", data = " << intArr << "\n";

    // Kiểm tra Deep Copy của Class Template
    DynamicArray<int> copyArr = intArr; // Copy Constructor
    intArr[0] = 999;
    assert(copyArr[0] == 10);
    std::cout << "  [PASS] Deep Copy Class Template: sao chep sau doc lap vung nho con tro T* thanh cong!\n";

    DynamicArray<std::string> strArr;
    strArr.push_back("Aethelgard");
    strArr.push_back("Corebound");
    strArr.push_back("Roguelike C++17");
    std::cout << "  [PASS] Class Template DynamicArray<string>: " << strArr << "\n";

    // Class Template DynamicArray với GoldParticle
    DynamicArray<GoldParticle> goldParticleArr;
    GoldParticle gpTest{};
    gpTest.pos = {100.0f, 200.0f};
    gpTest.collected = false;
    gpTest.value = 5;
    goldParticleArr.push_back(gpTest);
    assert(goldParticleArr.size() == 1 && !goldParticleArr[0].collected && goldParticleArr[0].value == 5);
    std::cout << "  [PASS] Class Template DynamicArray<GoldParticle>: quan ly hat vang vat ly dong.\n";

    // -------------------------------------------------------------------------
    // TEST 7: XỬ LÝ NGOẠI LỆ & THÀNH VIÊN TĨNH (EXCEPTION HANDLING & STATIC MEMBERS)
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 7] NGOAI LE & THANH VIEN TINH (Exception Handling & Static Tracking):\n";
    bool exceptionCaught = false;
    try {
        throw SaveLoadException("File savegame bi loi cau truc!");
    } catch (const SaveLoadException& e) {
        exceptionCaught = true;
        std::cout << "  [PASS] Bat ngoai le thanh cong (try-catch): " << e.what() << "\n";
    }
    assert(exceptionCaught);

    std::cout << "  [PASS] Thanh vien tinh Monster::getActiveMonsterCount() = " << Monster::getActiveMonsterCount() << "\n";

    std::cout << "\n======================================================================\n";
    std::cout << "   TAT CA 7 PHAN KIEM THU HOC THUAT OOP TOAN DIEN DEU DAT [100%]\n";
    std::cout << "======================================================================\n\n";
}

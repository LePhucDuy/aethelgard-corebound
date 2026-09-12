#include "engine/GameEngine.h"
#include "graphics/TextureManager.h"
#include "systems/CombatSystem.h"
#include "systems/SaveLoadManager.h"
#include "items/Potion.h"
#include "items/Weapon.h"
#include "entities/Snail.h"
#include "core/Constants.h"
#include <iostream>
#include <cmath>
#include <algorithm>

GameEngine::GameEngine(int spawnX, int spawnY, bool startWithInventory)
    : player("Hiep Si Aethelgard", Position(4, 17), 100, 16, 5),
      dungeon(Constants::DUNGEON_WIDTH, Constants::DUNGEON_HEIGHT),
      state(GameState::RUNNING),
      moveTimer(0.0f),
      attackTimer(0.0f),
      userZoomOffset(0.0f),
      currentZone(-1),
      bannerText(""),
      bannerTimer(0.0f),
      monstersDefeated(0),
      showInventory(startWithInventory),
      showCombatLog(true),
      spawnOverrideX(spawnX),
      spawnOverrideY(spawnY) {
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
    tm.load("boar_walk",      "assets/mobs/boar/Walk/Walk-Base-Sheet.png");
    tm.load("boar_idle",      "assets/mobs/boar/Idle/Idle-Sheet.png");
    tm.load("boar_run",       "assets/mobs/boar/Run/Run-Sheet.png");
    tm.load("boar_hit",       "assets/mobs/boar/Hit-Vanish/Hit-Sheet.png");
    tm.load("bee_fly",        "assets/mobs/small_bee/Fly/Fly-Sheet.png");
    tm.load("bee_attack",     "assets/mobs/small_bee/Attack/Attack-Sheet.png");
    tm.load("bee_hit",        "assets/mobs/small_bee/Hit/Hit-Sheet.png");
    tm.load("snail_walk",     "assets/mobs/snail/walk-Sheet.png");
    tm.load("snail_hide",     "assets/mobs/snail/Hide-Sheet.png");
    tm.load("snail_dead",     "assets/mobs/snail/Dead-Sheet.png");

    // Nạp toàn bộ tài nguyên hình ảnh vật phẩm (Items)
    tm.load("item_sword_steel",     "assets/items/sword_steel.png");
    tm.load("item_sword_mystic",    "assets/items/sword_mystic.png");
    tm.load("item_potion_starter",  "assets/items/potion_starter.png");
    tm.load("item_potion_health",   "assets/items/potion_health.png");
    tm.load("item_potion_strength", "assets/items/potion_strength.png");
    tm.load("item_potion_elixir",   "assets/items/potion_elixir.png");

    // 2. Thiết lập ĐẦY ĐỦ hệ thống hoạt họa phong phú cho Player
    player.addAnimation("idle",   std::make_unique<Animation>("warrior_idle", 4, 64, 80, 0.14f, true));
    player.addAnimation("run",    std::make_unique<Animation>("warrior_run", 8, 80, 80, 0.08f, true));
    player.addAnimation("attack", std::make_unique<Animation>("warrior_attack", 8, 96, 80, 0.06f, false));
    player.addAnimation("jump",   std::make_unique<Animation>("warrior_jump", 15, 64, 64, 0.03f, true));
    player.addAnimation("dead",   std::make_unique<Animation>("warrior_dead", 10, 64, 64, 0.12f, false));
    player.setState("idle");

    // 3. Khởi tạo tầng 1 hầm ngục 2D Side dài 75 ô
    dungeon.generate(1);
    // Vị trí xuất phát: ghi đè bởi --spawn (debug) nếu có, ngược lại dùng điểm start của map
    if (spawnOverrideX >= 0 && spawnOverrideY >= 0) {
        player.setPosition(Position(spawnOverrideX, spawnOverrideY), true);
    } else {
        player.setPosition(dungeon.getPlayerStartPos(), true);
    }

    // 4. Cung cấp vật phẩm khởi đầu vào túi đồ
    player.getInventory().addItem(std::make_unique<Potion>("Binh Thuoc Khoi Dau", "Hoi phuc 30 HP", 30, Position(0, 0), "item_potion_starter"));
    player.getInventory().addItem(std::make_unique<Weapon>("Dao Gam Khoi Dau", "Vu khi co ban +3 ATK", 3, Position(0, 0), "item_sword_steel"));

    // 5. Nhật ký chào mừng
    combatLog.push_back("Chao mung ban den voi Ham nguc Aethelgard!");
    combatLog.push_back("Nhan [B] hoac click chuot de mo Tui do.");
    combatLog.push_back("Ha guc Chua Heo Rung de pha giai phong an Cong Cua!");
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
            state = GameState::RUNNING;
        }
        return;
    }

    Position pPos = player.getPosition();

    // Phím Toàn màn hình [F11] hoặc [Alt + Enter]
    if (IsKeyPressed(KEY_F11) || ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && IsKeyPressed(KEY_ENTER))) {
        ToggleFullscreen();
    }

    // Phím B / I / Tab: bật/tắt hiển thị bảng túi đồ
    if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_I) || IsKeyPressed(KEY_TAB)) {
        showInventory = !showInventory;
    }
    // Phím ESC: đóng túi đồ nếu đang mở
    if (IsKeyPressed(KEY_ESCAPE) && showInventory) {
        showInventory = false;
        return;
    }

    // Phím L: thu gọn / mở khung nhật ký chiến đấu
    if (IsKeyPressed(KEY_L)) {
        showCombatLog = !showCombatLog;
    }

    // Tương tác chuột trái (Click UI Buttons & Inventory Slots)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        // 1. Nút [B] TÚI ĐỒ trên thanh Header
        Rectangle invBtn = { (float)(screenW - 325), 8.0f, 145.0f, 34.0f };
        if (CheckCollisionPointRec(mouse, invBtn)) {
            showInventory = !showInventory;
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

        // 3. Tương tác khi bảng Túi Đồ đang mở
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
        }
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
            CombatSystem::attack(player, *targetMonster, combatLog);
            bool wasKilled = !targetMonster->isAlive();
            dungeon.removeDeadMonsters(player);
            if (wasKilled) monstersDefeated++;

            // Boss gate: hạ BoarKing -> mở khóa Cổng Cửa trên Đỉnh Đền Thờ
            if (wasKilled && dungeon.checkBossDefeated()) {
                combatLog.push_back(">>> CHUA HEO RUNG DA HA GUOC! Cong cua o Dinh Den Tho da MO KHOA! <<<");
                combatLog.push_back(">>> Hay leo len be vang (x70) va nhan [Space] de chien thang! <<<");
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
            // Ưu tiên đáp XA trước: chéo xa 2 ô lên bệ -> ngang xa 3 ô ->
            // ngang xa 2 ô -> chéo gần -> thẳng đứng -> bước tới.
            Position farCandidates[] = {
                Position(pPos.x + jumpDir * 2, pPos.y - 1), // Nhảy xa 2 ô + lên bệ
                Position(pPos.x + jumpDir * 3, pPos.y),     // Nhảy xa 3 ô ngang
                Position(pPos.x + jumpDir * 2, pPos.y),     // Nhảy xa 2 ô ngang
                Position(pPos.x + jumpDir * 3, pPos.y - 1), // Bay xa 3 ô + lên cao
                Position(pPos.x + jumpDir, pPos.y - 1),     // Nhảy chéo gần lên bệ
                Position(pPos.x + jumpDir * 2, pPos.y + 1), // Nhảy xa + đáp xuống dốc
                Position(pPos.x, pPos.y - 1),               // Nhảy thẳng lên 1 ô
                Position(pPos.x + jumpDir, pPos.y)          // Nhảy bước tới
            };
            for (const auto& target : farCandidates) {
                if (canLand(target)) {
                    player.setPosition(target);
                    jumped = true;
                    break;
                }
            }
        } else {
            Position jumpCandidates[] = {
                Position(pPos.x + dirX, pPos.y - 1), // Nhảy chéo lên bệ trên
                Position(pPos.x + dirX * 2, pPos.y), // Nhảy xa 2 ô phía trước
                Position(pPos.x, pPos.y - 1),         // Nhảy thẳng lên 1 ô
                Position(pPos.x, pPos.y - 2),         // Nhảy cao 2 ô
                Position(pPos.x + dirX, pPos.y)      // Nhảy bước tới
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

    // Phím Lưu game [F5] & Tải game [F9]
    if (IsKeyPressed(KEY_F5)) {
        if (SaveLoadManager::saveGame("saves/savegame.txt", player, dungeon)) {
            combatLog.push_back("[HE THONG] Da luu game thanh cong (F5)!");
        }
    }
    if (IsKeyPressed(KEY_F9)) {
        if (SaveLoadManager::loadGame("saves/savegame.txt", player, dungeon)) {
            combatLog.push_back("[HE THONG] Da tai lai game thanh cong (F9)!");
        }
    }
}

void GameEngine::update(float deltaTime) {
    if (moveTimer > 0.0f) moveTimer -= deltaTime;
    if (attackTimer > 0.0f) attackTimer -= deltaTime;

    // ===== KỊCH BẢN PHÂN KHU: banner khi người chơi đi qua mốc khu mới =====
    if (state == GameState::RUNNING) {
        int px = player.getPosition().x;
        int zone = (px < 19) ? 0 : (px < 38) ? 1 : (px < 57) ? 2 : 3;
        if (zone != currentZone) {
            currentZone = zone;
            static const char* zoneBanners[4] = {
                "KHU A - TRAI KHOI DAU: Lam quen dieu khien. Oc sen ban dau chi phan don!",
                "KHU B - RUNG EP KHAC: Heo rung dang tuan tra, ong sat thu lurot treo tren dau!",
                "KHU C - VACH DA HUYEN BI: Oc sen giap chan duong. Co thuoc cuong hoa phia truoc!",
                "KHU D - DINH DEN THO: BOAR KING canh Cong Cua! Ha guc no de mo khoa be vang!"
            };
            bannerText = zoneBanners[zone];
            bannerTimer = 3.5f;
            combatLog.push_back(std::string("--- ") + zoneBanners[zone] + " ---");
        }
    }
    if (bannerTimer > 0.0f) bannerTimer -= deltaTime;

    handleInput();
    player.update(deltaTime);

    // Cập nhật AI quái vật thời gian thực độc lập khi không mở túi đồ
    if (!showInventory && state == GameState::RUNNING) {
        dungeon.update(deltaTime, player, combatLog);
        if (!player.isAlive()) {
            state = GameState::GAME_OVER;
            combatLog.push_back(">>> BAN DA TU TRAN! Nhan [R] de hoi sinh va thu lai. <<<");
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
    DrawRectangleRounded(Rectangle{ 98, 9, 90, 30 }, 0.3f, 4, Color{ 26, 22, 38, 255 });
    DrawRectangleRoundedLinesEx(Rectangle{ 98, 9, 90, 30 }, 0.3f, 4, 1.5f, Color{ 180, 140, 40, 255 });
    DrawCircle(112, 24, 6, GOLD);
    DrawCircle(112, 24, 3, YELLOW);
    drawText(TextFormat("%d", player.getGold()), 124, 14, 16, Color{ 255, 230, 110, 255 });

    // 1.3. Thanh Máu HP (RPG Health Bar)
    int hpX = 196, hpY = 9, hpW = 180, hpH = 30;
    float hpPercent = (float)player.getHp() / (float)player.getMaxHp();
    if (hpPercent < 0.0f) hpPercent = 0.0f;
    if (hpPercent > 1.0f) hpPercent = 1.0f;

    DrawRectangle(hpX, hpY, hpW, hpH, Color{ 35, 12, 16, 255 });
    int fillW = (int)((hpW - 4) * hpPercent);
    DrawRectangle(hpX + 2, hpY + 2, fillW, hpH - 4, Color{ 190, 25, 40, 255 });
    DrawRectangle(hpX + 2, hpY + 2, fillW, (hpH - 4) / 2, Color{ 255, 90, 100, 140 });
    DrawRectangleLines(hpX, hpY, hpW, hpH, Color{ 140, 50, 60, 255 });
    DrawRectangleLines(hpX + 1, hpY + 1, hpW - 2, hpH - 2, Color{ 55, 18, 24, 255 });
    drawText(TextFormat("HP %d/%d", player.getHp(), player.getMaxHp()), hpX + 44, hpY + 6, 16, WHITE);

    // 1.4. Chỉ số Tấn công & Phòng ngự
    drawText(TextFormat("ATK %d", player.getAttack()), 386, 15, 16, Color{ 255, 165, 70, 255 });
    drawText(TextFormat("DEF %d", player.getDefense()), 456, 15, 16, Color{ 120, 210, 255, 255 });

    // --- CỤM GIỮA: TẦNG NGỤC & LỘ TRÌNH (KHÔNG CHỒNG ĐÈ CHỮ) ---
    int midX = 535;
    drawText(TextFormat("TANG %d", dungeon.getFloorLevel()), midX, 7, 15, Color{ 90, 205, 255, 255 });

    const char* zoneName = dungeon.getZoneName(player.getPosition().x);
    drawText(zoneName, midX, 26, 13, Color{ 255, 215, 90, 255 });

    int pbX = midX + 175, pbY = 17, pbW = 120, pbH = 14;
    int stairsX = dungeon.getStairsPos().x;
    float progress = (stairsX > 0) ? (float)player.getPosition().x / (float)stairsX : 0.0f;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    DrawRectangle(pbX, pbY, pbW, pbH, Color{ 25, 20, 36, 255 });
    DrawRectangle(pbX + 1, pbY + 1, (int)((pbW - 2) * progress), pbH - 2, Color{ 220, 165, 35, 255 });
    DrawRectangleLines(pbX, pbY, pbW, pbH, Color{ 85, 75, 105, 255 });
    bool bossDefeated = dungeon.isBossDefeated();
    drawText("CONG CUA", pbX + pbW + 8, pbY - 1, 13, bossDefeated ? GOLD : Color{ 205, 75, 75, 255 });

    // --- CỤM PHẢI: NÚT MỞ TÚI ĐỒ & PHÍM TẮT HỆ THỐNG ---
    Rectangle invBtn = { (float)(screenW - 325), 8.0f, 145.0f, 32.0f };
    bool invHover = CheckCollisionPointRec(mouse, invBtn);
    Color invBg = showInventory 
        ? Color{ 60, 48, 85, 255 } 
        : (invHover ? Color{ 45, 38, 65, 255 } : Color{ 28, 24, 40, 255 });
    Color invBorder = showInventory 
        ? GOLD 
        : (invHover ? Color{ 240, 200, 70, 255 } : Color{ 85, 75, 105, 255 });
    
    DrawRectangleRounded(invBtn, 0.25f, 4, invBg);
    DrawRectangleRoundedLinesEx(invBtn, 0.25f, 4, 1.5f, invBorder);
    drawText(TextFormat("[B] TUI DO (%d/8)", (int)inv.getSize()), invBtn.x + 12, invBtn.y + 7, 15, invHover ? YELLOW : RAYWHITE);

    drawText("[F5] Luu  [F9] Tai  [F11] Toan man", screenW - 170, 17, 13, Color{ 165, 165, 185, 255 });

    // =========================================================================
    // 2. THANH MÁU TRÙM (BOSS HP BAR)
    // =========================================================================
    if (state == GameState::RUNNING) {
        Monster* boss = dungeon.getBossMonster();
        if (boss && boss->isAlive()) {
            Position pPos = player.getPosition();
            Position bPos = boss->getPosition();
            int cheb = std::max(std::abs(pPos.x - bPos.x), std::abs(pPos.y - bPos.y));
            if (cheb <= 11) {
                int bossBarW = 380, bossBarH = 18;
                int bossBarX = screenW / 2 - bossBarW / 2;
                int bossBarY = 56;
                float bossHp = (float)boss->getHp() / (float)boss->getMaxHp();
                if (bossHp < 0.0f) bossHp = 0.0f;
                if (bossHp > 1.0f) bossHp = 1.0f;

                DrawRectangle(bossBarX - 8, bossBarY - 6, bossBarW + 16, bossBarH + 28, Color{ 16, 12, 22, 235 });
                DrawRectangleLines(bossBarX - 8, bossBarY - 6, bossBarW + 16, bossBarH + 28, Color{ 180, 130, 45, 255 });
                
                drawText("BOAR KING - CHUA HEO RUNG", bossBarX + 60, bossBarY - 2, 15, Color{ 255, 170, 80, 255 });
                
                DrawRectangle(bossBarX, bossBarY + 18, bossBarW, bossBarH, Color{ 40, 15, 15, 255 });
                int fillBossW = (int)((bossBarW - 4) * bossHp);
                DrawRectangle(bossBarX + 2, bossBarY + 20, fillBossW, bossBarH - 4, Color{ 220, 60, 30, 255 });
                DrawRectangle(bossBarX + 2, bossBarY + 20, fillBossW, (bossBarH - 4) / 2, Color{ 255, 120, 60, 160 });
                DrawRectangleLines(bossBarX, bossBarY + 18, bossBarW, bossBarH, Color{ 130, 50, 40, 255 });
                
                drawText(TextFormat("HP: %d/%d", boss->getHp(), boss->getMaxHp()), bossBarX + bossBarW / 2 - 35, bossBarY + 19, 14, WHITE);
            }
        }
    }

    // =========================================================================
    // 3. BANNER KHU VỰC (HIỂN THỊ KHI VÀO PHÂN KHU MỚI)
    // =========================================================================
    if (state == GameState::RUNNING && bannerTimer > 0.0f) {
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
                    Color nameColor = w ? Color{ 255, 175, 75, 255 } : Color{ 100, 245, 150, 255 };
                    drawText(item->getName().c_str(), cardX + 86, cardY + 12, 17, nameColor);

                    if (w) {
                        drawText(TextFormat("+%d ATK", w->getBonusAttack()), cardX + cardW - 85, cardY + 13, 15, Color{ 255, 205, 120, 255 });
                    } else if (p) {
                        drawText(TextFormat("+%d HP", p->getHealAmount()), cardX + cardW - 85, cardY + 13, 15, Color{ 130, 255, 170, 255 });
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

    // =========================================================================
    // 5. MÀN HÌNH GAME OVER
    // =========================================================================
    if (state == GameState::GAME_OVER) {
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

void GameEngine::render() const {
    BeginDrawing();
    ClearBackground(Color{ 108, 160, 220, 255 });

    BeginMode2D(camera);

    // 1. Vẽ lớp nền xa (back.png) và quang cảnh cây cối (middle.png)
    dungeon.renderBackground(Vector2{0.0f, 0.0f});

    // 2. Vẽ mặt đất và vách đá (tiles.png)
    dungeon.render(Vector2{0.0f, 0.0f});

    // 3. Vẽ vật phẩm rơi trên sàn
    dungeon.renderItems(Vector2{0.0f, 0.0f});

    // 4. Vẽ quái vật (đa hình, đứng chân chuẩn trên mặt cỏ)
    dungeon.renderMonsters(Vector2{0.0f, 0.0f});

    // 5. Vẽ người chơi (bàn chân đứng vững chãi ngay trên mặt cỏ)
    player.render(1.8f, Vector2{ 0.0f, 0.0f });

    EndMode2D();

    // 6. Vẽ giao diện người dùng
    renderHUD();

    EndDrawing();
}

void GameEngine::run(const std::string& autoScreenshot) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT, Constants::GAME_TITLE);
    SetTargetFPS(Constants::TARGET_FPS);

    init();

    int testFrames = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        update(dt);
        render();

        if (IsKeyPressed(KEY_F12)) {
            TakeScreenshot("screenshot.png");
        }

        if (!autoScreenshot.empty()) {
            testFrames++;
            if (testFrames >= 10) {
                TakeScreenshot(autoScreenshot.c_str());
                break;
            }
        }
    }

    TextureManager::getInstance().unloadAll();
    CloseWindow();
}

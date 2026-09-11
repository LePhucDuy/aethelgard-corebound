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

GameEngine::GameEngine(int spawnX, int spawnY)
    : player("Hiep Si Aethelgard", Position(4, 15), 100, 16, 5),
      dungeon(Constants::DUNGEON_WIDTH, Constants::DUNGEON_HEIGHT),
      state(GameState::RUNNING),
      moveTimer(0.0f),
      attackTimer(0.0f),
      userZoomOffset(0.0f),
      currentZone(-1),
      bannerText(""),
      bannerTimer(0.0f),
      monstersDefeated(0),
      spawnOverrideX(spawnX),
      spawnOverrideY(spawnY) {
    camera.target = Vector2{ 0.0f, 20.0f * (float)Constants::TILE_SIZE };
    camera.offset = Vector2{ (float)Constants::SCREEN_WIDTH / 2.0f, (float)Constants::SCREEN_HEIGHT - 145.0f };
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

    // 2. Thiết lập ĐẦY ĐỦ hệ thống hoạt họa phong phú cho Player
    player.addAnimation("idle",   std::make_unique<Animation>("warrior_idle", 4, 64, 80, 0.14f, true));
    player.addAnimation("run",    std::make_unique<Animation>("warrior_run", 8, 80, 80, 0.08f, true));
    player.addAnimation("attack", std::make_unique<Animation>("warrior_attack", 8, 96, 80, 0.06f, false));
    player.addAnimation("jump",   std::make_unique<Animation>("warrior_jump", 15, 64, 64, 0.05f, false));
    player.addAnimation("dead",   std::make_unique<Animation>("warrior_dead", 10, 64, 64, 0.12f, false));
    player.setState("idle");

    // 3. Khởi tạo tầng 1 hầm ngục 2D Side dài 75 ô
    dungeon.generate(1);
    // Vị trí xuất phát: ghi đè bởi --spawn (debug) nếu có, ngược lại dùng điểm start của map
    if (spawnOverrideX >= 0 && spawnOverrideY >= 0) {
        player.setPosition(Position(spawnOverrideX, spawnOverrideY));
    } else {
        player.setPosition(dungeon.getPlayerStartPos());
    }

    // 4. Cung cấp vật phẩm khởi đầu vào túi đồ
    player.getInventory().addItem(std::make_unique<Potion>("Binh Thuoc Khoi Dau", "Hoi phuc 30 HP", 30));

    // 5. Nhật ký chào mừng
    combatLog.push_back("=== CHAO MUNG DEN VOI AETHELGARD: COREBOUND ===");
    combatLog.push_back("Giu phi: [A] [D] / Mui ten de Chay | [W] [S] de Leo");
    combatLog.push_back("Tan cong: [J] hoac [F] | Nhay vuot chuong ngai: [Space]");
    combatLog.push_back("Tui do: [1-9] | [F5]: Luu | [F9]: Tai | [F11]: Toan man hinh");
    combatLog.push_back("Thu / Phong man hinh: Cuon chuot giua (Mouse Wheel)");
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

        processMonsterTurn();
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
                processMonsterTurn();
                return;
            }
            state = GameState::VICTORY;
            combatLog.push_back(">>> BAN DA CHIEN THANG! AETHELGARD DUOC GIAI CUU! <<<");
            return;
        }

        int dirX = player.isFacingRight() ? 1 : -1;
        player.triggerJump(dirX, -1);

        Position jumpCandidates[] = {
            Position(pPos.x + dirX, pPos.y - 1), // Nhảy chéo lên bệ trên
            Position(pPos.x + dirX * 2, pPos.y), // Nhảy xa 2 ô phía trước
            Position(pPos.x, pPos.y - 1),         // Nhảy thẳng lên 1 ô
            Position(pPos.x, pPos.y - 2),         // Nhảy cao 2 ô
            Position(pPos.x + dirX, pPos.y)      // Nhảy bước tới
        };

        bool jumped = false;
        for (const auto& target : jumpCandidates) {
            if (dungeon.isWalkable(target) && dungeon.getMonsterAt(target) == nullptr) {
                player.setPosition(target);
                jumped = true;
                break;
            }
        }

        if (jumped) {
            std::unique_ptr<Item> item = dungeon.takeItemAt(player.getPosition());
            if (item) {
                combatLog.push_back("Nhat duoc: " + item->getName() + "!");
                player.getInventory().addItem(std::move(item));
            }
        }

        processMonsterTurn();
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

        if (isInit || moveTimer <= 0.0f) {
            moveTimer = isInit ? 0.22f : 0.10f; // Nhấn đầu tiên chờ 0.22s, giữ phím lặp lại mỗi 0.10s
            int dx = moveRightHeld ? 1 : -1;

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

            if (moved) {
                processMonsterTurn();
            } else {
                // Nếu bị quái vật cản bước
                Position forwardPos(pPos.x + dx, pPos.y);
                if (dungeon.getMonsterAt(forwardPos)) {
                    combatLog.push_back("Quai vat dang chan duong! Nhan [J] hoac [F] de tan cong.");
                }
            }
            return;
        }
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
            moveTimer = isInit ? 0.22f : 0.10f;
            int dy = moveUpHeld ? -1 : 1;

            Position candidates[] = {
                Position(pPos.x, pPos.y + dy),
                Position(pPos.x - 1, pPos.y + dy),
                Position(pPos.x + 1, pPos.y + dy)
            };

            bool moved = false;
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
                    moved = true;
                    break;
                }
            }

            if (moved) {
                processMonsterTurn();
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
                    processMonsterTurn();
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

void GameEngine::processMonsterTurn() {
    if (!player.isAlive()) return;

    // Lượt của quái: mỗi loài tự quyết định hành vi qua act() (đa hình thật sự)
    for (auto& monster : dungeon.getMonsters()) {
        if (!monster || !monster->isAlive()) continue;

        monster->act(dungeon, player, combatLog);

        if (!player.isAlive()) {
            state = GameState::GAME_OVER;
            combatLog.push_back(">>> BAN DA TU TRAN! Nhan [R] de hoi sinh va thu lai. <<<");
            break;
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
    dungeon.update(deltaTime);

    // Căn chỉnh camera ghim chặt đáy mặt đất vào sát mép trên thanh Nhật ký chiến đấu
    // Tuyệt đối loại bỏ hoàn toàn 100% vùng trống / khoảng trống bên dưới!
    float viewTop = 46.0f;
    float viewBottom = (float)GetScreenHeight() - 145.0f;
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

    // 1. Camera Offset:
    // Trục X căn giữa màn hình theo chiều ngang
    // Trục Y GHIM CHÍNH XÁC tại viewBottom (đỉnh khung nhật ký chiến đấu)
    camera.offset = Vector2{ (float)GetScreenWidth() / 2.0f, viewBottom };

    // 2. Camera Target:
    // Trục X bám theo người chơi trên suốt 75 ô ngang của tầng ngục
    float targetX = (float)(player.getPosition().x * Constants::TILE_SIZE + Constants::TILE_SIZE / 2);

    // Kẹp chặt camera target X để màn hình không bao giờ trôi ra ngoài biên trái (x < 0) hoặc biên phải
    float halfViewWidth = ((float)GetScreenWidth() / 2.0f) / camera.zoom;
    float worldWidth = (float)(Constants::DUNGEON_WIDTH * Constants::TILE_SIZE);
    if (worldWidth > halfViewWidth * 2.0f) {
        if (targetX < halfViewWidth) targetX = halfViewWidth;
        if (targetX > worldWidth - halfViewWidth) targetX = worldWidth - halfViewWidth;
    }

    // Trục Y:
    // Mặt đất dưới cùng là y = 18, khối đất dày xuống y = 19 (đáy = 20 * TILE_SIZE = 640px)
    float groundBottomY = 20.0f * (float)Constants::TILE_SIZE;
    float targetY = groundBottomY;

    // Khi người chơi leo lên các tầng cao (y <= 9) và mức zoom lớn khiến nhân vật chạm trần màn hình:
    float playerWorldY = (float)(player.getPosition().y * Constants::TILE_SIZE);
    float playerScreenY = (playerWorldY - targetY) * camera.zoom + viewBottom;
    if (playerScreenY < viewTop + 75.0f) {
        targetY = playerWorldY - (viewTop + 75.0f - viewBottom) / camera.zoom;
    }

    // Luôn đảm bảo targetY không bao giờ lớn hơn groundBottomY
    // Đảm bảo toán học: Đáy mặt đất KHÔNG BAO GIỜ trôi lên trên viewBottom -> 0 pixel vùng trống bên dưới!
    if (targetY > groundBottomY) {
        targetY = groundBottomY;
    }

    camera.target = Vector2{ targetX, targetY };
}

void GameEngine::renderHUD() const {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    // 1. Thanh trạng thái phía trên (Top Header Bar)
    DrawRectangle(0, 0, screenW, 46, Color{ 15, 13, 23, 240 });
    DrawLine(0, 46, screenW, 46, DARKGRAY);

    // Tên & Cấp độ
    drawText(TextFormat("Level %d | Vang: %d", player.getLevel(), player.getGold()), 16, 11, 20, GOLD);

    // Thanh máu HP
    int hpBarX = 230;
    int hpBarY = 10;
    int hpBarW = 180;
    int hpBarH = 26;
    float hpPercent = (float)player.getHp() / (float)player.getMaxHp();
    DrawRectangle(hpBarX, hpBarY, hpBarW, hpBarH, RED);
    DrawRectangle(hpBarX, hpBarY, (int)(hpBarW * hpPercent), hpBarH, GREEN);
    DrawRectangleLines(hpBarX, hpBarY, hpBarW, hpBarH, RAYWHITE);
    drawText(TextFormat("HP: %d/%d", player.getHp(), player.getMaxHp()), hpBarX + 40, hpBarY + 3, 17, BLACK);

    // Chỉ số ATK, DEF, Tầng ngục
    drawText(TextFormat("ATK: %d  DEF: %d", player.getAttack(), player.getDefense()), 430, 12, 19, RAYWHITE);
    drawText(TextFormat("TANG HAM NGUC: %d", dungeon.getFloorLevel()), 630, 12, 19, SKYBLUE);

    // ===== Kịch bản: tên khu vực + thanh tiến trình lộ trình tới Cổng Cửa =====
    const char* zoneName = dungeon.getZoneName(player.getPosition().x);
    drawText(zoneName, 740, 8, 14, GOLD);
    int pbX = 740, pbY = 30, pbW = 170, pbH = 10;
    int stairsX = dungeon.getStairsPos().x;
    float progress = (stairsX > 0) ? (float)player.getPosition().x / (float)stairsX : 0.0f;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;
    DrawRectangle(pbX, pbY, pbW, pbH, Color{ 40, 35, 55, 255 });
    DrawRectangle(pbX, pbY, (int)(pbW * progress), pbH, GOLD);
    DrawRectangleLines(pbX, pbY, pbW, pbH, RAYWHITE);
    drawText("CONG CUA", pbX + pbW + 8, pbY - 3, 14, (dungeon.hasBoss() && !dungeon.isBossDefeated()) ? MAROON : GOLD);

    // Phím tắt góc phải
    drawText("[F5] Luu | [F9] Tai | [F11] Toan man hinh", screenW - 350, 13, 17, LIGHTGRAY);

    // ===== Boss HP bar: hiện khi Boar King đang sống và ở gần người chơi =====
    if (state == GameState::RUNNING) {
        Monster* boss = dungeon.getBossMonster();
        if (boss && boss->isAlive()) {
            Position pPos = player.getPosition();
            Position bPos = boss->getPosition();
            int cheb = std::max(std::abs(pPos.x - bPos.x), std::abs(pPos.y - bPos.y));
            if (cheb <= 10) {
                int bossBarX = screenW - 530, bossBarY = 58, bossBarW = 280, bossBarH = 20;
                float bossHp = (float)boss->getHp() / (float)boss->getMaxHp();
                DrawRectangle(bossBarX - 4, bossBarY - 4, bossBarW + 8, bossBarH + 22, Color{ 20, 10, 10, 220 });
                drawText("BOAR KING - CHUA HEO RUNG", bossBarX, bossBarY - 2, 15, Color{ 255, 160, 90, 255 });
                DrawRectangle(bossBarX, bossBarY + 16, bossBarW, bossBarH, RED);
                DrawRectangle(bossBarX, bossBarY + 16, (int)(bossBarW * bossHp), bossBarH, Color{ 255, 120, 40, 255 });
                DrawRectangleLines(bossBarX, bossBarY + 16, bossBarW, bossBarH, RAYWHITE);
                drawText(TextFormat("HP: %d/%d", boss->getHp(), boss->getMaxHp()), bossBarX + 80, bossBarY + 20, 16, BLACK);
            }
        }
    }

    // 2. Bảng Túi đồ bên phải (Right Inventory Panel)
    int invW = 250;
    int invH = 280;
    int invX = screenW - invW - 16;
    int invY = 58;
    DrawRectangle(invX, invY, invW, invH, Color{ 18, 16, 28, 230 });
    DrawRectangleLines(invX, invY, invW, invH, DARKGRAY);
    drawText("TUI DO (Phim 1-9)", invX + 14, invY + 12, 19, YELLOW);

    const Inventory& inv = player.getInventory();
    if (inv.isEmpty()) {
        drawText("(Tui do trong)", invX + 14, invY + 44, 17, GRAY);
    } else {
        for (size_t i = 0; i < inv.getSize() && i < 8; ++i) {
            const Item* item = inv[i];
            if (item) {
                Color itemColor = (dynamic_cast<const Weapon*>(item)) ? ORANGE : GREEN;
                drawText(TextFormat("[%d] %s", (int)(i + 1), item->getName().c_str()), 
                         invX + 14, invY + 44 + (int)(i * 28), 17, itemColor);
            }
        }
    }

    // 3. Khung nhật ký chiến đấu phía dưới (Bottom Combat Log Panel)
    int logH = 145;
    int logY = screenH - logH;
    DrawRectangle(0, logY, screenW, logH, Color{ 12, 10, 20, 245 });
    DrawLine(0, logY, screenW, logY, DARKGRAY);
    drawText("NHAT KY CHIEN DAU (Turn-based Log) - Giu phim de di chuyen lien tuc:", 16, logY + 8, 17, GOLD);

    int maxLines = (logH - 36) / 21;
    int startIdx = (int)combatLog.size() > maxLines ? (int)combatLog.size() - maxLines : 0;
    for (size_t i = startIdx; i < combatLog.size(); ++i) {
        Color logColor = RAYWHITE;
        if (combatLog[i].find("tan cong") != std::string::npos) logColor = RED;
        else if (combatLog[i].find("CHUC MUNG") != std::string::npos || combatLog[i].find("CHAO MUNG") != std::string::npos) logColor = YELLOW;
        else if (combatLog[i].find("Nhat duoc") != std::string::npos) logColor = GREEN;
        else if (combatLog[i].find("HE THONG") != std::string::npos) logColor = SKYBLUE;
        drawText(combatLog[i].c_str(), 16, logY + 32 + (int)((i - startIdx) * 21), 16, logColor);
    }

    // 4. Màn hình Game Over
    if (state == GameState::GAME_OVER) {
        DrawRectangle(0, 0, screenW, screenH, Color{ 0, 0, 0, 200 });
        drawText("BAN DA THAT TRAN!", screenW / 2 - 180, screenH / 2 - 40, 36, RED);
        drawText("Nhan phim [R] de hoi sinh va thu lai", screenW / 2 - 160, screenH / 2 + 20, 20, RAYWHITE);
    }

    // 5. Banner khu vực (hiện giữa màn hình khi bước vào khu mới)
    if (state == GameState::RUNNING && bannerTimer > 0.0f) {
        float alpha = (bannerTimer > 3.0f) ? (3.5f - bannerTimer) * 2.0f
                                           : (bannerTimer < 0.5f ? bannerTimer * 2.0f : 1.0f);
        if (alpha > 1.0f) alpha = 1.0f;
        Color bannerBg = Color{ 15, 13, 23, (unsigned char)(200 * alpha) };
        Color bannerFg = GOLD;
        bannerFg.a = (unsigned char)(255 * alpha);
        int bW = 720, bH = 64;
        int bX = screenW / 2 - bW / 2, bY = 120;
        DrawRectangle(bX, bY, bW, bH, bannerBg);
        DrawRectangleLines(bX, bY, bW, bH, bannerFg);
        drawText(bannerText.c_str(), screenW / 2 - (float)bW / 2 + 20, bY + 20, 20, bannerFg);
    }

    // 6. Màn hình Chiến Thắng (Victory) — kết thúc kịch bản màn chơi
    if (state == GameState::VICTORY) {
        DrawRectangle(0, 0, screenW, screenH, Color{ 20, 15, 5, 215 });
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

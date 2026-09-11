#include "map/Dungeon.h"
#include "graphics/TextureManager.h"
#include "systems/MonsterFactory.h"
#include "entities/BoarKing.h"
#include "items/Weapon.h"
#include "items/Potion.h"
#include "core/Constants.h"
#include "entities/Player.h"
#include <cstdlib>
#include <algorithm>
#include <iostream>

Dungeon::Dungeon(int width, int height)
    : width(width), height(height), floorLevel(1),
      playerStartPos(4, 17), stairsPos(25, 7),
      hasBossFlag(false), bossDefeated(false) {
    grid.resize(height, std::vector<Tile>(width, Tile(TileType::EMPTY)));
}

void Dungeon::carveRoom(int rx, int ry, int rw, int rh) {
    (void)rx; (void)ry; (void)rw; (void)rh;
}

void Dungeon::carveCorridor(int x1, int y1, int x2, int y2) {
    (void)x1; (void)y1; (void)x2; (void)y2;
}

void Dungeon::generate(int floor) {
    floorLevel = floor;
    monsters.clear();
    groundItems.clear();
    hasBossFlag = false;
    bossDefeated = false;

    // 1. Khởi tạo toàn bộ không gian là trời trống (EMPTY)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            grid[y][x].setType(TileType::EMPTY);
        }
    }

    // Helper tạo bệ địa hình có mặt cỏ trên và lòng đất bên dưới
    auto makePlatform = [this](int startX, int endX, int surfaceY, int thickness) {
        for (int x = startX; x <= endX && x < width; ++x) {
            if (surfaceY >= 0 && surfaceY < height) {
                if (x == startX) {
                    grid[surfaceY][x].setCustom(TileType::FLOOR, Rectangle{ 16.0f, 16.0f, 16.0f, 16.0f }, true);
                } else if (x == endX) {
                    grid[surfaceY][x].setCustom(TileType::FLOOR, Rectangle{ 64.0f, 16.0f, 16.0f, 16.0f }, true);
                } else {
                    grid[surfaceY][x].setCustom(TileType::FLOOR, Rectangle{ 32.0f, 16.0f, 16.0f, 16.0f }, true);
                }
            }

            for (int y = surfaceY + 1; y <= surfaceY + thickness && y < height; ++y) {
                if (x == startX) {
                    grid[y][x].setCustom(TileType::WALL, Rectangle{ 16.0f, 32.0f, 16.0f, 16.0f }, false);
                } else if (x == endX) {
                    grid[y][x].setCustom(TileType::WALL, Rectangle{ 64.0f, 32.0f, 16.0f, 16.0f }, false);
                } else {
                    grid[y][x].setCustom(TileType::WALL, Rectangle{ 32.0f, 32.0f, 16.0f, 16.0f }, false);
                }
            }
        }
    };

    // Helper tạo bậc thang bước đi nối liền không bao giờ bị ngắt quãng
    auto makeStair = [this](int x, int y) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            grid[y][x].setCustom(TileType::FLOOR, Rectangle{ 112.0f, 16.0f, 16.0f, 16.0f }, true);
            // Đặt một khối đất bên dưới bậc để vững chắc
            if (y + 1 < height) {
                grid[y + 1][x].setCustom(TileType::WALL, Rectangle{ 32.0f, 32.0f, 16.0f, 16.0f }, false);
            }
        }
    };

    // =========================================================================
    // 2. BỐ TRÍ ĐỊA HÌNH 2D SIDE-VIEW DÀI 75 Ô (2400 PIXELS) THÔNG SUỐT 100%
    // =========================================================================

    // A. MẶT ĐẤT CHÍNH (Main Ground Floor) chạy suốt từ x = 0 đến x = width - 1 ở y = 18
    // Mặt cỏ ở y = 18, các khối đất lấp kín toàn bộ xuống tận đáy bản đồ (y = 19 đến height - 1)
    for (int x = 0; x < width; ++x) {
        grid[18][x].setCustom(TileType::FLOOR, Rectangle{ 32.0f, 16.0f, 16.0f, 16.0f }, true);
        for (int y = 19; y < height; ++y) {
            grid[y][x].setCustom(TileType::WALL, Rectangle{ 32.0f, 32.0f, 16.0f, 16.0f }, false);
        }
    }

    // B. KHU VỰC 1: ĐỒI KHỞI ĐẦU (x = 0 đến 16) ở y = 15
    makePlatform(0, 16, 15, 2);
    for (int x = 0; x <= 16; ++x) {
        grid[18][x].setCustom(TileType::WALL, Rectangle{ 32.0f, 32.0f, 16.0f, 16.0f }, false);
    }
    playerStartPos = Position(4, 15);

    // Cầu thang nối Đồi 1 xuống mặt đất chính (y=15 đến 18)
    makeStair(17, 16);
    makeStair(18, 17);

    // Cầu thang nối Đồi 1 lên Cao Nguyên 2 (y=15 lên 12)
    makeStair(17, 14);
    makeStair(18, 13);

    // C. KHU VỰC 2: RỪNG NẤM TRUNG TÂM (x = 19 đến 35) ở y = 12
    makePlatform(19, 35, 12, 2);

    // Cầu thang nối Khu 2 xuống mặt đất chính
    makeStair(26, 13);
    makeStair(27, 14);
    makeStair(28, 15);
    makeStair(29, 16);
    makeStair(30, 17);

    // Cầu thang nối Khu 2 lên Vách Đá 3 (y=12 lên 9)
    makeStair(36, 11);
    makeStair(37, 10);

    // D. KHU VỰC 3: VÁCH ĐÁ HUYỀN BÍ (x = 38 đến 54) ở y = 9
    makePlatform(38, 54, 9, 2);

    // Cầu thang nối Khu 3 xuống mặt đất chính
    makeStair(45, 10);
    makeStair(46, 11);
    makeStair(47, 12);
    makeStair(48, 13);
    makeStair(49, 14);
    makeStair(50, 15);
    makeStair(51, 16);
    makeStair(52, 17);

    // Cầu thang nối Khu 3 lên Đền Thờ 4 (y=9 lên 6)
    makeStair(55, 8);
    makeStair(56, 7);

    // E. KHU VỰC 4: ĐỀN THỜ CỔ TÍCH ĐỈNH NÚI (x = 57 đến 72) ở y = 6
    makePlatform(57, 72, 6, 2);

    // Cầu thang nối Đền Thờ xuống mặt đất chính ở bên phải
    makeStair(62, 7);
    makeStair(63, 8);
    makeStair(64, 9);
    makeStair(65, 10);
    makeStair(66, 11);
    makeStair(67, 12);
    makeStair(68, 13);
    makeStair(69, 14);
    makeStair(70, 15);
    makeStair(71, 16);
    makeStair(72, 17);

    // Bệ đá cổ chuyển tầng ngục tối đặt tại đỉnh Đền Thờ (x = 70, y = 6)
    stairsPos = Position(70, 6);
    grid[stairsPos.y][stairsPos.x].setCustom(TileType::STAIRS_DOWN, Rectangle{ 128.0f, 16.0f, 16.0f, 16.0f }, true);

    // =========================================================================
    // 3. PHÂN BỔ QUÁI VẬT THEO KỊCH BẢN 4 KHU (spawn có kiểm tra ô hợp lệ)
    // =========================================================================

    // KHU A - TRẠI KHỞI ĐẦU (x0-16, y15): ốc sên hiền chỉ phản đòn — mục tiêu tập đánh
    spawnMonster(MonsterType::SNAIL, Position(11, 15));

    // KHU B - RỪNG ÉP KHẮC (x19-35, y12): 2 heo rừng tuần tra + 1 ong trên không
    spawnMonster(MonsterType::BOAR, Position(23, 12));
    spawnMonster(MonsterType::BOAR, Position(31, 12));
    spawnMonster(MonsterType::SMALL_BEE, Position(26, 10));

    // KHU C - VÁCH ĐÁ HUYỀN BÍ (x38-54, y9): 2 ốc sên chặn lối + 1 ong
    spawnMonster(MonsterType::SNAIL, Position(44, 9));
    spawnMonster(MonsterType::SNAIL, Position(50, 9));
    spawnMonster(MonsterType::SMALL_BEE, Position(46, 7));

    // KHU D - ĐỈNH ĐỀN THỜ (x57-72, y6): BOSS canh Cổng Cửa + 1 ong hộ vệ
    spawnMonster(MonsterType::BOAR_KING, Position(64, 6));
    spawnMonster(MonsterType::SMALL_BEE, Position(68, 4));

    // =========================================================================
    // 4. SINH VẬT PHẨM TRÊN BẢN ĐỒ
    // =========================================================================
    groundItems.push_back(std::make_unique<Potion>(
        "Binh Mau Nho", "Hoi phuc 35 HP", 35, Position(8, 15)
    ));
    groundItems.push_back(std::make_unique<Weapon>(
        "Thanh Kiem Thep", "Vu khi tang +8 ATK", 8, Position(22, 12)
    ));
    groundItems.push_back(std::make_unique<Potion>(
        "Binh Thuoc Cuong Hoa", "Hoi phuc 60 HP", 60, Position(42, 9)
    ));
    groundItems.push_back(std::make_unique<Weapon>(
        "Dai Kiem Huyen Bi", "Vu khi tang +15 ATK", 15, Position(60, 6)
    ));
    groundItems.push_back(std::make_unique<Potion>(
        "Than Duoc Aethelgard", "Hoi phuc 100 HP", 100, Position(69, 6)
    ));

    std::cout << "[Dungeon] Sinh dia hinh 2D Side thanh cong: 75 o ngang cho Tang " << floorLevel << std::endl;
}

bool Dungeon::isValidPos(const Position& pos) const {
    return (pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height);
}

bool Dungeon::isWalkable(const Position& pos) const {
    if (!isValidPos(pos)) return false;
    return grid[pos.y][pos.x].isWalkable();
}

Monster* Dungeon::getMonsterAt(const Position& pos) {
    for (auto& monster : monsters) {
        if (monster && monster->isAlive() && monster->getPosition() == pos) {
            return monster.get();
        }
    }
    return nullptr;
}

Item* Dungeon::getItemAt(const Position& pos) {
    for (auto& item : groundItems) {
        if (item && item->isOnGround() && item->getPosition() == pos) {
            return item.get();
        }
    }
    return nullptr;
}

std::unique_ptr<Item> Dungeon::takeItemAt(const Position& pos) {
    for (auto it = groundItems.begin(); it != groundItems.end(); ++it) {
        if (*it && (*it)->getPosition() == pos) {
            std::unique_ptr<Item> taken = std::move(*it);
            taken->setOnGround(false);
            groundItems.erase(it);
            return taken;
        }
    }
    return nullptr;
}

void Dungeon::removeDeadMonsters(Player& player) {
    for (auto it = monsters.begin(); it != monsters.end(); ) {
        if (*it && !(*it)->isAlive()) {
            // Chờ animation chết chạy xong rồi mới xóa khỏi bản đồ
            if ((*it)->isDeathAnimFinished()) {
                (*it)->onDeath(player);
                it = monsters.erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
}

void Dungeon::update(float deltaTime) {
    for (auto& monster : monsters) {
        if (monster && monster->isAlive()) {
            monster->update(deltaTime);
        }
    }
}

// ===== Kịch bản màn chơi =====

const char* Dungeon::getZoneName(int x) const {
    if (x < 19) return "TRAI KHOI DAU";
    if (x < 38) return "RUNG EP KHAC";
    if (x < 57) return "VACH DA HUYEN BI";
    return "DINH DEN THO";
}

TileType Dungeon::getTileType(const Position& pos) const {
    if (!isValidPos(pos)) return TileType::EMPTY;
    return grid[pos.y][pos.x].getType();
}

bool Dungeon::spawnMonster(MonsterType type, const Position& desiredPos) {
    // Ong bay chỉ chấp nhận ô không khí (EMPTY); quái bộ cần ô đi được có sàn đỡ
    auto isValidSpawn = [&](const Position& p) {
        if (!isValidPos(p)) return false;
        if (getMonsterAt(p) != nullptr) return false;
        if (type == MonsterType::SMALL_BEE) return getTileType(p) == TileType::EMPTY;
        return isWalkable(p);
    };

    Position spawn = desiredPos;
    if (!isValidSpawn(spawn)) {
        // Tìm ô hợp lệ gần nhất theo vòng xoắn bán kính 1..4
        bool found = false;
        for (int r = 1; r <= 4 && !found; ++r) {
            for (int dy = -r; dy <= r && !found; ++dy) {
                for (int dx = -r; dx <= r && !found; ++dx) {
                    Position cand(desiredPos.x + dx, desiredPos.y + dy);
                    if (isValidSpawn(cand)) {
                        spawn = cand;
                        found = true;
                    }
                }
            }
        }
        if (!found) {
            std::cerr << "[Dungeon][WARN] Khong tim du o spawn hop le cho quai tai "
                      << desiredPos << " - bo qua!" << std::endl;
            return false;
        }
        std::cerr << "[Dungeon][WARN] Di chuyen spawn quai tu " << desiredPos
                  << " sang " << spawn << " (o goc khong hop le)" << std::endl;
    }

    monsters.push_back(MonsterFactory::create(type, spawn));

    // Đánh dấu boss để kích hoạt boss gate
    if (type == MonsterType::BOAR_KING) {
        hasBossFlag = true;
        bossDefeated = false;
    }
    return true;
}

Monster* Dungeon::getBossMonster() const {
    for (const auto& monster : monsters) {
        if (monster && dynamic_cast<BoarKing*>(monster.get()) != nullptr) {
            return monster.get();
        }
    }
    return nullptr;
}

bool Dungeon::checkBossDefeated() {
    // hasBossFlag chỉ bật khi boss đã sinh; getBossMonster() == nullptr
    // nghĩa là boss đã bị xóa khỏi danh sách (đã chết)
    if (hasBossFlag && !bossDefeated && getBossMonster() == nullptr) {
        bossDefeated = true;
        return true;
    }
    return false;
}

void Dungeon::renderBackground(Vector2 offset) const {
    TextureManager& tm = TextureManager::getInstance();
    const Texture2D& backTex = tm.get("bg_back");
    const Texture2D& midTex  = tm.get("bg_middle");

    float worldW = (float)(width * Constants::TILE_SIZE);
    float groundSurfaceY = 18.0f * (float)Constants::TILE_SIZE; // 576.0f

    // 1. LỚP BẦU TRỜI & NÚI NON XA: back.png (160x272)
    // Scale 4.0f (1088px chiều cao), phủ kín từ trên cao -400px xuống tận 688px
    if (backTex.id != 0) {
        float scale = 4.0f;
        float texW = (float)backTex.width * scale;
        float texH = (float)backTex.height * scale;
        float startY = groundSurfaceY + 112.0f - texH;

        for (float x = -texW * 2.0f; x < worldW + texW * 3.0f; x += texW) {
            DrawTextureEx(backTex, Vector2{ offset.x + x, offset.y + startY }, 0.0f, scale, WHITE);
        }
    }

    // 2. LỚP RỪNG CÂY CỔ THỤ: middle.png (384x272)
    // Scale 3.0f (816px chiều cao), cắm rễ vào mặt đất chính ở y = 576px, vươn cao lên -224px
    if (midTex.id != 0) {
        float scale = 3.0f;
        float texW = (float)midTex.width * scale;
        float texH = (float)midTex.height * scale;
        float treeY = groundSurfaceY + 16.0f - texH;

        for (float x = -texW * 2.0f; x < worldW + texW * 3.0f; x += texW) {
            DrawTextureEx(midTex, Vector2{ offset.x + x, offset.y + treeY }, 0.0f, scale, Color{ 255, 255, 255, 245 });
        }
    }
}

void Dungeon::render(Vector2 offset) const {
    const Texture2D& tilesTex = TextureManager::getInstance().get("tileset");
    if (tilesTex.id == 0) return;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            TileType type = grid[y][x].getType();
            if (type == TileType::EMPTY) continue;

            Rectangle destRec = {
                offset.x + (float)(x * Constants::TILE_SIZE),
                offset.y + (float)(y * Constants::TILE_SIZE),
                (float)Constants::TILE_SIZE,
                (float)Constants::TILE_SIZE
            };

            DrawTexturePro(tilesTex, grid[y][x].getSourceRect(), destRec, Vector2{0, 0}, 0.0f, WHITE);

            if (type == TileType::STAIRS_DOWN) {
                // Cổng Cửa: đã hạ boss -> sáng vàng rực rỡ (mở khóa);
                // chưa hạ -> bọc sắc đỏ đậm + bóng tối (đang bị phong ấn)
                Color gateColor = bossDefeated ? GOLD : Color{ 180, 70, 70, 255 };
                DrawRectangleLines((int)destRec.x, (int)destRec.y, (int)destRec.width, (int)destRec.height, gateColor);
                if (!bossDefeated) {
                    DrawRectangle((int)destRec.x, (int)destRec.y, (int)destRec.width, (int)destRec.height, Color{ 120, 30, 30, 110 });
                }
            }
        }
    }
}

void Dungeon::renderItems(Vector2 offset) const {
    for (auto& item : groundItems) {
        if (item && item->isOnGround()) {
            Position p = item->getPosition();
            int screenX = (int)(offset.x + (float)(p.x * Constants::TILE_SIZE) + 8);
            int screenY = (int)(offset.y + (float)(p.y * Constants::TILE_SIZE) - 18);
            
            DrawCircle(screenX + 8, screenY + 8, 9.0f, Color{ 255, 215, 0, 190 });
            DrawCircle(screenX + 8, screenY + 8, 6.0f, GOLD);
            DrawText("?", screenX + 5, screenY + 1, 14, BLACK);
        }
    }
}

void Dungeon::renderMonsters(Vector2 offset) const {
    for (auto& monster : monsters) {
        if (monster && monster->isAlive()) {
            // Đứng vững chãi ngay trên mặt cỏ
            monster->render(1.8f, offset);

            Position p = monster->getPosition();
            float barX = offset.x + (float)(p.x * Constants::TILE_SIZE);
            // Thanh máu nổi ngay trên đầu sprite (đỉnh sprite - 8px)
            float barY = offset.y + (float)((monster->isFlying() ? p.y : p.y + 1) * Constants::TILE_SIZE)
                       - 1.8f * 32.0f - 8.0f;
            float hpPercent = (float)monster->getHp() / (float)monster->getMaxHp();

            DrawRectangle((int)barX, (int)barY, Constants::TILE_SIZE, 4, RED);
            DrawRectangle((int)barX, (int)barY, (int)(Constants::TILE_SIZE * hpPercent), 4, GREEN);
        }
    }
}

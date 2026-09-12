#include "map/Dungeon.h"
#include "graphics/TextureManager.h"
#include "systems/MonsterFactory.h"
#include "entities/BoarKing.h"
#include "entities/WhiteBoar.h"
#include "items/Weapon.h"
#include "items/Potion.h"
#include "core/Constants.h"
#include "entities/Player.h"
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <cmath>

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

    // Helper tạo vực nước sâu (water chasm) ở tầng đáy (y = 18 đến đáy)
    auto makeWaterChasm = [this](int startX, int endX) {
        for (int x = startX; x <= endX && x < width; ++x) {
            for (int y = 18; y < height; ++y) {
                grid[y][x].setType(TileType::WATER);
            }
        }
    };

    // =========================================================================
    // 2. BỐ TRÍ ĐỊA HÌNH 2D SIDE-VIEW DÀI 130 Ô (4160 PIXELS)
    // =========================================================================

    // A. MẶT ĐẤT CHÍNH (Main Ground Floor) chạy suốt từ x = 0 đến x = width - 1 ở y = 18
    for (int x = 0; x < width; ++x) {
        grid[18][x].setCustom(TileType::FLOOR, Rectangle{ 32.0f, 16.0f, 16.0f, 16.0f }, true);
        for (int y = 19; y < height; ++y) {
            grid[y][x].setCustom(TileType::WALL, Rectangle{ 32.0f, 32.0f, 16.0f, 16.0f }, false);
        }
    }

    // TẠO CÁC VỰC ĐẦM LẦY LÚN Ở TẦNG ĐÁY (y = 18 đến height - 1)
    // Đầm lầy 1: Rừng nấm (x = 41..43, rộng 3 ô -> đứng ở bờ x=40 nhảy vọt qua x=44)
    makeWaterChasm(41, 43);
    // Đầm lầy 2: Vách đá huyền bí (x = 69..71, rộng 3 ô -> nhảy vọt từ x=68 sang x=72)
    makeWaterChasm(69, 71);
    // Đầm lầy 3: Bình nguyên tàn tích (x = 99..101, rộng 3 ô -> nhảy vọt từ x=98 sang x=102)
    makeWaterChasm(99, 101);

    // =========================================================================
    // KHU A: TRẠI KHỞI ĐẦU (x = 0 đến 24)
    // =========================================================================
    // Đồi khởi đầu (x = 0..16) ở y = 15
    makePlatform(0, 16, 15, 2);
    playerStartPos = Position(4, 15);

    // Cầu thang từ Đồi 1 xuống mặt đất (x = 12..14)
    makeStair(12, 16);
    makeStair(13, 17);

    // HỐ PIT KHOẢNG CÁCH 0: x = 17..18 (khoảng không 2 ô để nhảy vọt qua)
    // Dưới đáy x = 17..18 là mặt sàn đất y = 18 an toàn để rơi xuống khi trượt chân

    // BẬC BỆ ĐÓN BÊN KHU B: phẳng ngang bằng tầm đồi (y = 15) từ x = 19 đến 23
    makePlatform(19, 23, 15, 2);

    // Bậc thang nối từ y = 15 lên tầng cao y = 12 của Khu B
    makeStair(24, 14);
    makeStair(25, 13);

    // =========================================================================
    // KHU B: RỪNG NẤM & CẦU TREO (x = 26 đến 55) - Tầng cao y = 12
    // Hố pit 1: x = 36..37 (rơi xuống sàn đất y = 18)
    // Cầu treo: x = 38..47 (bắc ngang qua Vực nước 1 x = 40..44)
    // Hố pit 2: x = 48..49 (rơi xuống sàn đất y = 18)
    // =========================================================================
    makePlatform(26, 35, 12, 2); // Đoạn 1 kéo dài từ x = 26 đến 35
    // Hố pit 1: x = 36..37 (trống để nhảy hoặc rơi)
    makePlatform(38, 47, 12, 1); // Cầu treo bắc qua vực nước
    // Hố pit 2: x = 48..49 (trống)
    makePlatform(50, 54, 12, 2); // Đoạn 3

    // Bậc thang nối Khu B xuống mặt đất chính
    makeStair(26, 17);
    makeStair(27, 16);
    makeStair(28, 15);
    makeStair(29, 14);
    makeStair(30, 13);

    // Bậc thang nối Khu B lên Vách Đá Khu C (y=12 lên 9)
    makeStair(55, 11);
    makeStair(56, 10);

    // =========================================================================
    // KHU C: VÁCH ĐÁ & VỰC NƯỚC (x = 56 đến 88) - Tầng cao y = 9
    // Hố pit 3: x = 69..70 - RƠI XUỐNG VỰC NƯỚC 2 -> CHẾT ĐUỐI!
    // Hố pit 4: x = 81..82 - Rơi xuống sàn đất tầng dưới y = 18
    // =========================================================================
    makePlatform(57, 68, 9, 2); // Đoạn vách đá 1
    // Hố pit 3: x = 69..70 (trống - bên dưới là vực nước sâu!)
    makePlatform(71, 80, 9, 2); // Đoạn vách đá 2
    // Hố pit 4: x = 81..82 (trống - bên dưới là sàn đất)
    makePlatform(83, 88, 9, 2); // Đoạn vách đá 3

    // Cầu thang nối Khu C xuống tầng đất bên trái
    makeStair(58, 17);
    makeStair(59, 16);
    makeStair(60, 15);
    makeStair(61, 14);
    makeStair(62, 13);
    makeStair(63, 12);
    makeStair(64, 11);
    makeStair(65, 10);

    // Cầu thang nối Khu C xuống tầng đất bên phải
    makeStair(76, 17);
    makeStair(77, 16);
    makeStair(78, 15);
    makeStair(79, 14);
    makeStair(80, 13);
    makeStair(81, 12);
    makeStair(82, 11);
    makeStair(83, 10);

    // =========================================================================
    // KHU D: BÌNH NGUYÊN TÀN TÍCH (x = 89 đến 112) - Tầng cao y = 9
    // Hố pit 5: x = 99..100 - RƠI XUỐNG VỰC NƯỚC 3 -> CHẾT ĐUỐI!
    // =========================================================================
    makePlatform(89, 98, 9, 2);   // Đoạn tàn tích 1
    // Hố pit 5: x = 99..100 (trống - bên dưới là vực nước sâu!)
    makePlatform(101, 110, 9, 2); // Đoạn tàn tích 2

    // Cầu thang từ tầng dưới lên Khu D
    makeStair(90, 17);
    makeStair(91, 16);
    makeStair(92, 15);
    makeStair(93, 14);
    makeStair(94, 13);
    makeStair(95, 12);
    makeStair(96, 11);
    makeStair(97, 10);

    // Cầu thang nối Khu D lên Đỉnh Đền Thờ Khu E (y=9 lên 6)
    makeStair(111, 8);
    makeStair(112, 7);

    // =========================================================================
    // KHU E: ĐỈNH ĐỀN THỜ (x = 113 đến 129) - Tầng cao y = 6
    // Cầu thang phía sau Đền Thờ dẫn xuống Đấu Trường Boss phía trước
    // =========================================================================
    makePlatform(113, 128, 6, 2);

    // Cầu thang phía sau Đền Thờ dẫn xuống tầng đất (y = 18)
    makeStair(122, 7);
    makeStair(123, 8);
    makeStair(124, 9);
    makeStair(124, 10);
    makeStair(125, 11);
    makeStair(125, 12);
    makeStair(126, 13);
    makeStair(126, 14);
    makeStair(127, 15);
    makeStair(127, 16);
    makeStair(128, 17);

    // =========================================================================
    // KHU F: ĐẤU TRƯỜNG BOAR KING (x = 130 đến 164) - Tầng trệt y = 18
    // Đấu trường phẳng lì, trống hoàn toàn, không có nền trên cao
    // =========================================================================
    // Bức tường thành chặn biên phải ở x = 164
    for (int y = 0; y < height; ++y) {
        grid[y][164].setCustom(TileType::WALL, Rectangle{ 32.0f, 32.0f, 16.0f, 16.0f }, false);
    }
    // Cổng vòm cổ kính vào đấu trường ở x = 129
    for (int y = 0; y <= 14; ++y) {
        grid[y][129].setCustom(TileType::WALL, Rectangle{ 32.0f, 32.0f, 16.0f, 16.0f }, false);
    }

    // Bệ đá cổ chuyển tầng / Chiến thắng đặt tại cuối Đấu Trường (x = 162, y = 18)
    stairsPos = Position(162, 18);
    grid[stairsPos.y][stairsPos.x].setCustom(TileType::STAIRS_DOWN, Rectangle{ 128.0f, 16.0f, 16.0f, 16.0f }, true);

    // =========================================================================
    // 3. PHÂN BỔ QUÁI VẬT THEO 6 KHU VỰC TRÊN BẢN ĐỒ 165 Ô
    // =========================================================================
    // KHU A - TRẠI KHỞI ĐẦU (x = 0..24)
    spawnMonster(MonsterType::SNAIL, Position(11, 15));
    spawnMonster(MonsterType::SNAIL, Position(21, 18));

    // KHU B - RỪNG NẤM & CẦU TREO (x = 25..55)
    spawnMonster(MonsterType::BOAR, Position(27, 12));
    spawnMonster(MonsterType::SMALL_BEE, Position(33, 10));
    spawnMonster(MonsterType::BOAR, Position(43, 12)); // Trên cầu treo bắc qua nước
    spawnMonster(MonsterType::SMALL_BEE, Position(52, 10));
    // Tầng dưới Khu B
    spawnMonster(MonsterType::BOAR, Position(29, 18));
    spawnMonster(MonsterType::SNAIL, Position(35, 18));
    spawnMonster(MonsterType::BOAR, Position(52, 18));

    // KHU C - VÁCH ĐÁ & VỰC NƯỚC (x = 56..88)
    spawnMonster(MonsterType::SNAIL, Position(60, 9));
    spawnMonster(MonsterType::SMALL_BEE, Position(65, 7));
    spawnMonster(MonsterType::BOAR, Position(75, 9));
    spawnMonster(MonsterType::SNAIL, Position(79, 9));
    spawnMonster(MonsterType::SMALL_BEE, Position(85, 7));
    // Tầng dưới Khu C
    spawnMonster(MonsterType::BOAR, Position(60, 18));
    spawnMonster(MonsterType::SNAIL, Position(66, 18));
    spawnMonster(MonsterType::BOAR, Position(78, 18));
    spawnMonster(MonsterType::SNAIL, Position(86, 18));

    // KHU D - BÌNH NGUYÊN TÀN TÍCH (x = 89..112)
    spawnMonster(MonsterType::BOAR, Position(93, 9));
    spawnMonster(MonsterType::SMALL_BEE, Position(96, 7));
    spawnMonster(MonsterType::WHITE_BOAR, Position(105, 9));   // Quái cấp trung canh giữ tầng trên Khu D
    spawnMonster(MonsterType::SMALL_BEE, Position(109, 7));
    // Tầng dưới Khu D
    spawnMonster(MonsterType::BOAR, Position(92, 18));
    spawnMonster(MonsterType::SNAIL, Position(96, 18));
    spawnMonster(MonsterType::WHITE_BOAR, Position(106, 18));  // Quái cấp trung tuần tra lối vào đền thờ
    spawnMonster(MonsterType::SNAIL, Position(110, 18));

    // KHU E - ĐỈNH ĐỀN THỜ (x = 113..129)
    spawnMonster(MonsterType::WHITE_BOAR, Position(121, 6));   // Quái cấp trung Hộ Vệ Đền Thờ
    spawnMonster(MonsterType::SMALL_BEE, Position(117, 4));    // Hộ vệ bay
    spawnMonster(MonsterType::SMALL_BEE, Position(125, 4));
    // Tầng hầm Đền Thờ (y = 18)
    spawnMonster(MonsterType::WHITE_BOAR, Position(118, 18));  // Quái cấp trung chốt chặn cổng đấu trường
    spawnMonster(MonsterType::SNAIL, Position(123, 18));

    // KHU F - ĐẤU TRƯỜNG BOAR KING (x = 130..164)
    spawnMonster(MonsterType::BOAR_KING, Position(160, 18)); // BOSS CHÍNH - Chờ ở rìa phải lao ra

    // =========================================================================
    // 4. SINH VẬT PHẨM TRÊN BẢN ĐỒ 130 Ô
    // =========================================================================
    // KHU A
    groundItems.push_back(std::make_unique<Potion>(
        "Binh Mau Nho", "Hoi phuc 35 HP", 35, Position(8, 15), "item_potion_health"
    ));
    groundItems.push_back(std::make_unique<Potion>(
        "Binh Mau Nho", "Hoi phuc 35 HP", 35, Position(19, 18), "item_potion_health"
    ));

    // KHU B
    groundItems.push_back(std::make_unique<Weapon>(
        "Thanh Kiem Thep", "Vu khi tang +8 ATK", 8, Position(26, 12), "item_sword_steel"
    ));
    groundItems.push_back(std::make_unique<Potion>(
        "Binh Thuoc Cuong Hoa", "Hoi phuc 60 HP", 60, Position(44, 12), "item_potion_strength"
    ));
    groundItems.push_back(std::make_unique<Potion>(
        "Binh Mau Nho", "Hoi phuc 35 HP", 35, Position(33, 18), "item_potion_health"
    ));

    // KHU C
    groundItems.push_back(std::make_unique<Potion>(
        "Binh Thuoc Cuong Hoa", "Hoi phuc 60 HP", 60, Position(62, 9), "item_potion_strength"
    ));
    groundItems.push_back(std::make_unique<Weapon>(
        "Dai Kiem Huyen Bi", "Vu khi tang +15 ATK", 15, Position(76, 9), "item_sword_mystic"
    ));
    groundItems.push_back(std::make_unique<Potion>(
        "Than Duoc Aethelgard", "Hoi phuc 100 HP", 100, Position(85, 9), "item_potion_elixir"
    ));
    groundItems.push_back(std::make_unique<Potion>(
        "Binh Thuoc Cuong Hoa", "Hoi phuc 60 HP", 60, Position(64, 18), "item_potion_strength"
    ));
    groundItems.push_back(std::make_unique<Potion>(
        "Than Duoc Aethelgard", "Hoi phuc 100 HP", 100, Position(82, 18), "item_potion_elixir"
    ));

    // KHU D
    groundItems.push_back(std::make_unique<Weapon>(
        "Thanh Kiem Thep", "Vu khi tang +8 ATK", 8, Position(95, 9), "item_sword_steel"
    ));
    groundItems.push_back(std::make_unique<Potion>(
        "Binh Thuoc Cuong Hoa", "Hoi phuc 60 HP", 60, Position(106, 9), "item_potion_strength"
    ));
    groundItems.push_back(std::make_unique<Potion>(
        "Than Duoc Aethelgard", "Hoi phuc 100 HP", 100, Position(94, 18), "item_potion_elixir"
    ));
    groundItems.push_back(std::make_unique<Potion>(
        "Binh Mau Nho", "Hoi phuc 35 HP", 35, Position(108, 18), "item_potion_health"
    ));

    // KHU E - ĐỈNH ĐỀN THỜ
    groundItems.push_back(std::make_unique<Potion>(
        "Than Duoc Aethelgard", "Hoi phuc 100 HP", 100, Position(116, 6), "item_potion_elixir"
    ));
    groundItems.push_back(std::make_unique<Weapon>(
        "Dai Kiem Huyen Bi", "Vu khi tang +15 ATK", 15, Position(124, 6), "item_sword_mystic"
    ));
    // KHU F - ĐẤU TRƯỜNG BOAR KING
    groundItems.push_back(std::make_unique<Potion>(
        "Than Duoc Aethelgard", "Hoi phuc 100 HP", 100, Position(133, 18), "item_potion_elixir"
    ));
    groundItems.push_back(std::make_unique<Potion>(
        "Binh Thuoc Cuong Hoa", "Hoi phuc 60 HP", 60, Position(145, 18), "item_potion_strength"
    ));
    groundItems.push_back(std::make_unique<Potion>(
        "Than Duoc Aethelgard", "Hoi phuc 100 HP", 100, Position(155, 18), "item_potion_elixir"
    ));

    std::cout << "[Dungeon] Sinh dia hinh 2D Side thanh cong: 165 o ngang (6 khu) cho Tang " << floorLevel << std::endl;
}

bool Dungeon::isValidPos(const Position& pos) const {
    return (pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height);
}

bool Dungeon::isWalkable(const Position& pos) const {
    if (!isValidPos(pos)) return false;
    return grid[pos.y][pos.x].isWalkable();
}

bool Dungeon::isWater(const Position& pos) const {
    if (!isValidPos(pos)) return false;
    return grid[pos.y][pos.x].getType() == TileType::WATER;
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

void Dungeon::update(float deltaTime, Player& player, std::vector<std::string>& combatLog) {
    for (auto& monster : monsters) {
        if (monster) {
            monster->update(deltaTime);
            if (monster->isAlive()) {
                monster->updateAI(deltaTime, *this, player, combatLog);
            }
        }
    }
    removeDeadMonsters(player);
}

// ===== Kịch bản màn chơi =====

const char* Dungeon::getZoneName(int x) const {
    if (x < 25) return "TRAI KHOI DAU";
    if (x < 56) return "RUNG NAM & CAU TREO";
    if (x < 89) return "VACH DA & VUC NUOC";
    if (x < 113) return "BINH NGUYEN TAN TICH";
    if (x < 130) return "DINH DEN THO";
    return "DAU TRUONG BOAR KING";
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
    // hasBossFlag chỉ bật khi boss đã sinh;
    // boss chết khi getBossMonster() == nullptr hoặc !boss->isAlive()
    if (hasBossFlag && !bossDefeated) {
        Monster* boss = getBossMonster();
        if (boss == nullptr || !boss->isAlive()) {
            bossDefeated = true;
            return true;
        }
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

            if (type == TileType::WATER) {
                float timeSec = (float)GetTime();
                float swampDrop = 14.0f; // Mặt bùn đầm lầy thấp hơn nền đất 14px

                if (y == 18) {
                    // 1. KHE NỨT ĐẦM LẦY / BỜ VÁCH BÙN THỦNG (vùng từ mép nền y=0 đến swampDrop)
                    // Hốc tối bùn than
                    DrawRectangle((int)destRec.x, (int)destRec.y, (int)destRec.width, (int)swampDrop, Color{ 16, 20, 12, 235 });
                    // Vòm bóng đổ rìa vực
                    DrawRectangle((int)destRec.x, (int)destRec.y, (int)destRec.width, 3, Color{ 8, 12, 6, 210 });

                    // BỜ VÁCH ĐẤT BÙN TRÁI (nếu ô bên trái là đất liền)
                    if (x > 0 && grid[18][x - 1].getType() != TileType::WATER) {
                        DrawRectangle((int)destRec.x, (int)destRec.y, 6, (int)swampDrop + 8, Color{ 52, 38, 26, 255 });
                        DrawRectangle((int)destRec.x + 6, (int)destRec.y, 2, (int)swampDrop + 4, Color{ 36, 26, 18, 220 });
                        DrawRectangle((int)destRec.x, (int)destRec.y, 5, 3, Color{ 45, 95, 35, 255 }); // Rêu rủ mép trái
                    }
                    // BỜ VÁCH ĐẤT BÙN PHẢI (nếu ô bên phải là đất liền)
                    if (x < width - 1 && grid[18][x + 1].getType() != TileType::WATER) {
                        DrawRectangle((int)(destRec.x + destRec.width - 6), (int)destRec.y, 6, (int)swampDrop + 8, Color{ 52, 38, 26, 255 });
                        DrawRectangle((int)(destRec.x + destRec.width - 8), (int)destRec.y, 2, (int)swampDrop + 4, Color{ 36, 26, 18, 220 });
                        DrawRectangle((int)(destRec.x + destRec.width - 5), (int)destRec.y, 5, 3, Color{ 45, 95, 35, 255 }); // Rêu rủ mép phải
                    }

                    // 2. MẶT BÙN ĐẦM LẦY LÚN (bắt đầu từ destRec.y + swampDrop)
                    Rectangle swampRec = {
                        destRec.x,
                        destRec.y + swampDrop,
                        destRec.width,
                        destRec.height - swampDrop
                    };
                    float bogH = sinf(timeSec * 2.2f + (float)x * 0.8f) * 2.0f;

                    // Thân bùn đầm lầy xanh rêu đậm
                    DrawRectangleRec(swampRec, Color{ 34, 46, 24, 245 });

                    // Lớp váng rêu đầm lầy dập dềnh
                    DrawRectangle((int)swampRec.x, (int)(swampRec.y + bogH + 2.0f), (int)swampRec.width, 4, Color{ 58, 86, 38, 235 });
                    DrawRectangle((int)swampRec.x, (int)(swampRec.y + bogH), (int)swampRec.width, 2, Color{ 88, 128, 54, 230 });

                    // Bọt khí mê-tan đầm lầy sôi sùng sục (Toxic swamp bubbles)
                    // Bọt khí to
                    float b1X = swampRec.x + 8.0f + sinf(timeSec * 1.8f + (float)x * 2.0f) * 6.0f;
                    float b1Y = swampRec.y + 4.0f + cosf(timeSec * 2.2f + (float)x) * 2.5f;
                    float b1R = 2.2f + sinf(timeSec * 3.0f + (float)x) * 0.8f;
                    if (b1R > 0.8f) {
                        DrawCircle((int)b1X, (int)b1Y, b1R, Color{ 135, 205, 80, 220 });
                        DrawCircle((int)b1X, (int)b1Y, b1R * 0.5f, Color{ 210, 250, 160, 240 });
                    }
                    // Bọt khí nhỏ
                    float b2X = swampRec.x + 22.0f + cosf(timeSec * 2.5f + (float)x * 1.5f) * 5.0f;
                    float b2Y = swampRec.y + 6.0f + sinf(timeSec * 2.0f + (float)x * 3.0f) * 2.0f;
                    DrawCircle((int)b2X, (int)b2Y, 1.4f, Color{ 160, 225, 95, 200 });

                    // Làn khói độc mờ ảo bốc lên trên mặt bùn (Toxic swamp mist)
                    float mistY = swampRec.y - 6.0f + sinf(timeSec * 1.5f + (float)x) * 3.0f;
                    DrawRectangle((int)swampRec.x, (int)mistY, (int)swampRec.width, 5, Color{ 85, 160, 65, 40 });
                } else {
                    // Bùn lầy tầng sâu (y > 18): bùn đen quánh đặc
                    DrawRectangleRec(destRec, Color{ 18, 25, 14, 250 });
                    // Vệt quánh trầm tích đầm lầy
                    if ((x + y) % 3 == 0) {
                        DrawRectangle((int)destRec.x + 4, (int)destRec.y + 8, (int)destRec.width - 8, 3, Color{ 32, 44, 22, 110 });
                    }
                }
                continue;
            }

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
    const TextureManager& tm = TextureManager::getInstance();
    float timeSec = (float)GetTime();

    for (auto& item : groundItems) {
        if (item && item->isOnGround()) {
            Position p = item->getPosition();
            float baseX = offset.x + (float)(p.x * Constants::TILE_SIZE);
            float baseY = offset.y + (float)(p.y * Constants::TILE_SIZE);

            // 1. Bóng đổ (Shadow) mờ nhẹ ngay trên bề mặt sàn gạch/cỏ
            DrawEllipse((int)(baseX + 16), (int)(baseY + 2), 14, 5, Color{ 0, 0, 0, 120 });

            // 2. Hiệu ứng lơ lửng nhấp nhô nhẹ nhàng (Floating / bobbing animation)
            float bobOffset = sinf(timeSec * 3.5f + (float)p.x * 0.8f) * 4.0f;

            // 3. Vầng hào quang phát sáng đa sắc (Golden cho vũ khí, Emerald cho bình thuốc)
            bool isWeapon = (dynamic_cast<const Weapon*>(item.get()) != nullptr);
            Color auraColor = isWeapon 
                ? Color{ 255, 205, 80, (unsigned char)(70 + 30 * sinf(timeSec * 4.0f)) } 
                : Color{ 90, 240, 160, (unsigned char)(70 + 30 * sinf(timeSec * 4.0f)) };
            Color auraCenter = isWeapon ? Color{ 255, 220, 100, 90 } : Color{ 100, 255, 170, 90 };
            
            DrawCircleGradient(Vector2{ baseX + 16.0f, baseY - 14.0f + bobOffset }, 20.0f + 2.0f * sinf(timeSec * 3.0f), auraCenter, Color{ 0, 0, 0, 0 });
            DrawCircleLines((int)(baseX + 16), (int)(baseY + 2), 15.0f + sinf(timeSec * 4.0f) * 2.0f, auraColor);

            // 4. Vẽ Texture của vật phẩm to rõ ràng (40x40 - tăng từ 28x28)
            constexpr float ITEM_SIZE = 40.0f;
            float itemX = baseX + ((float)Constants::TILE_SIZE - ITEM_SIZE) / 2.0f;
            float itemY = baseY - ITEM_SIZE + 4.0f + bobOffset;

            const std::string& texId = item->getTextureId();
            if (tm.has(texId)) {
                const Texture2D& tex = tm.get(texId);
                Rectangle srcRec = { 0, 0, (float)tex.width, (float)tex.height };
                Rectangle destRec = { itemX, itemY, ITEM_SIZE, ITEM_SIZE };
                DrawTexturePro(tex, srcRec, destRec, Vector2{ 0, 0 }, 0.0f, WHITE);
            } else {
                // Fallback nếu chưa có texture
                DrawCircle((int)(baseX + 16), (int)(itemY + ITEM_SIZE / 2.0f), 12.0f, GOLD);
                DrawText("?", (int)(baseX + 12), (int)(itemY + ITEM_SIZE / 2.0f - 7.0f), 16, BLACK);
            }
        }
    }
}

void Dungeon::renderMonsters(Vector2 offset) const {
    for (auto& monster : monsters) {
        if (monster && (monster->isAlive() || monster->isDying())) {
            // Đứng vững chãi ngay trên mặt cỏ
            monster->render(1.8f, offset);

            if (monster->isAlive()) {
                // Boss BoarKing đã có thanh máu trùm đồ sộ ở đỉnh màn hình
                if (dynamic_cast<BoarKing*>(monster.get()) != nullptr) {
                    continue;
                }

                const Vector2& vPos = monster->getVisualPosition();
                float barX = offset.x + vPos.x;
                // Quái cấp trung (WhiteBoar) to hơn 30% nên nâng thanh máu lên 10px để không che đỉnh gai lưng
                float extraH = (dynamic_cast<WhiteBoar*>(monster.get()) != nullptr) ? 10.0f : 0.0f;
                float barY = offset.y + vPos.y - 1.8f * 32.0f - 8.0f - extraH;
                float hpPercent = (float)monster->getHp() / (float)monster->getMaxHp();

                DrawRectangle((int)barX, (int)barY, Constants::TILE_SIZE, 4, RED);
                DrawRectangle((int)barX, (int)barY, (int)(Constants::TILE_SIZE * hpPercent), 4, GREEN);
            }
        }
    }
}

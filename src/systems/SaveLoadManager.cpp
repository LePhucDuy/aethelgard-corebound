#include "systems/SaveLoadManager.h"
#include "core/GameException.h"
#include "items/Weapon.h"
#include "items/Potion.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool SaveLoadManager::saveGame(const std::string& filePath, const Player& player, const Dungeon& dungeon) {
    std::ofstream outFile(filePath);
    if (!outFile.is_open()) {
        throw SaveLoadException("Khong the mo file de ghi: " + filePath);
    }

    // 1. Ghi thông tin Người chơi
    outFile << "[PLAYER]\n";
    outFile << "Name=" << player.getName() << "\n";
    outFile << "Level=" << player.getLevel() << "\n";
    outFile << "HP=" << player.getHp() << "\n";
    outFile << "MaxHP=" << player.getMaxHp() << "\n";
    outFile << "Attack=" << player.getAttack() << "\n";
    outFile << "Defense=" << player.getDefense() << "\n";
    outFile << "EXP=" << player.getExp() << "\n";
    outFile << "EXPToNext=" << player.getExpToNextLevel() << "\n";
    outFile << "Gold=" << player.getGold() << "\n";
    outFile << "Pos=" << player.getPosition() << "\n"; // Minh họa nạp chồng toán tử xuất stream operator<<
    outFile << "PosX=" << player.getPosition().x << "\n";
    outFile << "PosY=" << player.getPosition().y << "\n";

    // 2. Ghi thông tin Hầm ngục
    outFile << "[DUNGEON]\n";
    outFile << "Floor=" << dungeon.getFloorLevel() << "\n";
    outFile << "BossDefeated=" << (dungeon.isBossDefeated() ? 1 : 0) << "\n";

    // 3. Ghi thông tin Túi đồ
    const Inventory& inv = player.getInventory();
    outFile << "[INVENTORY]\n";
    outFile << "ItemCount=" << inv.getSize() << "\n";
    for (size_t i = 0; i < inv.getSize(); ++i) {
        const Item* item = inv[i];
        if (item) {
            // Kiểm tra dynamic_cast (RTTI / Polymorphism) để biết loại item
            const Weapon* w = dynamic_cast<const Weapon*>(item);
            if (w) {
                outFile << "Item=" << w->getName() << "|" << w->getDescription() << "|WEAPON|" << w->getBonusAttack() << "|" << w->getTextureId() << "\n";
            } else {
                const Potion* p = dynamic_cast<const Potion*>(item);
                if (p) {
                    outFile << "Item=" << p->getName() << "|" << p->getDescription() << "|POTION|" << p->getHealAmount() << "|" << p->getTextureId() << "\n";
                }
            }
        }
    }

    outFile.close();
    std::cout << "[Save Success] Da luu tien trinh vao file: " << filePath << std::endl;
    return true;
}

bool SaveLoadManager::loadGame(const std::string& filePath, Player& player, Dungeon& dungeon) {
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
        throw SaveLoadException("Khong the mo file de doc: " + filePath);
    }

    std::string line;
    std::string section = "";

    // Biến tạm để khôi phục
    int level = 1, hp = 100, maxHp = 100, attack = 15, defense = 5;
    int exp = 0, expToNext = 50, gold = 0;
    int posX = 2, posY = 2;
    int floor = 1;
    int bossDefeated = 0;

    // Xóa sạch túi đồ hiện tại để nạp lại
    while (player.getInventory().getSize() > 0) {
        player.getInventory().removeItem(0);
    }

    while (std::getline(inFile, line)) {
        if (line.empty()) continue;

        if (line[0] == '[' && line.back() == ']') {
            section = line.substr(1, line.length() - 2);
            continue;
        }

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = line.substr(0, eqPos);
        std::string val = line.substr(eqPos + 1);

        if (section == "PLAYER") {
            if (key == "Level") level = std::stoi(val);
            else if (key == "HP") hp = std::stoi(val);
            else if (key == "MaxHP") maxHp = std::stoi(val);
            else if (key == "Attack") attack = std::stoi(val);
            else if (key == "Defense") defense = std::stoi(val);
            else if (key == "EXP") exp = std::stoi(val);
            else if (key == "EXPToNext") expToNext = std::stoi(val);
            else if (key == "Gold") gold = std::stoi(val);
            else if (key == "Pos") {
                // Minh họa nạp chồng toán tử nhập stream operator>> (Slide 25 Chương 4)
                Position readPos;
                std::stringstream ss(val);
                ss >> readPos;
                posX = readPos.x;
                posY = readPos.y;
            }
            else if (key == "PosX") posX = std::stoi(val);
            else if (key == "PosY") posY = std::stoi(val);
        } else if (section == "DUNGEON") {
            if (key == "Floor") floor = std::stoi(val);
            else if (key == "BossDefeated") bossDefeated = std::stoi(val);
        } else if (section == "INVENTORY") {
            if (key == "Item") {
                // Parse dạng: Name|Description|Type|Value[|TextureId]
                std::stringstream ss(val);
                std::string iName, iDesc, iType, iVal, iTex;
                std::getline(ss, iName, '|');
                std::getline(ss, iDesc, '|');
                std::getline(ss, iType, '|');
                std::getline(ss, iVal, '|');
                std::getline(ss, iTex, '|');

                if (iType == "WEAPON") {
                    player.getInventory().addItem(std::make_unique<Weapon>(iName, iDesc, std::stoi(iVal), Position(0, 0), iTex));
                } else if (iType == "POTION") {
                    player.getInventory().addItem(std::make_unique<Potion>(iName, iDesc, std::stoi(iVal), Position(0, 0), iTex));
                }
            }
        }
    }

    inFile.close();

    // Khôi phục toàn bộ chỉ số cho người chơi
    player.setPosition(Position(posX, posY), true);
    player.setLevel(level);
    player.setMaxHp(maxHp);
    player.setHp(hp);
    player.setAttack(attack);
    player.setDefense(defense);
    player.setExp(exp);
    player.setExpToNextLevel(expToNext);
    player.setGold(gold);
    
    // Tái tạo tầng hầm ngục
    dungeon.generate(floor);
    dungeon.setBossDefeated(bossDefeated != 0);

    std::cout << "[Load Success] Da nap thanh cong tien trinh tu file: " << filePath << std::endl;
    std::cout << " -> Level " << level << " | HP: " << hp << "/" << maxHp 
              << " | Tang: " << floor << " | Vang: " << gold << std::endl;
    return true;
}

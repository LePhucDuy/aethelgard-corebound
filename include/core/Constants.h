#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants {
    // Window settings
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;
    constexpr const char* GAME_TITLE = "Aethelgard: Corebound - 2D Roguelike";
    constexpr int TARGET_FPS = 60;

    // Grid & Map settings
    constexpr int TILE_SIZE = 32;       // Kích thước ô lưới hiển thị
    constexpr int DUNGEON_WIDTH = 75;   // 75 ô ngang (2400px chiều dài map)
    constexpr int DUNGEON_HEIGHT = 26;  // 26 ô dọc (832px chiều cao map)

    // Inventory settings
    constexpr int MAX_INVENTORY_SLOTS = 10;
}

#endif // CONSTANTS_H

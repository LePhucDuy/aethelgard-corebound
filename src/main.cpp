#include "engine/GameEngine.h"
#include <ctime>
#include <cstdlib>

int main(int argc, char* argv[]) {
    // Khởi tạo hạt giống số ngẫu nhiên phục vụ sinh quái và chỉ số chí mạng
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    std::string screenshotPath = "";
    int spawnX = -1, spawnY = -1; // Tuỳ chọn debug: --spawn X Y (nhảy thẳng tới khu bất kỳ)
    bool startWithInventory = false; // Tuỳ chọn debug: --inventory (mở sẵn túi đồ khi khởi động)

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--screenshot" && i + 1 < argc) {
            screenshotPath = argv[++i];
        } else if (arg == "--spawn" && i + 2 < argc) {
            spawnX = std::atoi(argv[++i]);
            spawnY = std::atoi(argv[++i]);
        } else if (arg == "--inventory") {
            startWithInventory = true;
        }
    }

    // Khởi chạy vòng lặp trò chơi Aethelgard: Corebound
    GameEngine engine(spawnX, spawnY, startWithInventory);
    engine.run(screenshotPath);

    return 0;
}

#include "engine/GameEngine.h"
#include <ctime>
#include <cstdlib>

int main(int argc, char* argv[]) {
    // Khởi tạo hạt giống số ngẫu nhiên phục vụ sinh quái và chỉ số chí mạng
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    std::string screenshotPath = "";
    int spawnX = -1, spawnY = -1; // Tuỳ chọn debug: --spawn X Y (nhảy thẳng tới khu bất kỳ)
    bool startWithInventory = false; // Tuỳ chọn debug: --inventory (mở sẵn túi đồ khi khởi động)
    bool startLethal = false;        // Tuỳ chọn debug: --kill (kiểm tra hoạt cảnh chết)
    bool startWithShop = false;      // Tuỳ chọn debug: --shop (mở sẵn cửa hàng khi khởi động)
    bool startWithForge = false;     // Tuỳ chọn debug: --forge (mở sẵn đe rèn khi khởi động)

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--test-oop") {
            GameEngine::runOOPAcademicTests();
            return 0;
        } else if (arg == "--screenshot" && i + 1 < argc) {
            screenshotPath = argv[++i];
        } else if (arg == "--spawn" && i + 2 < argc) {
            spawnX = std::atoi(argv[++i]);
            spawnY = std::atoi(argv[++i]);
        } else if (arg == "--inventory") {
            startWithInventory = true;
        } else if (arg == "--kill") {
            startLethal = true;
        } else if (arg == "--shop") {
            startWithShop = true;
        } else if (arg == "--forge") {
            startWithForge = true;
        }
    }

    // Khởi chạy vòng lặp trò chơi Aethelgard: Corebound
    GameEngine engine(spawnX, spawnY, startWithInventory, startLethal, startWithShop, startWithForge);
    engine.run(screenshotPath);

    return 0;
}

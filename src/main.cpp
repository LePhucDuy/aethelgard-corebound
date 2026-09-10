#include "engine/GameEngine.h"
#include <ctime>
#include <cstdlib>

int main(int argc, char* argv[]) {
    // Khởi tạo hạt giống số ngẫu nhiên phục vụ sinh quái và chỉ số chí mạng
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    std::string screenshotPath = "";
    if (argc >= 3 && std::string(argv[1]) == "--screenshot") {
        screenshotPath = argv[2];
    }

    // Khởi chạy vòng lặp trò chơi Aethelgard: Corebound
    GameEngine engine;
    engine.run(screenshotPath);

    return 0;
}
